#pragma once

#include "windows.h"

#include "RingBuffer.h"
#include "Buffer.h"
#include "ObjectPool.h"
#include <atomic> 
namespace NetIO {

    using namespace ObjectPool;

    const unsigned int MAX_BLOCK_SIZE = 1024;
    const unsigned int MAX_INFLIGHT   = 4;
    
    class Stream {
    public:

        enum IO_MODE {
            READ,
            WRITE,
            READ_SUBMIT,
            WRITE_SUBMIT
        };

        struct Header {
            uint64_t     payloadSize;
    
        }__attribute__((packed));

        struct Body {

            char      data[MAX_BLOCK_SIZE];

        }__attribute__((packed));

        struct Transaction;

        struct OverlappedExtended  {
            
            OVERLAPPED   overlapped;
            Transaction* tr;

        }__attribute__((packed));

        struct Transaction : ObjectPool::Pooled {

            Stream* parentStream;
            Body payload;
            
            WSABUF       buffInfo;

            IO_MODE      mode;

            uint64_t processedBytes;
            bool     firstUpdate;


            Buffer               buff;
            OverlappedExtended   overlapped;


            Transaction() {

                buff.clear();

                SecureZeroMemory(&overlapped, sizeof(overlapped));
                SecureZeroMemory(&payload, sizeof(payload));
                
                overlapped.tr = this;

                processedBytes = 0;

                firstUpdate = true;
            }

            void init(Stream* stream, IO_MODE mode) {

                std::cout << "Transaction: Init: mode: " << mode << std::endl;
                
                SecureZeroMemory(&payload, sizeof(payload));
                
                this->mode          = mode;
                this->parentStream  = stream;

                processedBytes = 0;

                firstUpdate = true;
                overlapped.tr = this;
            }

            void clearTxData() {

                std::cout << "CLEARING TX DATA" << std::endl;

                SecureZeroMemory(&payload, sizeof(payload));
            }

            void reset() {

                std::cout << "RESETTING TRANSACTION" << std::endl;

                buff.clear();

                SecureZeroMemory(&overlapped, sizeof(overlapped));
                SecureZeroMemory(&payload, sizeof(payload));

                processedBytes = 0;
                firstUpdate = true;

                clearTxData();
                
                buffInfo.len = MAX_BLOCK_SIZE;
                buffInfo.buf = payload.data;
                
                overlapped.tr   = this; 
            }
            
        };

    private:

        ObjectPool::Pool<Transaction>       m_transactionPool;

        HANDLE                              m_port;
        SOCKET                              m_socket;

        Transaction*                        m_inputTr;

        std::function<void(Buffer& data)> m_onReadComplete;
        std::function<void()>             m_onReadStart;

        std::function<void()>             m_onWriteComplete;
        std::function<Buffer()>           m_onWriteStart; 

        bool                             m_transactionInProgress;  
        std::deque<Transaction*>        m_pendingTransactions;       
        std::mutex*                     m_readLock;       
        std::mutex*                     m_writeLock;

        void submitRead(Transaction* tr) {

            tr->buffInfo.len = MAX_BLOCK_SIZE;
            tr->buffInfo.buf = tr->payload.data;

            tr->mode = IO_MODE::READ_SUBMIT;

            PostQueuedCompletionStatus(
                                        m_port,
                                        sizeof(tr),
                                        IO_MODE::READ,
                                        (OVERLAPPED*)&tr->overlapped
                                        );
            
        }

        void submitWrite(Transaction* tr) {

            tr->mode = IO_MODE::WRITE_SUBMIT;

            PostQueuedCompletionStatus(
                                        m_port,
                                        sizeof(tr),
                                        IO_MODE::WRITE,
                                        (OVERLAPPED*)&tr->overlapped
                                        );

        }

    public:

        Stream() : m_transactionPool(16) {
            
            m_inputTr         = nullptr;
            m_transactionInProgress = false;
        }

        Header appendHeader(Transaction* tr) {

            Header header;
            header.payloadSize = tr->buff.getSize();

            Buffer headerData;
            headerData.write((char*)&header, sizeof(Header));

            tr->buff.getVec().insert(
                                        tr->buff.getVec().begin(), 
                                        headerData.getVec().begin(), 
                                        headerData.getVec().end()
                                    );

            return header;
        }

        void handleIOSubmit(Transaction* tr) {

            DWORD timeout = 0;
            DWORD flags = 0;
            
            int result = 0;

            if(tr->mode == IO_MODE::READ_SUBMIT) {
                
                tr->buffInfo.len = MAX_BLOCK_SIZE;
                tr->buffInfo.buf = tr->payload.data;

                tr->mode = IO_MODE::READ;
   
                result = WSARecv(
                                m_socket,
                                &tr->buffInfo,
                                1,
                                NULL,
                                &flags,
                                (OVERLAPPED*)&tr->overlapped,
                                NULL
                            );


            } else if(tr->mode == IO_MODE::WRITE_SUBMIT) {

                tr->mode = IO_MODE::WRITE;

                result = WSASend(
                            m_socket,
                            &tr->buffInfo,
                            1,
                            NULL,
                            0,
                            (OVERLAPPED*)&tr->overlapped,
                            NULL
                            );  
                    
            }
        }

