
#ifndef __BC_NET_H__
#define __BC_NET_H__

extern char g_rev_buf[256];//���ջ���
void bc_net_init();  //�����ʼ��
void bc_net_send(char *szBuff);  //����һ���ַ���
void bc_net_deinit();//����ȥ��ʼ��

#endif
