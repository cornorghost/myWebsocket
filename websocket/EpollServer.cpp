#include "EpollServer.h"
#include "Message.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>

EpollServer::EpollServer(size_t thread_num = 8)
{
    port = 3000;
    shared_ptr <ThreadPool> temp(new ThreadPool(thread_num));
    pools = temp;
}

void EpollServer::addClient(int socket)
{
    if (this->clientList.count(socket)) {
        cout << "client int list already" << endl;
        return;
    }
    unique_lock<mutex> guard(this->clientMutex);
    clientList[socket] = socket;
    cout << "client added to list" << endl;
}

void EpollServer::delClient(int socket)
{
    unique_lock<mutex> guard(this->clientMutex);
    clientList.erase(socket);
    cout << "client remove client" << endl;
}

vector<int> EpollServer::getClients()
{
    vector<int> res;
    for (auto item : this->clientList) res.push_back(item.first);
    return res;
}



int EpollServer::initSocket()
{
    //创建一个监听socket
    this->listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->listenfd == -1)
    {
        cout << "create listen socket error" << endl;
        return -1;
    }

    //设置重用IP地址和端口号
    int on = 1;
    setsockopt(this->listenfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
    setsockopt(this->listenfd, SOL_SOCKET, SO_REUSEPORT, (char*)&on, sizeof(on));

    //将监听socker设置为非阻塞的
    int oldSocketFlag = fcntl(this->listenfd, F_GETFL, 0);
    int newSocketFlag = oldSocketFlag | O_NONBLOCK;
    if (fcntl(this->listenfd, F_SETFL, newSocketFlag) == -1)
    {
        close(this->listenfd);
        cout << "set listenfd to nonblock error" << endl;
        return -1;
    }

    //初始化服务器地址
    struct sockaddr_in bindaddr;
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bindaddr.sin_port = htons(this->port);

    if (bind(this->listenfd, (struct sockaddr*)&bindaddr, sizeof(bindaddr)) == -1)
    {
        cout << "bind listen socker error." << endl;
        close(this->listenfd);
        return -1;
    }

    //启动监听
    if (listen(this->listenfd, SOMAXCONN) == -1)
    {
        cout << "listen error." << endl;
        close(this->listenfd);
        return -1;
    }


    //创建epollfd
    this->epollfd = epoll_create(1111);
    if (this->epollfd == -1)
    {
        cout << "create epollfd error." << endl;
        close(this->listenfd);
        return -1;
    }
    if (addEvent(this->listenfd, EPOLLIN) == -1) {
        cout << "add listen event failed!" << endl;
    }
    else  cout << "add listen event succeed!" << endl;
}

int EpollServer::addEvent(int sockfd, int events)
{
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = sockfd;
	int value = epoll_ctl(this->epollfd, EPOLL_CTL_ADD, sockfd, &ev);

	return value;
}

int EpollServer::deleteEvent(int sockfd, int events)
{
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = sockfd;
    int value = epoll_ctl(this->epollfd, EPOLL_CTL_DEL, sockfd, &ev);

    return value;
}

int EpollServer::modifyEvent(int sockfd, int events)
{
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = sockfd;
    int value = epoll_ctl(this->epollfd, EPOLL_CTL_MOD, sockfd, &ev);

    return value;
}

int EpollServer::closeSocket(int socketfd)
{
    deleteEvent(socketfd, EPOLLIN | EPOLLET);
    close(socketfd);
    delClient(socketfd);
    return 0;
}

int EpollServer::shutDown(int socketfd)
{
    shutdown(socketfd, SHUT_RDWR);
    delClient(socketfd);
    return 0;
}

void EpollServer::onAccept()
{
    //接受新连接
    struct sockaddr_in clientaddr;
    socklen_t clientaddrlen = sizeof(clientaddr);
    int clientfd = accept(this->listenfd, (struct sockaddr*)&clientaddr, &clientaddrlen);
    if (clientfd != -1)
    {
        int oldSocketFlag = fcntl(clientfd, F_GETFL, 0);
        int newSocketFlag = oldSocketFlag | O_NONBLOCK;
        if (fcntl(clientfd, F_SETFL, newSocketFlag) == -1)
        {
            close(clientfd);
            cout << "set clientfd to nonblocking error." << endl;
        }
        else
        {

            if (addEvent(clientfd, EPOLLIN | EPOLLOUT | EPOLLET) != -1)
            {
                cout << "new client accepted,clientfd: " << clientfd << endl;
                addClient(clientfd);
            }
            else
            {
                cout << "add client fd to epollfd error" << endl;
                close(clientfd);
            }
        }
    }
    else {
        cout << "accept client failed" << endl;
    }
}

void EpollServer::onRecv(int socketfd)
{
    //cout << "client fd: " << socketfd << " recv data." << endl;
    //普通clientfd
    char recvbuf[2048] = { 0 };

    //读取数据
    int m = recv(socketfd, recvbuf, 2048, 0);
    if (m == 0)
    {
        //对端关闭了连接，从epollfd上移除clientfd
        if (deleteEvent(socketfd, NULL) != -1)
        {
            cout << "client disconnected,clientfd:" << socketfd << endl;
        }
        closeSocket(socketfd);
    }
    else if (m < 0)
    {
        //出错
        if (errno != EWOULDBLOCK && errno != EINTR)
        {
            if (deleteEvent(socketfd, NULL) != -1)
            {
                cout << "client disconnected,clientfd:" << socketfd << endl;
            }
            closeSocket(socketfd);
        }
    }
    else
    {
        //正常收到数据
        cout << "recv from client:" << socketfd << ", " << recvbuf << endl;

        /*int ret = send(7, "1234567890q", 11, 0);*/
        pools->enqueue(
            [this, recvbuf]() {
                for (auto item : this->getClients()) {
                    cout << "broadcast " << item << endl;
                    send(item, recvbuf, strlen(recvbuf), 0);
                }
            }
        );

        if (modifyEvent(socketfd, EPOLLIN | EPOLLOUT | EPOLLET) != -1)
        {
            cout << "epoll_ctl successfully, mode: EPOLL_CTL_MOD, clientfd:" << socketfd << endl;
        }
    }
}



