/*
	ETģʽ��EpollServer
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
	int   epollfd;                //epoll���
	int   listenfd;                //listen���
	int   port;                   //�˿ں�
	int   connectCount;           //��ǰ������
	bool  isRunning;              //�Ƿ�������

	mutex   acceptMutex;     //����������
	mutex   recvMutex;       //��������

	condition_variable   aceeptcond;       //����������
	condition_variable   recvcond;         //��������

	shared_ptr <thread> recv_thread;
	shared_ptr <thread> accept_thread;
	shared_ptr <thread> manager_thread;

	shared_ptr <ThreadPool> pools;         //�������ݵ��̳߳�
	size_t                       thread_nums;   //�̳߳��߳���

	list<int>               recv_socketfds;   //�������ݴ洢�Ķ���

	mutex   clientMutex;       //��ɾ�ͻ�����
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