/*
	ET模式的EpollServer
*/

#ifndef EPOLL_SERVER_H
#define EPOLL_SERVER_H


#include <condition_variable>
#include <map>
#include <list>
#include "ThreadPool.h"

using namespace std;

class EpollServer {
private:
	int   epollfd;                //epoll句柄
	int   listenfd;                //listen句柄
	int   port;                   //端口号
	int   connectCount;           //当前连接数
	bool  isRunning;              //是否在运行

	mutex   acceptMutex;     //接受新连接
	mutex   recvMutex;       //接受数据

	condition_variable   aceeptcond;       //接受新连接
	condition_variable   recvcond;         //接受数据

	shared_ptr <thread> recv_thread;
	shared_ptr <thread> accept_thread;
	shared_ptr <thread> manager_thread;

	shared_ptr <ThreadPool> pools;         //发送数据的线程池
	size_t                       thread_nums;   //线程池线程数

	list<int>               recv_socketfds;   //接受数据存储的队列

	mutex   clientMutex;       //增删客户连接
	map<int, int> clientList;

public:
	EpollServer(size_t);
public:
	void addClient(int socket);
	void delClient(int socket);
	vector<int> getClients();

	int initSocket();
	int addEvent(int socketfd, int events);
	int deleteEvent(int sockfd, int events);
	int modifyEvent(int sockfd, int events);

	int closeSocket(int socketfd);
	int shutDown(int socketfd);

	void onAccept();
	void onRecv(int socketfd);
	void runThread();
	static void run_manager(EpollServer* epserver);
	static void run_accept(EpollServer* epserver);
	static void run_recv(EpollServer* epserver);

	void run();
};

#endif