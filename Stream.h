#pragma once

#include "windows.h"

#include "RingBuffer.h"
#include "Buffer.h"
#include "ObjectPool.h"

namespace NetIO {

    using namespace ObjectPool;

    const unsigned int MAX_BLOCK_SIZE = 1023;
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
            bool         lastPacket;

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

            Header header;
            Body payload;
            
            WSABUF       buffInfo[2];

            IO_MODE      mode;

            uint64_t received;
            uint64_t sent;

            Buffer               buff;
            OverlappedExtended   overlapped;

            Transaction() {

                buff.clear();

                SecureZeroMemory(&overlapped, sizeof(overlapped));
                SecureZeroMemory(&payload, sizeof(payload));
                SecureZeroMemory(&header, sizeof(header));
                

                overlapped.tr = this;

                received = 0;
                sent     = 0;

                //Setup header
                buffInfo[0].len = sizeof(header);
                buffInfo[0].buf = (char*)&header;

                //Setup payload
                buffInfo[1].len = MAX_BLOCK_SIZE;
                buffInfo[1].buf = (char*)&payload;
            }

            void init(Stream* stream, IO_MODE mode) {

                std::cout << "Transaction: Init: mode: " << mode << std::endl;

                this->mode          = mode;
                this->parentStream  = stream;

                overlapped.tr = this;

                buffInfo[0].len = sizeof(header);
                buffInfo[0].buf = (char*)&header;

                //Setup payload
                buffInfo[1].len = MAX_BLOCK_SIZE;
                buffInfo[1].buf = (char*)&payload;
            }

            void clearTxData() {

                std::cout << "CLEARING TX DATA" << std::endl;

                SecureZeroMemory(&payload, sizeof(payload));
                SecureZeroMemory(&header, sizeof(header));

                buffInfo[0].len = sizeof(header);
                buffInfo[0].buf = (char*)&header;

                //Setup payload
                buffInfo[1].len = MAX_BLOCK_SIZE;
                buffInfo[1].buf = (char*)&payload;
            }

