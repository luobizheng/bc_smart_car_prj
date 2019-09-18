
#ifndef __BC_CAR_H__
#define __BC_CAR_H__

void bc_car_init();  //初始化
void bc_car_run();  //直行
void bc_car_stop(); //停止
void bc_car_turn_l(int angle);  //左转
void bc_car_turn_r(int angle);  //右转
void bc_car_deinit();     //去初始化

#endif
