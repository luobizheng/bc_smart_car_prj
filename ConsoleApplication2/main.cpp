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

/***************************************************************************
模块层次依赖关系
----------
tg				|
----------
car | TG	|
----------
net |		|
----------
****************************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <windows.h> 
#include <Mmsystem.h>             	
#pragma comment(lib, "Winmm.lib")    
#include <cstring>
#include <cstdio>
#include <algorithm> 
using namespace std;

#include "bc_log.h"
#include "bc_car.h"
#include "thinkgear.h"
/************************************************************************************
* Private Types
************************************************************************************/

/************************************************************************************
* Private Data
************************************************************************************/
int	connectionId = 0;
int val_signal,val_attention,val_meditation,val_raw;
DWORD mouth_time[2];
int mouth_index=0;

HANDLE hThread_tg_process_dat;
DWORD dwThreadId_tg_process_dat;
/************************************************************************************
* Private Functions
************************************************************************************/

void bc_tg_init()
{
    int   dllVersion   = 0;
   	char *comPortName  = NULL;
    int   errCode      = 0;

    /* Print driver version number */
    dllVersion = TG_GetDriverVersion();
    printf( "ThinkGear DLL version: %d\n", dllVersion );
    
    /* Get a connection ID handle to ThinkGear */
    connectionId = TG_GetNewConnectionId();
    if( connectionId < 0 ) 
    {
        fprintf( stderr, "ERROR: TG_GetNewConnectionId() returned %d.\n",connectionId );
        wait_exit();
    }
    
    /* Set/open stream (raw bytes) log file for connection */
    errCode = TG_SetStreamLog( connectionId, "streamLog.txt" );
    if( errCode < 0 ) 
    {
        fprintf( stderr, "ERROR: TG_SetStreamLog() returned %d.\n", errCode );
        wait_exit();
    }
    
    /* Set/open data (ThinkGear values) log file for connection */
    errCode = TG_SetDataLog( connectionId, "dataLog.txt" );
    if( errCode < 0 ) 
    {
        fprintf( stderr, "ERROR: TG_SetDataLog() returned %d.\n", errCode );
        wait_exit();
    }
    
    /* Attempt to connect the connection ID handle to serial port "COM5" */
    /* NOTE: On Windows, COM10 and higher must be preceded by \\.\, as in
     *       "\\\\.\\COM12" (must escape backslashes in strings).  COM9
     *       and lower do not require the \\.\, but are allowed to include
     *       them.  On Mac OS X, COM ports are named like
     *       "/dev/tty.MindSet-DevB-1".
     */
    comPortName = "\\\\.\\COM8";
    errCode = TG_Connect( connectionId,
                         comPortName,
                         TG_BAUD_57600,
                         TG_STREAM_PACKETS );
    if( errCode < 0 ) 
    {
				printf("错误 ERROR TG_Connect():returned %d--%s:%d\n",errCode, __FUNCTION__, __LINE__);
        wait_exit();
    }
    else
    {
    		printf("成功 SUCCEED: TG_Connect()--%s:%d\n", __FUNCTION__, __LINE__);
    }

		Sleep(1000);

		#if 0
    
		// 启动眨眼检测
		if (0 == TG_EnableBlinkDetection(connectionId, 1)) 	
		{
			printf("Success enalbe blink");
		}

		#endif
}


DWORD WINAPI bc_tg_process_dat_pthread(LPVOID lpParam)
{

	while(1)
	{
		if(val_signal < 50) //信号质量检测
		{
			if(mouth_index==1)	//单次
			{
				if( GetTickCount()- mouth_time[0] > 2000 )
				{
					mouth_index=0;//清除索引--开始转弯
					bc_car_turn_l(20);
					bc_car_stop();	//转弯完之后停止
				}
			}

			if(mouth_index>=2)		//双次
			{
				mouth_index=0;		//清除索引--开始转弯
				bc_car_turn_r(20);	
				bc_car_stop();	//转弯完之后停止
			}
		}
		else
		{
			//信号质量低 则停车
			bc_car_stop();	
			printf("err signal poor\n");	
		}

		Sleep(10);
	}

	return 0;
}


