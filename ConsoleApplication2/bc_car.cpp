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

#include "bc_log.h"
#include "bc_net.h"
/************************************************************************************
* Private Types
************************************************************************************/

/************************************************************************************
* Private Data
************************************************************************************/
HANDLE hThread_car_speed;
DWORD dwThreadId_car_speed;
int g_bc_car_live=0;
int g_bc_car_speed_mode=0;  // 1:H  0:L
/************************************************************************************
* Private Functions
************************************************************************************/
DWORD WINAPI ThreadProFunc_car_speed(LPVOID lpParam)
{
	/*		
	cmd=control&d=
	0 ֹͣ
	1 ǰ��
	2 ����
	3	��ת
	4	��ת
	5	��ȡ�ٶ�
	6	����
	7	����
	*/
	while(1)
	{

		if(g_bc_car_live > 0)
		{
			bc_net_send("cmd=control&d=1");

			do
			{
				g_bc_car_live -= 100;
			
				Sleep(100);
			}while( g_bc_car_live > 0 );
		
			bc_net_send("cmd=control&d=0");
		}

		Sleep(10);
	}

	return 0;
}

void set_car_speed_l()
{
	if(g_bc_car_speed_mode==1)
	{
		for(int i=0;i<9;i++)
		{	
			bc_net_send("cmd=control&d=7");
			Sleep(30);
		}
		g_bc_car_speed_mode=0;
	}
}

void set_car_speed_h()
{
	if(g_bc_car_speed_mode==0)
	{
		for(int i=0;i<12;i++)
		{	
			bc_net_send("cmd=control&d=6");
			Sleep(30);
		}
		g_bc_car_speed_mode=1;
	}
}

/************************************************************************************
*  Public Functions
************************************************************************************/
void bc_car_init()
{
	bc_net_init();

	//����car
	Sleep(200);		//�ȴ������߳�����
	for(int i=0;i<3;i++)
	{
		bc_net_send("cmd=control&d=5");
		Sleep(2000);

		if( strncmp(g_rev_buf,"CMD OK",3) == 0)
		{
			printf("%s--%s:%d\n", "�ɹ� car connected ok",__FUNCTION__, __LINE__);
			break;
		}

		if(i==2)
		{
			printf("���� car not connected--%s:%d\n",__FUNCTION__, __LINE__);
			wait_exit();
		}
	}

	//����Ϊ����
	set_car_speed_h();

	#if 0

	//�˶�ι���߳�
	hThread_car_speed = CreateThread(NULL	// Ĭ�ϰ�ȫ����
		, NULL		// Ĭ�϶�ջ��С
		, ThreadProFunc_car_speed // car �ٶ�ά���߳�
		, NULL	//���ݸ��̺߳����Ĳ���
		, 0		// ָ���߳���������
		, &dwThreadId_car_speed	//�߳�ID��
	);

	#endif

}

void bc_car_run()
{
	set_car_speed_l();
	bc_net_send("cmd=control&d=1");
	printf("%s--%s:%d\n","set car run",__FUNCTION__, __LINE__);
}

void bc_car_stop()
{
	bc_net_send("cmd=control&d=0");
	printf("%s--%s:%d\n","set car stop",__FUNCTION__, __LINE__);
}

//����
void bc_car_turn_l(int angle)
{
	printf("tunr L:%d--%s:%d\n",angle,__FUNCTION__, __LINE__);
	bc_car_stop();
	set_car_speed_h();

	bc_net_send("cmd=control&d=3");
	Sleep(angle*10);
	bc_net_send("cmd=control&d=0");

	set_car_speed_l();
}

void bc_car_turn_r(int angle)
{
	printf("tunr R:%d--%s:%d\n",angle,__FUNCTION__, __LINE__);

	bc_car_stop();
	set_car_speed_h();
	
	bc_net_send("cmd=control&d=4");
	Sleep(angle*10);
	bc_net_send("cmd=control&d=0");

	set_car_speed_l();
}

void bc_car_deinit()
{
	bc_net_deinit();
	CloseHandle(hThread_car_speed);	//�ر��߳̾�����ں����ü�����һ	
}

