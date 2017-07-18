//
// Created by ht on 17-6-29.
//


#include "TcpServer.h"
#include "TcpConnection.h"
#include "EventLoop.h"

#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <string>
#include <unordered_map>
#include <stdio.h>
#include <sys/stat.h> // fstat
#include <string.h>
#include <errno.h>


using std::cout;
using std::endl;
using std::string;

using std::unordered_map;

namespace {
    int numThreads = 5;
    char WorkSpace[32] = "/var/www/html/names/";
}


class UsbServer : boost::noncopyable {
public:
    UsbServer(EventLoop *loop, const InetAddress &listenAddr)
            : loop_(loop),
              server_(loop_, listenAddr, "UsbServer")
    {
        server_.setThreadNum(numThreads);
        server_.setConnectionCallback(boost::bind(&UsbServer::onConnection, this, _1));
        server_.setMessageCallback(boost::bind(&UsbServer::onMessage, this, _1, _2, _3));
        server_.setWriteCompleteCallback(boost::bind(&UsbServer::onWriteComplete, this, _1));
    }

    void start()
    {
        server_.start();
    }

private:
    enum MessageType {
        kCode,
        kCommand,
        kResult,
        kKeepalive,
        kRetry
    };

    enum ConnectionState {
        kReceiveHeader,
        kReceiveVerifyCode,
        kWaiting,
        kSendFile,
        kReceiveFile
    };

    struct MessageHeader {
        MessageType type_;
        uint32_t length_;
    };

    struct ClientInfo {
        ConnectionState state_;
        string verifyCode_;
        FILE *fpRecv_;
        FILE *fpSend_;
        uint32_t process_;
        uint32_t length_;
        TimerId checkCFileTimerId_;
        TimerId keepAliveTimerId_;

        ClientInfo()
                : state_(kReceiveVerifyCode),
                  verifyCode_(),
                  fpRecv_(NULL),
                  fpSend_(NULL),
                  process_(0),
                  length_(0),
                  checkCFileTimerId_(),
                  keepAliveTimerId_()
        {
            cout << "ClientInfo Construct" << endl;
        }

        ~ClientInfo()
        {
            cout << "ClientInfo Destroy" << endl;
            if (fpRecv_)
                fclose(fpRecv_);
            if (fpSend_)
                fclose(fpSend_);
        }
    };

    typedef boost::shared_ptr<struct ClientInfo> ClientInfoPtr;