        void updateReadTransactions(DWORD bytesRead) {

            Buffer completedTr;

            Buffer& inputStream = m_inputTr->buff;
            char*   payload     = m_inputTr->payload.data;

            m_inputTr->buff.write(payload, bytesRead);

            if(m_inputTr->buff.getSize() > sizeof(Header) && m_inputTr->firstUpdate) {
            
                m_inputTr->firstUpdate = false;

                Header header = *(Header*)m_inputTr->buff.getBase();

                m_inputTr->processedBytes = header.payloadSize;

                printf("%s%d\n", "!!!!!!!!!!!!!!!!Payload Size!!!!!!!!!!!!!!!!!!!!!: ", m_inputTr->processedBytes);
            }

            if(m_inputTr->buff.getSize() < m_inputTr->processedBytes) {
                
               submitRead(m_inputTr);

            } else {

                completedTr.write(      m_inputTr->buff.getBase() + sizeof(Header), 
                                        m_inputTr->processedBytes
                                );
                
                m_inputTr->buff.getVec().erase( m_inputTr->buff.getVec().begin(), 
                                                m_inputTr->buff.getVec().begin() + m_inputTr->processedBytes + sizeof(Header));

                printf("%s%d\n", "Completed Transaction: ", m_inputTr->processedBytes);

                m_onReadComplete(completedTr);

                m_inputTr->processedBytes = 0;
                m_inputTr->firstUpdate    = true;
                
                triggerRead();
            }
        }

        void writeNext(Transaction* tr, std::function<void()> onLastPacket) {

            unsigned int amountToSend = tr->buff.readSeq(tr->payload.data, MAX_BLOCK_SIZE);
                
            if(amountToSend == 0) {

                m_onWriteComplete();
                m_transactionPool.freeItem(tr);

                onLastPacket();
            
            } else {

                tr->buffInfo.len = amountToSend;
                tr->buffInfo.buf = tr->payload.data;

                submitWrite(tr);
            }
        }

        void transactionCompleted(Transaction* tr, DWORD numBytes) {

            if(tr->mode == IO_MODE::READ) {
                
                tr->parentStream->updateReadTransactions(numBytes);

            } else if(tr->mode == IO_MODE::WRITE) {
              
                writeNext(tr, [this]() { 
                
                    this->m_transactionInProgress = true;

                    startNextPendingWrite();
                });

            } else {

                handleIOSubmit(tr);
            }
        }

        void init(SOCKET socket, HANDLE completionPort) {

            m_port = completionPort;
            m_socket = socket;

            if(!CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), completionPort, 0, 4)) {

                std::cout << "Stream: Failed to attach completion port to socket. " << GetLastError() << std::endl;
            }
        }
        
        void triggerRead() {

            if(m_inputTr == nullptr) {

                m_inputTr = m_transactionPool.getItem();
            }

            m_inputTr->init(this, IO_MODE::READ_SUBMIT);

            submitRead(m_inputTr);
        }

        void regWriteStartCallback(std::function<Buffer()> callback) {

            m_onWriteStart = callback;
        }

        void regReadStartCallback(std::function<void()> callback) {

            m_onReadStart = callback;
        }

        void regReadCompleteCallback(std::function<void(Buffer&)> callback) {

            m_onReadComplete = callback;
        }


        void regWriteCompleteCallback(std::function<void()> callback) {

            m_onWriteComplete = callback;
        }

        void initWrite(Transaction* tr) {

            tr->init(this, IO_MODE::WRITE_SUBMIT);

            tr->overlapped.tr       = tr;
            tr->buff                = m_onWriteStart();
            
            appendHeader(tr);
        }

        void startNextPendingWrite() {

            Transaction* tr = nullptr;
            
            if(m_pendingTransactions.size() > 0) {

            
                tr = m_pendingTransactions.back();
                m_pendingTransactions.pop_back();
            }

            if(tr != nullptr) {

                m_transactionInProgress = true;
                writeNext(tr, [this]() {

                    this->m_transactionInProgress = true;

                    startNextPendingWrite();

                });

            } else {

                m_transactionInProgress = false;
            }
        }

        void triggerWrite() {

            printf("%s\n", "Triggering Write!");

            Transaction* tr = nullptr;

            if(!m_transactionInProgress) {

                printf("%s\n", "Not Queueing write request");
                tr = m_transactionPool.getItem();

                initWrite(tr);

                m_pendingTransactions.push_back(tr);

                startNextPendingWrite();

            } else {

                tr = m_transactionPool.getItem();

                initWrite(tr);

                printf("%s\n", "Queuing write request");
                m_pendingTransactions.push_back(tr);

            }
        }

    };

}