void bc_tg_filter_dat_pthread()
{

	//tg数据采集主线程
	DWORD start_time=GetTickCount();  //程序运行时间
	while(GetTickCount() - start_time < 3000*1000)
	{
		int packetsRead = 0;

		/* Read a single Packet from the connection */
		packetsRead = TG_ReadPackets( connectionId, 1 );


		/* If TG_ReadPackets() was able to read a Packet of data... */
		if( packetsRead == 1 ) 
		{

			if( TG_GetValueStatus(connectionId, TG_DATA_POOR_SIGNAL) != 0 )
			{
				val_signal = (int)TG_GetValue(connectionId, TG_DATA_POOR_SIGNAL);
				printf("val_signal:%d\n",val_signal);
					
			}

			#if 0
			if( TG_GetValueStatus(connectionId, TG_DATA_ATTENTION) != 0 )
			{
				val_attention = (int)TG_GetValue(connectionId, TG_DATA_ATTENTION);
				printf("val_attention:%d\n",val_attention);
			}

			if( TG_GetValueStatus(connectionId, TG_DATA_MEDITATION) != 0 )
			{
				val_meditation = (int)TG_GetValue(connectionId, TG_DATA_MEDITATION);
				printf("val_meditation:%d\n",val_meditation);
				
			}
			#endif

			#if 0

			if( TG_GetValueStatus(connectionId, TG_DATA_BLINK_STRENGTH) != 0 )
			{
				val_meditation = (int)TG_GetValue(connectionId, TG_DATA_BLINK_STRENGTH);
				printf("TG_DATA_BLINK_STRENGTH:%d\n",val_meditation);
				
			}

			#endif


			if( TG_GetValueStatus(connectionId, TG_DATA_RAW) != 0 )
			{
				val_raw = (int)TG_GetValue(connectionId, TG_DATA_RAW);


				#if 1
				//printf  dat

				if(  ( val_raw < -900 )  || (900 < val_raw )  )
				{
					printf("%d TG_DATA_RAW:%d\n",GetTickCount(),val_raw);
				}
				#endif


				//blink test
				#define BLINK_LEVEL	800
				
				typedef enum 
				{
					BLINK_SIGNAL_IDEL,
					BLINK_SIGNAL_H,
					BLINK_SIGNAL_L,
				}blink_signal_status_t;

				static blink_signal_status_t blink_signal_status=BLINK_SIGNAL_IDEL;
				static DWORD blink_signal_start_time=0;
				static int blink_signal_cnt=0;

				if((blink_signal_status==BLINK_SIGNAL_IDEL) && (BLINK_LEVEL<val_raw)) //初始状态
				{
					blink_signal_status=BLINK_SIGNAL_H;
					blink_signal_start_time=GetTickCount();
					blink_signal_cnt++;
				}
				else if(blink_signal_status==BLINK_SIGNAL_H)	//高电平  8个 判断
				{
					if(BLINK_LEVEL<val_raw)
					{						
						blink_signal_cnt++;
						if( blink_signal_cnt==8 )
						{
							blink_signal_status=BLINK_SIGNAL_L;
							blink_signal_cnt=0;
						}
					}
					else if(val_raw<-BLINK_LEVEL)//非连续性抛掉
					{
						blink_signal_status=BLINK_SIGNAL_IDEL;
						blink_signal_cnt=0;
						blink_signal_start_time=0;
						printf("clear blink noise\n");
					}
				}
				else if(blink_signal_status==BLINK_SIGNAL_L)	//低电平  16个 判断
				{
					if(val_raw<-BLINK_LEVEL)
					{
						blink_signal_cnt++;
						if( ( blink_signal_cnt==12 ) && ( GetTickCount() - blink_signal_start_time < 400 ) )
						{
							blink_signal_status=BLINK_SIGNAL_IDEL;
							blink_signal_cnt=0;
							blink_signal_start_time=0;


							static int blink_test_cnt=1;
							printf("Get blink:%d\n",blink_test_cnt++);	
							if(blink_test_cnt%2==0)
							{
								bc_car_run();
							}
							else
							{
								bc_car_stop();
							}

							
						}							
					}
					else if((val_raw>BLINK_LEVEL) && (blink_signal_cnt!=0))//非连续性抛掉
					{
						blink_signal_status=BLINK_SIGNAL_IDEL;
						blink_signal_cnt=0;
						blink_signal_start_time=0;
						printf("clear blink noise\n");							
					}
				}					

				if( (blink_signal_status!=BLINK_SIGNAL_IDEL) && ( GetTickCount() - blink_signal_start_time > 400 ) )//超时噪音
				{
					blink_signal_status=BLINK_SIGNAL_IDEL;
					blink_signal_cnt=0;
					blink_signal_start_time=0;
					printf("clear blink noise\n");
				}


				//mouth test

				#define MOUTN_LEVEL 800
				
				typedef enum 
				{
					SIGNAL_IDEL,
					SIGNAL_H1,
					SIGNAL_L1,
					SIGNAL_H2,
					SIGNAL_L2,
				}mouth_signal_status_t;

				static DWORD mouth_signal_start_time=0;
				static mouth_signal_status_t mouth_signal_st=SIGNAL_IDEL;
				static int mouth_signal_cnt=0;

				if( (mouth_signal_st==SIGNAL_IDEL) && (MOUTN_LEVEL<val_raw) )		//初始状态
				{
					mouth_signal_st=SIGNAL_H1;
					mouth_signal_start_time=GetTickCount();
					mouth_signal_cnt++;
				}
				else if( (mouth_signal_st==SIGNAL_H1) && (MOUTN_LEVEL<val_raw) )	//高电平1  判定
				{
					mouth_signal_cnt++;
					
					if(mouth_signal_cnt==2)
					{
						mouth_signal_st=SIGNAL_L1;
						mouth_signal_cnt=0;
					}
				}
				else if( (mouth_signal_st==SIGNAL_L1) && (val_raw<-MOUTN_LEVEL) )//低电平1  判定
				{
					mouth_signal_cnt++;
				
					if(mouth_signal_cnt==2)
					{
						mouth_signal_st=SIGNAL_H2;
						mouth_signal_cnt=0;
					}
				}
				else if( (mouth_signal_st==SIGNAL_H2) && (MOUTN_LEVEL<val_raw) )	//高电平2  判定
				{
					mouth_signal_cnt++;
				
					if(mouth_signal_cnt==2)
					{
						mouth_signal_st=SIGNAL_L2;
						mouth_signal_cnt=0;
					}
				}
				else if( (mouth_signal_st==SIGNAL_L2) && (val_raw<-MOUTN_LEVEL) )	//高电平2  判定
				{
					mouth_signal_cnt++;
					
					if( (mouth_signal_cnt==2) && ((GetTickCount()-mouth_signal_start_time)<200) )
					{
						mouth_signal_st=SIGNAL_IDEL;
						mouth_signal_cnt=0;
						mouth_signal_start_time=0;

						static int mouth_test_cnt=1;
						printf("Get mouth:%d\n",mouth_test_cnt++);

						if(mouth_index<=1)
						{
							mouth_time[mouth_index]=GetTickCount();
							mouth_index++;
						}
						else
						{
							//mouth_time[0]=GetTickCount();
							//mouth_index=1;
							printf("err--Get blink but buf overflow\n");
						}

					}
				}

				if((mouth_signal_st!=SIGNAL_IDEL) && (200<(GetTickCount()-mouth_signal_start_time))) //噪声清除
				{
					mouth_signal_st=SIGNAL_IDEL;
					mouth_signal_cnt=0;
					mouth_signal_start_time=0;

					printf("clear mouth noise\n");
				}	
			}
		} /* end "If TG_ReadPackets() was able to read a Packet..." */
	}
}