    void onConnection(const TcpConnectionPtr &conn)
    {
        cout << conn->name() << " " << conn->localAddress().toIpPort() << "->" << conn->peerAddress().toIpPort()
             << endl;
        if (conn->connected())
        {
            ClientInfoPtr clientInfo(new ClientInfo);
            conn->setContext(clientInfo);
            cout << "UsbServer::onConnection,clientInfo.use_count:" << clientInfo.use_count() << endl;
            conn->setTcpNoDelay(true);
        }
    }

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
    {
        ClientInfoPtr &client_info = boost::any_cast<ClientInfoPtr &>(*conn->getMutableConText());
        cout << "UsbServer::onMessage,clientInfo.use_count:" << client_info.use_count() << endl;
        switch (client_info->state_)
        {
            case kReceiveVerifyCode:
                if (buf->readableBytes() >= kHeaderLen)
                {
                    const void *data = buf->peek();
                    struct MessageHeader messageHeader;
                    memcpy(&messageHeader, data, sizeof(struct MessageHeader));
                    assert(messageHeader.type_ == kCode);
                    if (messageHeader.length_ > 0x7fffffff)
                    {
                        cout << "UsbServer::OnMessage,Invalid length " << messageHeader.length_ << endl;
                        conn->forceClose();
                        return;
                    }
                    else if (buf->readableBytes() >= messageHeader.length_ + kHeaderLen)
                    {
                        buf->retrieve(kHeaderLen);
                        string verifyCode = buf->retrieveAsString(messageHeader.length_);
                        verifyCode += "@";
                        client_info->verifyCode_ = verifyCode;
                        cerFileCreate(verifyCode.c_str());
                        auto it = tasks_.find(client_info->verifyCode_);
                        if (it != tasks_.end())
                        {
                            struct MessageHeader messageHeader;
                            messageHeader.type_ = kRetry;
                            messageHeader.length_ = it->second;
                            //set process
                            client_info->process_ = it->second;
                            conn->send((const char *) &messageHeader, sizeof(messageHeader));
                        }

                        client_info->state_ = kWaiting;
                        TimerId check_timerId = conn->getLoop()->runEvery(1.0,
                                                                          boost::bind(&UsbServer::checkAndSendCFile,
                                                                                      this, TcpConnectionWkPtr(conn)));
                        client_info->checkCFileTimerId_ = check_timerId;
                        TimerId keepAlive_timerId = conn->getLoop()->runEvery(60.0,
                                                                              boost::bind(&UsbServer::checkKeepAlive,
                                                                                          this,
                                                                                          TcpConnectionWkPtr(conn)));
                        client_info->keepAliveTimerId_ = keepAlive_timerId;


                    }
                }
                break;
            case kReceiveHeader:
                if (buf->readableBytes() >= kHeaderLen)
                {
                    tasks_.insert(TaskList::value_type(client_info->verifyCode_, 0));

                    const void *data = buf->peek();
                    struct MessageHeader messageHeader;
                    memcpy(&messageHeader, data, sizeof(struct MessageHeader));
                    assert(messageHeader.type_ == kResult);
                    client_info->length_ = messageHeader.length_;
                    client_info->state_ = kReceiveFile;
                    buf->retrieve(kHeaderLen);

                    cout << "UsbServer::OnMessage ReceiveFile Size:" << messageHeader.length_ << endl;
                    //set write tempfile
                    char tempTFilePath[50];
                    strcpy(tempTFilePath, WorkSpace);
                    strcat(tempTFilePath, client_info->verifyCode_.c_str());
                    strcat(tempTFilePath, ".ttemp");
                    FILE *fp = ::fopen(tempTFilePath, "ab+");
                    if (fp)
                    {
                        client_info->fpRecv_ = fp;
                    } else
                    {
                        cout << "UsbServer::OnMessage,fopen ttemp failed!" << endl;
                        cout << "UsbServer::OnMessage " << errno << " " << strerror(errno) << endl;
                        conn->forceClose();
                        return;
                    }
                }
                break;

            case kReceiveFile:
                cout << "UsbServer::OnMessage kReceiveFile" << endl;
                if (buf->readableBytes() > 0)
                {
                    const void *data = buf->peek();
                    size_t nwrote = fwrite(data, sizeof(char), buf->readableBytes(), client_info->fpRecv_);
                    if (nwrote < buf->readableBytes())
                    {
                        cout << "UsbServer::OnMessage,fwrite ttemp failed!" << endl;
                        conn->forceClose();
                        return;
                    } else
                    {
                        buf->retrieveAll();
                        client_info->process_ += nwrote;
                        tasks_[client_info->verifyCode_] = client_info->process_;
                        cout << "UsbServer::OnMessage write TFile " << nwrote << " process:" << client_info->process_
                             << " length:" << client_info->length_ << endl;

                        //recv file finished
                        if (client_info->process_ == client_info->length_)
                        {
                            cout << "UsbServer::OnMessage,receive tfile finish" << endl;
                            ::fclose(client_info->fpRecv_);
                            client_info->fpRecv_ = NULL;
                            conn->send("ack");

                            removeCFile(client_info->verifyCode_.c_str());
                            //clear process
                            client_info->process_ = 0;
                            int n = tasks_.erase(client_info->verifyCode_);
                            assert(n == 1);

                            char tempTFilePath[50];
                            strcpy(tempTFilePath, WorkSpace);
                            strcat(tempTFilePath, client_info->verifyCode_.c_str());
                            strcat(tempTFilePath, ".ttemp");

                            char TFilePath[50];
                            strcpy(TFilePath, WorkSpace);
                            strcat(TFilePath, client_info->verifyCode_.c_str());
                            strcat(TFilePath, ".t");
                            cout << "TFilePath:" << TFilePath << " TempFilePath:" << tempTFilePath << endl;
                            int ret = rename(tempTFilePath, TFilePath);
                            if (ret == -1)
                            {
                                cout << "UsbServer::OnMessage rename " << errno << " " << strerror(errno) << endl;
                            }
                            client_info->state_ = kWaiting;
                            TimerId timerId = conn->getLoop()->runEvery(1.0,
                                                                        boost::bind(&UsbServer::checkAndSendCFile, this,
                                                                                    conn));
                            client_info->checkCFileTimerId_ = timerId;

                        }
                    }

                }
                break;
            case kWaiting:
                if (buf->readableBytes() >= kHeaderLen)
                {
                    const void *data = buf->peek();
                    struct MessageHeader messageHeader;
                    memcpy(&messageHeader, data, sizeof(struct MessageHeader));
                    assert(messageHeader.type_ == kKeepalive);
                    assert(messageHeader.length_ == 0);
                    buf->retrieve(kHeaderLen);
                    cout << "UsbServer::onMessage, receive keepalive ack" << endl;
                }
                break;
            default:
                cout << "UsbServer::OnMessage wrong connection state" << endl;
                break;
        }

    }