            void reset() {

                std::cout << "RESETTING TRANSACTION" << std::endl;

                buff.clear();

                SecureZeroMemory(&overlapped, sizeof(overlapped));
                SecureZeroMemory(&payload, sizeof(payload));
                SecureZeroMemory(&header, sizeof(header));

                received = 0;
                sent     = 0;

                buffInfo[0].len = sizeof(header);
                buffInfo[0].buf = (char*)&header;

                //Setup payload
                buffInfo[1].len = MAX_BLOCK_SIZE;
                buffInfo[1].buf = (char*)&payload;
            }
            
        };

    private:

        ObjectPool::Pool<Transaction>       m_transactionPool;

        HANDLE                   m_port;
        SOCKET                   m_socket;

        std::function<void(Buffer& data)> m_onReadComplete;
        std::function<void()>             m_onReadStart;

        std::function<void()>             m_onWriteComplete;
        std::function<Buffer()>           m_onWriteStart;          


        void performRead(Transaction* tr) {


            tr->buffInfo[0].len = sizeof(tr->header);
            tr->buffInfo[0].buf = (char*)&tr->header;

            tr->buffInfo[1].len = MAX_BLOCK_SIZE;
            tr->buffInfo[1].buf = (char*)&tr->payload;

            PostQueuedCompletionStatus(
                                        m_port,
                                        sizeof(tr),
                                        IO_MODE::READ,
                                        (OVERLAPPED*)&tr->overlapped
                                        );


        }

        void performWrite(Transaction* tr) {

            tr->buffInfo[0].len = sizeof(tr->header);
            tr->buffInfo[0].buf = (char*)&tr->header;

            tr->buffInfo[1].len = MAX_BLOCK_SIZE;
            tr->buffInfo[1].buf = (char*)&tr->payload;

            PostQueuedCompletionStatus(
                                        m_port,
                                        sizeof(tr),
                                        IO_MODE::WRITE,
                                        (OVERLAPPED*)&tr->overlapped
                                        );

        }

    public:

        Stream() : m_transactionPool(16) {

        }

        std::string GetLastErrorAsString()
		{
            //Get the error message, if any.
            DWORD errorMessageID = ::WSAGetLastError();
            if (errorMessageID == 0)
                return std::string(); //No error message has been recorded

            LPSTR messageBuffer = nullptr;
            size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

            std::string message(messageBuffer, size);

            //Free the buffer.
            LocalFree(messageBuffer);

            return message;
        }

        void handleIOSubmit(Transaction* tr) {

            DWORD timeout = 0;
            DWORD flags = 0;

            int result = 0;

            if(tr->mode == IO_MODE::READ_SUBMIT) {
                
                tr->mode = IO_MODE::READ;
    
                result = WSARecv(
                                m_socket,
                                tr->buffInfo,
                                2,
                                NULL,
                                &flags,
                                (OVERLAPPED*)&tr->overlapped,
                                NULL
                            );

                std::cout << "Steam: Read: result: " << result << std::endl;
                std::cout << "Stream: Read: result as string: " << GetLastErrorAsString() << std::endl;    


            } else if(tr->mode == IO_MODE::WRITE_SUBMIT) {

                tr->mode = IO_MODE::WRITE;

                printf("%s\n", "Dumping write header and payload");

                printf("%x%x\n\n", tr->header.payloadSize, tr->header.lastPacket);
                
                for(int i = 0; i < tr->header.payloadSize; i++) {

                    printf("%x", tr->payload.data[i]);
                }

                result = WSASend(
                                m_socket,
                                tr->buffInfo,
                                2,
                                NULL,
                                0,
                                (OVERLAPPED*)&tr->overlapped,
                                NULL
                                );

                std::cout << "Steam: Sending Write: result: " << result << std::endl;
                std::cout << "Stream: Write: result as string: " << GetLastErrorAsString() << std::endl;    
                
            } else {

                std::cout << "Stream: Unknown IO mode." << std::endl;
            }


        }

        void transactionCompleted(Transaction* tr) {

            if(tr->mode == IO_MODE::READ) {

                tr->buff.write(tr->payload.data, tr->header.payloadSize); 

                if(tr->header.lastPacket) {

                    m_onReadComplete(tr->buff);

                    m_transactionPool.freeItem(tr);
                    
                    triggerRead();

                } else {
                    
                    tr->mode = IO_MODE::READ_SUBMIT;

                    performRead(tr);
                }

            } else if(tr->mode == IO_MODE::WRITE) {

                tr->clearTxData();

                printf("%s\n", "HANDLING WRITE");
                printf("%s%d\n", "COUNT", tr->buff.getSize());
                
                printf("%s%d\n", "OFFSET BEFORE READ", tr->buff.getCurrOffset());
                
                unsigned int amountToSend = tr->buff.readSeq(tr->payload.data, MAX_BLOCK_SIZE);

                tr->header.lastPacket   = (tr->buff.getCurrOffset() == tr->buff.getSize());
                tr->header.payloadSize  = amountToSend;

                printf("%s%d\n", "OFFSET AFTER READ", tr->buff.getCurrOffset());
                printf("%s%d\n", "AMOUNT TO SEND", amountToSend);
                
                if( (tr->buff.getCurrOffset() == tr->buff.getSize()) && amountToSend == 0) {

                    printf("%s\n", "HANDLING LAST PACKET");

                    m_onWriteComplete();
                
                    m_transactionPool.freeItem(tr);

                } else {

                    printf("%s\n", "WRITING NEXT PACKET");

                    tr->mode = IO_MODE::WRITE_SUBMIT;

                    performWrite(tr);
                }

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

        void triggerRead() {

            Transaction* tr    = m_transactionPool.getItem();
            
            tr->init(this, IO_MODE::READ_SUBMIT);
            
            tr->clearTxData();
      
            performRead(tr);
        }

        void triggerWrite() {

            Transaction* tr = m_transactionPool.getItem();

            tr->init(this, IO_MODE::WRITE_SUBMIT);

            tr->clearTxData();
      
            tr->overlapped.tr       = tr;
            tr->buff                = m_onWriteStart();

            printf("%s\n", "HANDLING WRITE");
            printf("%s%d\n", "COUNT", tr->buff.getSize());
            
            printf("%s%d\n", "OFFSET BEFORE READ", tr->buff.getCurrOffset());

            unsigned int amountToSend = tr->buff.readSeq(tr->payload.data, MAX_BLOCK_SIZE);

            printf("%s%d\n", "OFFSET AFTER READ", tr->buff.getCurrOffset());
            printf("%s%d\n", "AMOUNT TO SEND", amountToSend);
            
            tr->header.lastPacket   = (tr->buff.getCurrOffset() == tr->buff.getSize());
            tr->header.payloadSize  = amountToSend;

            printf("%s\n", "WRITING FIRST PACKET");
            
            performWrite(tr);
        }

    };

}
