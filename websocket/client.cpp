///**
// * TCP客户端通信基本流程
// * zhangyl 2018.12.13
// */
//#include <sys/types.h> 
//#include <sys/socket.h>
//#include <arpa/inet.h>
//#include <unistd.h>
//#include <iostream>
//#include <string.h>
//
//#define SERVER_ADDRESS "127.0.0.1"
//#define SERVER_PORT     3000
//#define SEND_DATA       "helloworld"
//
//int main(int argc, char* argv[])
//{
//	//1.创建一个socket
//	int clientfd = socket(AF_INET, SOCK_STREAM, 0);
//	if (clientfd == -1)
//	{
//		std::cout << "create client socket error." << std::endl;
//		return -1;
//	}
//
//	//2.连接服务器
//	struct sockaddr_in serveraddr;
//	serveraddr.sin_family = AF_INET;
//	serveraddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
//	serveraddr.sin_port = htons(SERVER_PORT);
//	if (connect(clientfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1)
//	{
//		std::cout << "connect socket error." << std::endl;
//		close(clientfd);
//		return -1;
//	}
//
//	//3. 向服务器发送数据
//	int ret = send(clientfd, SEND_DATA, strlen(SEND_DATA), 0);
//	if (ret != strlen(SEND_DATA))
//	{
//		std::cout << "send data error." << std::endl;
//		close(clientfd);
//		return -1;
//	}
//
//	std::cout << "send data successfully, data: " << SEND_DATA << std::endl;
//
//	//5. 关闭socket
//	//close(clientfd);
//	//这里仅仅是为了让客户端程序不退出
//
//	char recvbuf[11];
//	while (true) {
//		int recvlen = recv(clientfd, recvbuf, 11, 0);
//		if (recvlen > 0) std::cout << recvbuf << std::endl;
//		else if (recvlen == 0) std::cout << "recv 0, closeed!" << std::endl;
//		else std::cout << "error" << std::endl;
//	}
//
//	while (true)
//	{
//		sleep(3);
//	}
//
//	return 0;
//}