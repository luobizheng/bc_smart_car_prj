
#ifndef __BC_NET_H__
#define __BC_NET_H__

extern char g_rev_buf[256];//接收缓冲
void bc_net_init();  //网络初始化
void bc_net_send(char *szBuff);  //发送一个字符串
void bc_net_deinit();//网络去初始化

#endif