void bc_tg_deinit()
{	     
	/* Clean up */
	TG_FreeConnection( connectionId );
	CloseHandle(hThread_tg_process_dat);	//关闭线程句柄，内核引用计数减一	
}
/************************************************************************************
*  Public Functions
************************************************************************************/
int main( void ) 
{
	bc_car_init();
	bc_tg_init();

	//tg数据处理线程
	hThread_tg_process_dat=CreateThread(NULL,NULL,bc_tg_process_dat_pthread,NULL,0,&dwThreadId_tg_process_dat);


	//tg数据获取线程--阻塞至设定时间
	bc_tg_filter_dat_pthread();

	
	bc_car_deinit();
	bc_tg_deinit();
    
	/* End program */
	wait_exit();
	return( EXIT_SUCCESS );

}




#if 0

int main(int argc, char* argv[])
{
	//bc_net_init();
	//bc_net_send("test----------------\n");

	//Sleep(200);
	
	bc_car_init();

	bc_car_run();
	Sleep(1000);

	bc_car_run();
	Sleep(1000);

	//bc_car_run();
	Sleep(1000);

	bc_car_run();
	Sleep(1000);
	
	bc_car_run();
	Sleep(1000);

	//bc_car_set_speed(100);
	//Sleep(10*1000);

	//bc_car_stop();

	Sleep(2000);
	bc_car_turn_r(20);	

	Sleep(2*1000);
	
	bc_car_turn_l(20);
	Sleep(2*1000);

	bc_car_run();
	Sleep(1000);

	bc_car_run();
	Sleep(1000);

	//bc_car_deinit();

	system("pause");

	return 0;
}

#endif


