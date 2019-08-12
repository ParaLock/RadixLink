#pragma once

#include "windows.h"
#include "Stream.h"
#include "TaskExecutor.h"

namespace NetIO {


    class StreamGroup {
        private:

            HANDLE                          m_port;
            std::map<std::string, Stream>   m_streams;

            bool                            m_isProcessing;

            TaskExecutor&                   m_executor;
        public:

            StreamGroup(TaskExecutor& executor) : m_executor(executor) {

                //create io completion port
                executor.createQueue("async_net_io", 4);

                m_port = CreateIoCompletionPort(
                                                        INVALID_HANDLE_VALUE,
                                                        NULL,
                                                        0,
                                                        0
                                                    );

                m_isProcessing = false;
            }


            void activate() {

                if(!m_isProcessing) {


                    m_executor.addTask(Task("async_net_io", [this]() {

                            this->processStreams();
                    }));

                    m_executor.addTask(Task("async_net_io", [this]() {

                            this->processStreams();
                    }));

                    m_executor.addTask(Task("async_net_io", [this]() {

                            this->processStreams();
                    }));

                    m_executor.addTask(Task("async_net_io", [this]() {

                            this->processStreams();
                    }));

                    m_isProcessing = true;


                }

            }

            bool addStream(std::string name, SOCKET socket) {

                if(m_streams.find(name) == m_streams.end()) {

                    m_streams.insert({name, Stream()});

                    auto& s = m_streams.at(name);
                    s.init(socket, m_port);

                    return true;
                }

                return false;
            }

            void beginRead(std::string name, std::function<void(Buffer&)> onComplete, std::function<void()> onStart) {
                
                
                std::cout << "StreamGroup: Registering reader!" << std::endl;

                auto& stream = m_streams.at(name);

                stream.regReadCompleteCallback(onComplete);
                stream.regReadStartCallback(onStart);

            }

            void beginWrite(std::string name, std::function<void()> onComplete, std::function<Buffer()> onStart) {


                std::cout << "StreamGroup: Registering writer!" << std::endl;

                auto& stream = m_streams.at(name);

                stream.regWriteCompleteCallback(onComplete);
                stream.regWriteStartCallback(onStart);

            }


            void triggerRead(std::string name) {
                
                auto& stream = m_streams.at(name);
                stream.triggerRead();
            }

            void triggerWrite(std::string name) {

                auto& stream = m_streams.at(name);
                stream.triggerWrite();
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

            void processStreams() {

                DWORD                      numBytes;
                std::cout << "Processing Stream: Begin: " << std::endl; 
 
                unsigned long long key  = 0;
				DWORD waitTime          = INFINITE;

                OVERLAPPED* poverlapped = nullptr;

                BOOL result = GetQueuedCompletionStatus(
                        m_port,
                        &numBytes,
                        &key,
                        &poverlapped,
                        waitTime
                );

				if (result && poverlapped != nullptr) {
		     
                    Stream::OverlappedExtended* info = (Stream::OverlappedExtended*)poverlapped;

                    info->tr->parentStream->transactionCompleted(info->tr);
				
                    std::cout << "Processing Stream: End: " << GetLastErrorAsString() << std::endl;
                }
       
                if(m_isProcessing) {

                    m_executor.addTask(Task("async_net_io", [this]() {

                        this->processStreams();

                    }), true);
                }
            }
    };

}