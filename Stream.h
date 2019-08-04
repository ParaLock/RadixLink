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
        };

        struct Body {

            char      data[MAX_BLOCK_SIZE];
        };

        struct Transaction;

        struct OverlappedExtended  {
            
            OVERLAPPED   overlapped;
            Transaction* tr;
        };

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
                
                SecureZeroMemory(&overlapped, sizeof(overlapped));

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

                header.payloadSize = 0;
            }

            void reset() {

                buff.clear();

                SecureZeroMemory(&overlapped, sizeof(overlapped));
                SecureZeroMemory(&payload, sizeof(payload));
                SecureZeroMemory(&header, sizeof(header));

                                received = 0;
                sent     = 0;

                //Setup header
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

            //Update payload info.
            PostQueuedCompletionStatus(
                                        m_port,
                                        sizeof(tr),
                                        IO_MODE::READ,
                                        (OVERLAPPED*)&tr->overlapped
                                        );


        }

        void performWrite(Transaction* tr) {

            DWORD flags = 0;
            DWORD timeout = 0;

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

            std::cout << "Stream: handleIOSubmit Begin: mode:" << tr->mode << std::endl;

            DWORD timeout = 0;
            DWORD flags = 0;

            int result = 0;

            if(tr->mode == IO_MODE::READ_SUBMIT) {
                
                std::cout << "Stream: handleIOSubmit: READ" << std::endl;

                tr->mode = IO_MODE::READ;

                result = WSARecv(
                                m_socket,
                                tr->buffInfo,
                                2,
                                (LPDWORD)&tr->received,
                                &flags,
                                (OVERLAPPED*)&tr->overlapped,
                                NULL
                            );

                std::cout << "Steam: Read: result: " << result << std::endl;
                std::cout << "Stream: Read: result as string: " << GetLastErrorAsString() << std::endl;    


            } else if(tr->mode == IO_MODE::WRITE_SUBMIT) {

                std::cout << "Stream: handleIOSubmit: WRITE" << std::endl;

                tr->mode = IO_MODE::WRITE;

                std::cout << "Stream: Write Begin" << "Payload Size: " << tr->header.payloadSize << std::endl;

                result = WSASend(
                                m_socket,
                                tr->buffInfo,
                                2,
                                (LPDWORD)&tr->sent,
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

        void init(SOCKET socket, HANDLE completionPort) {

            m_port = completionPort;
            m_socket = socket;

            if(!CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), completionPort, 0, 4)) {

                std::cout << "Stream: Failed to attach completion port to socket. " << GetLastError() << std::endl;
            }

           // triggerRead();
            
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
            
            tr->received = 0;

            performRead(tr);
        }

        void triggerWrite() {

            //Here we need to perform all inital write setup. 
            // TriggerWrite will only be called on first write.

            Transaction* tr = m_transactionPool.getItem();

            tr->init(this, IO_MODE::WRITE_SUBMIT);

            tr->overlapped.tr       = tr;
            tr->buff                = m_onWriteStart();
            tr->sent                = 0;

            //Prepare initial buffer.
            unsigned int amountToSend = tr->buff.readSeq(tr->payload.data, MAX_BLOCK_SIZE);

            tr->header.lastPacket   = (tr->buff.getCurrOffset() == tr->buff.getSize());
            tr->header.payloadSize  = amountToSend;

            performWrite(tr);
        }

        void transactionCompleted(Transaction* tr) {

            std::cout << "Stream: transactionCompleted: Begin" << std::endl;

            if(tr->mode == IO_MODE::READ) {

                //If we are here, we just got notified that a read has completed on this transaction. 
                // This means that we need to either prepare for another read or terminal the transaction if all packets for the transaction have been read.

                std::cout << "Stream: transactionCompleted: Mode Read: " << std::endl;
                std::cout << "Stream: transactionCompleted: Mode Read: Payload size: " << tr->header.payloadSize << std::endl;

                tr->buff.write(tr->payload.data, tr->header.payloadSize); 

                std::cout << "Stream: transactionCompleted: Finished buffer write" << std::endl;

                if(tr->header.lastPacket) {

                    std::cout << "Stream: transactionCompleted: Write Handling last packet" << std::endl;

                    m_onReadComplete(tr->buff);
                    
                    m_transactionPool.freeItem(tr);

                } else {

                    std::cout << "Stream: transactionCompleted: Reading next packet" << std::endl;

                    performRead(tr);
                }

            } else if(tr->mode == IO_MODE::WRITE) {

                //We just got notified that a write has completed on this transaction. 
                // This means we need to prepare the next write.

                //This first step is to see if we even need to send more data for this transaction. 
                //We can do this by checking whether buff in transaction is equal to amount sent.

                std::cout << "Stream: transactionCompleted: Mode Write" << std::endl;

                unsigned int amountToSend = tr->buff.readSeq(tr->payload.data, MAX_BLOCK_SIZE);

                tr->header.lastPacket   = (tr->buff.getCurrOffset() == tr->buff.getSize());
                tr->header.payloadSize  = amountToSend;

                if(tr->header.lastPacket) {

                    std::cout << "Stream: transactionCompleted: Read Handling last packet" << std::endl;

                    m_onWriteComplete();

                    m_transactionPool.freeItem(tr);

                } else {


                    std::cout << "Stream: transactionCompleted: Writing next packet" << std::endl;

                    performWrite(tr);
                }

            } else {


                std::cout << "Stream: transactionCompleted: Handling io submit" << std::endl;

                handleIOSubmit(tr);
            }


            std::cout << "Stream: transactionCompleted: End" << std::endl;
        }


    };

}
