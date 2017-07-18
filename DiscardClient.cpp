//
// Created by ht on 17-6-20.
//

#include "TcpClient.h"
#include "InetAddress.h"
#include "EventLoop.h"

#include <boost/bind.hpp>

#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;


class DiscardClient : boost::noncopyable {
public:
    DiscardClient(EventLoop *loop, const InetAddress &servAddr, int size)
            : loop_(loop),
              client_(loop_, servAddr, "DiscardClient"),
              message_(size, 'H')
    {
        client_.setConnectionCallback(boost::bind(&DiscardClient::onConnection, this, _1));
        client_.setMessageCallback(boost::bind(&DiscardClient::onMessage, this, _1, _2, _3));
        client_.setWriteCompleteCallback(boost::bind(&DiscardClient::onWriteComplete, this, _1));

    }

    void connect()
    {
        client_.connect();
    }

private:

    void onConnection(const TcpConnectionPtr &conn);

    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);

    void onWriteComplete(const TcpConnectionPtr &conn);

    EventLoop *loop_;
    TcpClient client_;
    string message_;

};

void DiscardClient::onConnection(const TcpConnectionPtr &conn)
{
    cout << conn->name() << " " << conn->localAddress().toIpPort() << "->" << conn->peerAddress().toIpPort() << endl;

    if (conn->connected())
    {
        conn->setTcpNoDelay(true);
        conn->send(message_);
    }
    else {
        loop_->quit();
    }
}

void DiscardClient::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    buf->retrieveAll();
}

void DiscardClient::onWriteComplete(const TcpConnectionPtr &conn)
{
    cout << "write " << message_.size() << " bytes complete" << endl;
    //TODO: why? ->to Write messages continuous
    conn->send(message_);
}

int main(int argc, char **argv)
{
    EventLoop loop;
    InetAddress servAddr(2009);
    int size = 1024;

    DiscardClient client(&loop, servAddr, size);
    client.connect();
    loop.loop();
}