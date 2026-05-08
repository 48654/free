/*
 * wireless_ctrl.h
 *
 * Adapted for zf_device_wireless_uart
 */

#ifndef CODE_WIRELESS_CTRL_H_
#define CODE_WIRELESS_CTRL_H_

#include "zf_common_headfile.h"
#include "zf_device_wireless_uart.h"

//=================================================== 宏定义 ===================================================
#define WIRELESS_CH_NUM  6      // 模拟6个通道
#define WIRELESS_FRAME_HEAD 0xAA // 假设一个帧头 (根据你的实际通信协议修改)
#define WIRELESS_FRAME_TAIL 0x55 // 假设一个帧尾

#define CMD_BUFFER_LEN 64  // 指令最大长度
//=================================================== 结构体声明 ===================================================

// 模仿 uart_receiver_struct 定义无线接收结构体
typedef struct
{
    int16 channel[WIRELESS_CH_NUM]; // CH1-CH6通道数据
    uint8 state;                    // 连接状态(1表示正常，0表示失控/断连)
    uint8 finsh_flag;               // 1：表示成功接收并解析到一帧数据
} wireless_receiver_struct;

//=================================================== 变量声明 ===================================================
extern wireless_receiver_struct wireless_receiver; // 全局变量，供外部调用

// 供外部调用的待调节变量
extern float target_speed;
extern float pid_kp;
extern float pid_ki;
extern float pid_kd;
extern int   run_mode;
//extern PID_Param *ps;
//=================================================== 函数声明 ===================================================
void Wireless_Vofa_Send(float data1, float data2, float data3, float data4, float data5, float data6);
void Wireless_Test_Send(void);

// 新增：指令检查与解析函数
void Wireless_Command_Check(void);
#endif /* CODE_WIRELESS_CTRL_H_ */
