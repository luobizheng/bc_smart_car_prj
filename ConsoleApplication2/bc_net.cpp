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
#include <WINSOCK.H>//�׽��������ͷ�ļ�

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#pragma comment(lib,"WSOCK32.LIB")	//windows�׽��ֵĿ��ļ�

#include "bc_net.h"
#include "bc_log.h"

/************************************************************************************
* Private Types
************************************************************************************/
#define SOCK_VER 2 									//����ʹ�õ�winsock���汾

/************************************************************************************
* Private Data
************************************************************************************/
SOCKET g_sock = 0; 									//�׽�������				 
sockaddr_in addr = { 0 };						//�������ݵ�Ŀ���ַ

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
		//�ȴ���Ӧ
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
			printf("Car��%s-%s,%d\n", g_rev_buf, inet_ntoa(saServer.sin_addr), ntohs(saServer.sin_port));
		}

	}
	return 0;
}


/************************************************************************************
*  Public Functions
************************************************************************************/
void bc_net_init()
{	
	//��ʼ��WinSock����
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

	//����һ��UDP SOCKET 
	g_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == g_sock)
	{
		printf("%s--%s:%d\n", "err socket", __FUNCTION__, __LINE__);
	}

	addr.sin_family = AF_INET;//IPV4
	addr.sin_port = htons(8089);//�˿�
	addr.sin_addr.s_addr = inet_addr("192.168.4.1");//IP

	bc_net_send("first send");
	printf("%s--%s:%d\n", "socket �����ɹ���", __FUNCTION__, __LINE__);

	//����Ϊ����ģʽ
	int imode = 0;
	if (ioctlsocket(g_sock, FIONBIO, (u_long *)&imode) == SOCKET_ERROR)
	{
		closesocket(g_sock);
		WSACleanup();

		printf("%s--%s:%d\n", "err set socket", __FUNCTION__, __LINE__);
		wait_exit();
	}

	//�����߳�
	hThread = CreateThread(NULL	// Ĭ�ϰ�ȫ����
		, NULL		// Ĭ�϶�ջ��С
		, thread_net_recv // �߳���ڵ�ַ
		, NULL	//���ݸ��̺߳����Ĳ���
		, 0		// ָ���߳���������
		, &dwThreadId	//�߳�ID��
	);
	
}

void bc_net_send(char *szBuff)
{

	//�������ݰ�
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
	CloseHandle(hThread);	//�ر��߳̾�����ں����ü�����һ	
	closesocket(g_sock);	//�ر������׽���
	WSACleanup();    //	//�����׽��ֻ���
}

