#include "EpollServer.h"

#include <iostream>
#include <sys/epoll.h>
#include <thread>

void EpollServer::runThread()
{
    this->isRunning = true;
    recv_thread.reset(new thread(EpollServer::run_recv, this));
    accept_thread.reset(new thread(EpollServer::run_accept, this));
    manager_thread.reset(new thread(EpollServer::run_manager, this));

    recv_thread->detach();
    accept_thread->detach();
    manager_thread->detach();
}

void EpollServer::run_manager(EpollServer* epserver)
{
    struct epoll_event events[2048];

    for (; ; )
    {
        int num = epoll_wait(epserver->epollfd, events, 2048, 1000);
        if (num == -1)
        {
            if (errno == EINTR) continue;
            cout << "return run manager..." << endl;
            break;
        }

        for (int i = 0; i < num; i++)
        {
            if (events[i].events & EPOLLIN) {
                if (events[i].data.fd == epserver->listenfd) {
                    //有新的连接
                    epserver->aceeptcond.notify_one();
                }
                else {
                    //有新的数据到来...
                    {
                        unique_lock<mutex> guard(epserver->recvMutex);
                        epserver->recv_socketfds.push_back(events[i].data.fd);
                    }
                    epserver->recvcond.notify_one();
                }
            }
        }
    }

    cout << "exit manager..." << endl;
}

void EpollServer::run_accept(EpollServer* epserver)
{
    cout << "run_accept..." << endl;

    while (epserver->isRunning)
    {
        {
            unique_lock<mutex> guard(epserver->acceptMutex);
            epserver->aceeptcond.wait(guard);
        }
        epserver->onAccept();
    }

    cout << "exit_accept..." << endl;
}

void EpollServer::run_recv(EpollServer* epserver)
{
    cout << "run_recv..." << endl;

    int socketfd = -1;

    while (epserver->isRunning)
    {
        {
            unique_lock<mutex> guard(epserver->recvMutex);
            while (epserver->recv_socketfds.empty())
            {
                epserver->recvcond.wait(guard);
            }

            socketfd = epserver->recv_socketfds.front();
            epserver->recv_socketfds.pop_front();
        }

        epserver->onRecv(socketfd);
    }

    cout << "exit_recv..." << endl;
}

void EpollServer::run()
{
    this->initSocket();
    this->runThread();
}