    void onWriteComplete(const TcpConnectionPtr &conn)
    {
        ClientInfoPtr &client_info = boost::any_cast<ClientInfoPtr &>(*conn->getMutableConText());
        /*sendfile version*/
        if (client_info->state_ == kSendFile)
        {
            cout << "UsbServer::onWriteComplete " << "kSendFile:Send CFile->senfile version" << endl;

            int fd = fileno(client_info->fpSend_);
            if (fd == -1)
            {
                cout << "UsbServer::onWriteComplete,fileno failed" << endl;
                cout << "UsbServer::onWriteComplete " << errno << " " << strerror(errno) << endl;
                conn->forceClose();
                return;
            }
            off_t offset = 0;
            struct stat cfileState;
            int ret = fstat(fd, &cfileState);
            if (ret == -1)
            {
                cout << "UsbServer::onWriteComplete,fstat failed" << endl;
                cout << "UsbServer::onWriteComplete " << errno << " " << strerror(errno) << endl;
                conn->forceClose();
                return;
            }
            size_t fileSize = static_cast<size_t>(cfileState.st_size);
            conn->sendfile(fd, offset, fileSize);
            cout << "UsbServer::onWriteComplete " << "Send Cfile finish" << endl;
            //removeCFile(client_info->verifyCode_.c_str());
            client_info->state_ = kReceiveHeader;
        }

        /*send version*/
        /*
        if (client_info->state_ == kSendFile)
        {
            cout << "UsbServer::onWriteComplete " << "kSendFile:Send CFile" << endl;
            char buf[kBufSize];
            size_t nread = ::fread(buf, 1, sizeof(buf), client_info->fpSend_);
            //long npos = ftell(client_info->fpSend_);
            cout << "UsbServer::onWriteComplete " << " read CFile:" << nread << " fpSend_:" << client_info->fpSend_
                 << endl;
            if(nread < 0)
            {
                cout << "UsbServer::onWriteComplete " << errno << " " << strerror(errno) << endl;
            }

            if (nread > 0)
            {
                conn->send(buf, nread);
            }
            else
            {
                //send cfile finish
                ::fclose(client_info->fpSend_);
                client_info->fpSend_ = NULL;
                cout << "UsbServer::onWriteComplete " << "Send Cfile finish" << endl;
                //removeCFile(client_info->verifyCode_.c_str());
                client_info->state_ = kReceiveHeader;
            }
        }*/


    }

    void checkAndSendCFile(const TcpConnectionWkPtr &conn)
    {
        TcpConnectionPtr guardConn = conn.lock();
        if (guardConn)
        {
            ClientInfoPtr &client_info = boost::any_cast<ClientInfoPtr &>(*guardConn->getMutableConText());
            if (client_info->state_ == kWaiting)
            {
                cout << "UsbServer::checkAndSendCFile " << "waiting for CFile down" << endl;
                //set cfile
                char CFilePath[50];
                strcpy(CFilePath, WorkSpace);
                strcat(CFilePath, client_info->verifyCode_.c_str());
                strcat(CFilePath, ".c");
                //waiting for .c file
                FILE *fpSend = ::fopen(CFilePath, "rb");
                //cout<<CFilePath<<endl;
                if (fpSend)
                {
                    cout << "UsbServer::checkAndSendCFile " << "CFile down" << endl;
                    client_info->state_ = kSendFile;
                    client_info->fpSend_ = fpSend;
                    struct stat cfileInfo;
                    stat(CFilePath, &cfileInfo);
                    struct MessageHeader sendHeader;
                    sendHeader.type_ = kCommand;
                    sendHeader.length_ = static_cast<uint32_t>(cfileInfo.st_size);
                    cout << "UsbServer::checkAndSendCFile  CFile size:" << cfileInfo.st_size << endl;
                    guardConn->send((const char *) &sendHeader, sizeof(sendHeader));
                    guardConn->getLoop()->Cancel(client_info->checkCFileTimerId_);
                }
            }
        }
    }

    void checkKeepAlive(const TcpConnectionWkPtr &conn)
    {
        TcpConnectionPtr guardConn = conn.lock();
        if (guardConn)
        {
            ClientInfoPtr &client_info = boost::any_cast<ClientInfoPtr &>(*guardConn->getMutableConText());
            if (client_info->state_ == kWaiting)
            {
                cout << "UsbServer::checkKeepAlive " << endl;
                struct MessageHeader sendHeader;
                sendHeader.type_ = kKeepalive;
                sendHeader.length_ = 0;
                guardConn->send((const char *) &sendHeader, sizeof(sendHeader));
            }
        }
    }

    void cerFileCreate(const char *pCode)
    {
        char buf[128];
        strcpy(buf, WorkSpace);
        strcat(buf, "~");
        strcat(buf, pCode);
        strcat(buf, ".t");
        FILE *fp = ::fopen(buf, "w+");
        ::fclose(fp);
    }

    void removeCFile(const char *pCode)
    {
        char buf[128];
        strcpy(buf, WorkSpace);
        strcat(buf, pCode);
        strcat(buf, ".c");
        int ret = remove(buf);
        if (ret != 0)
            cout << "UsbServer::removeCFile " << errno << " " << strerror(errno) << endl;
        assert(ret == 0);
    }

    typedef unordered_map<string, uint32_t> TaskList;

    TaskList tasks_;
    EventLoop *loop_;
    TcpServer server_;

    const static uint32_t kHeaderLen = sizeof(struct MessageHeader);
    const static int kBufSize = 64 * 1024;
};

int main(int argc, char **argv)
{
    InetAddress listenAddr(10796);
    EventLoop loop;
    UsbServer server(&loop, listenAddr);
    server.start();
    loop.loop();

}

