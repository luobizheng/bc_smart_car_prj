/**
 ******************************************************************************
 * @file    
 * @authors  
 * @version V1.0.0
 * @date    2019_09_15
 * @brief   
 ******************************************************************************
 */
/****************************************************************************
*Private Included Files
****************************************************************************/
#include <stdio.h>
#include <WINSOCK.H>//套接字所需的头文件

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#pragma comment(lib,"WSOCK32.LIB")	//windows套接字的库文件

#include "bc_net.h"
#include "bc_log.h"

/************************************************************************************
* Private Types
************************************************************************************/
#define SOCK_VER 2 									//程序使用的winsock主版本

/************************************************************************************
* Private Data
************************************************************************************/
SOCKET g_sock = 0; 									//套接字声明				 
sockaddr_in addr = { 0 };						//发送数据的目标地址

HANDLE hThread;
DWORD dwThreadId;

char g_rev_buf[256];
/************************************************************************************
* Private Functions
************************************************************************************/
DWORD WINAPI thread_net_recv(LPVOID lpParam)
{

	while (1)
	{
		//等待回应
		sockaddr_in saServer = { 0 };
		int nFromLen = sizeof(saServer);
		int nRecv = recvfrom(g_sock, g_rev_buf, 256, 0, (sockaddr *)&saServer, &nFromLen);

		if (SOCKET_ERROR == nRecv)
		{
			printf("err recvfrom--%s:%d\n",__FUNCTION__,__LINE__);
			wait_exit();
		}
		else
		{
			g_rev_buf[nRecv]=0x00;
			printf("Car：%s-%s,%d\n", g_rev_buf, inet_ntoa(saServer.sin_addr), ntohs(saServer.sin_port));
		}

	}
	return 0;
}


/************************************************************************************
*  Public Functions
************************************************************************************/
void bc_net_init()
{	
	//初始化WinSock环境
	WSADATA wd = { 0 };
	int nStart = WSAStartup(MAKEWORD(SOCK_VER, 0), &wd);
	if (0 != nStart)
	{
		printf("%s--%s:%d\n", "err WSAStartup", __FUNCTION__, __LINE__);
		wait_exit();
	}

	if (LOBYTE(wd.wVersion) != 2)
	{
		printf("%s--%s:%d\n", "err LOBYTE", __FUNCTION__, __LINE__);
	}

	//创建一个UDP SOCKET 
	g_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == g_sock)
	{
		printf("%s--%s:%d\n", "err socket", __FUNCTION__, __LINE__);
	}

	addr.sin_family = AF_INET;//IPV4
	addr.sin_port = htons(8089);//端口
	addr.sin_addr.s_addr = inet_addr("192.168.4.1");//IP

	bc_net_send("first send");
	printf("%s--%s:%d\n", "socket 创建成功！", __FUNCTION__, __LINE__);

	//设置为阻塞模式
	int imode = 0;
	if (ioctlsocket(g_sock, FIONBIO, (u_long *)&imode) == SOCKET_ERROR)
	{
		closesocket(g_sock);
		WSACleanup();

		printf("%s--%s:%d\n", "err set socket", __FUNCTION__, __LINE__);
		wait_exit();
	}

	//接收线程
	hThread = CreateThread(NULL	// 默认安全属性
		, NULL		// 默认堆栈大小
		, thread_net_recv // 线程入口地址
		, NULL	//传递给线程函数的参数
		, 0		// 指定线程立即运行
		, &dwThreadId	//线程ID号
	);
	
}

void bc_net_send(char *szBuff)
{

	//发送数据包
	int nSent = sendto(g_sock, szBuff, strlen(szBuff) + 1, 0, (sockaddr *)&addr, sizeof(addr));
	if (0 == nSent)
	{
		printf("%s--%s:%d\n", "err sendto", __FUNCTION__, __LINE__);
	}
	else
	{
		printf("%s--%s:%d\n",szBuff, __FUNCTION__, __LINE__);
	}	
}

void bc_net_deinit()
{
	CloseHandle(hThread);	//关闭线程句柄，内核引用计数减一	
	closesocket(g_sock);	//关闭连接套接字
	WSACleanup();    //	//清理套接字环境
}

