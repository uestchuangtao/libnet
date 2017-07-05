#libnet

### 2017-5-31 Mutex.h Condition.h Condition.cpp

### 2017-6-1 BlockingQueue.h Exception.h Exception.cpp
TODO: add stack for Exception

### 2017-6-3 BoundedBlockingQueue.h Thread.h Thread.cpp
TODO: CurrentThread, tid -> finisheds

### 2017-6-4 ThreadPool.h ThreadPool.cpp

### 2017-6-8 Socket.h Socket.cpp Acceptor.h Acceptor.cpp

### 2017-6-9 EventLoop.h EventLoop.cpp Channel.h Channel.cpp
Channel: tied ???  TcpConnectionPtr

### 2017-6-10 Poller.h Poller.cpp

### 2017-6-11 PollPoller EPollPoller Connector

### 2017-6-13
modify cmakelist
Atomic need to rewrite
copy TimeStamp from muduo
modify Mutex add UnassignGuard

### 2017-6-14
TcpConnection
Callbacks
copy Buffer from muduo
Channel add tie

### 2017-6-16
TcpServer  

### 2017-6-18
TcpClient EventLoopThread EventLoopThreadPool

### 2017-6-19
DaytimeServer 

### 2017-6-20
#### TcpConnection
 why??? ConnectionDestroyed&&HandleClose ->connectionCallback 
 add testCase:DaytimeServer
 
### 2017-6-21
add testCase:DiscardServer&DiscardClient

### 2017-6-30 -- 2017-7-3
edit UsbServer

### 2017-7-4
debug UsbServer, add resume from break-point






