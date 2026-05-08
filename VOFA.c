/*
 * wireless_ctrl.c
 *
 * Created on: 2026年1月23日
 * Description: 适配 zf_device_wireless_uart 的 VOFA 和控制逻辑
 */
#include "VOFA.h"

// 定义全局接收结构体
wireless_receiver_struct wireless_receiver = {0};

//================================================================================================
//  定义待调节的全局变量 (示例)
//================================================================================================
float target_speed = 0.0f;
float pid_kp = 1.5f;
float pid_ki = 0.05f;
float pid_kd = 0.0f;
int   run_mode = 1;
//PID_Param* ps;
//================================================================================================
//  函数名称：Wireless_Vofa_Send
//  功能描述：模仿 Vofa_data，通过无线串口发送波形数据
//================================================================================================
void Wireless_Vofa_Send(float data1, float data2, float data3, float data4, float data5, float data6)
{
    float data[6];
    uint8 tail[4] = {0x00, 0x00, 0x80, 0x7f}; // FireWater/JustFloat 协议尾

    data[0] = (float)data1;
    data[1] = (float)data2;
    data[2] = (float)data3;
    data[3] = (float)data4;
    data[4] = (float)data5;
    data[5] = (float)data6;

    // 使用无线串口库发送数据缓冲区
    // 注意：wireless_uart_send_buffer 内部会自动处理 RTS 流控和分包
    wireless_uart_send_buffer((uint8 *)data, sizeof(float) * 6);
    wireless_uart_send_buffer(tail, 4);
}

void Wireless_Test_Send()
{
    Wireless_Vofa_Send(1, 0, 1, 2, 3, 4);
}

//================================================================================================
//  内部函数：执行指令解析
//  cmd_line: 接收到的完整字符串，例如 "kp=10.5"
//================================================================================================
static void Execute_Command(char* cmd_line)
{
    char *name_ptr = cmd_line;
    char *val_ptr = NULL;
    char reply_buff[64];

    // 1. 寻找 '=' 符号位置
    val_ptr = strchr(cmd_line, '=');

    // 如果没有等号，视为无效指令
    if (val_ptr == NULL)
    {
        return;
    }

    // 2. 将 '=' 替换为字符串结束符 '\0'，从而将字符串切断为 "变量名" 和 "数值" 两部分
    *val_ptr = '\0';
    val_ptr++; // 指针后移一位，指向数值部分的开头

    // 3. 逐个比对变量名并赋值
    // 使用 atof 将字符串转浮点，atoi 转整型

    if (strcmp(name_ptr, "mode") == 0)
    {
        run_mode = atoi(val_ptr);
//        sprintf(reply_buff, "Set Mode OK: %d\r\n", run_mode);
    }

    if(run_mode == 0)    ps = &balance_cascade.pitch_cycle.PID_Param;//ps = &balance_cascade.angular_speed_cycle.PID_Param;
    else if(run_mode == 1) ps = &balance_cascade.pitch_cycle.PID_Param;
    else if(run_mode == 2)  ps = &balance_cascade.pitch_cycle.PID_Param;//ps = &balance_cascade.yaw_cycle.PID_Param;

    //if(!ps) return;
    if (strcmp(name_ptr, "z") == 0)
    {
        balance_cascade.cascade_value.mechanical_zero = (float)atof(val_ptr);
//        sprintf(reply_buff, "Set Speed OK: %.2f\r\n", target_speed);
    }
    else if (strcmp(name_ptr, "kp") == 0)
    {
        float temp = 0;   temp = (float)atof(val_ptr);

        if(temp > 1000)  temp = 0;//防止数据异常

        motor_pid.PID_Param.kp = temp;

//        sprintf(reply_buff, "Set Kp OK: %.3f\r\n", pid_kp);
    }
    else if (strcmp(name_ptr, "ki") == 0)
    {
        float temp = 0;   temp = (float)atof(val_ptr);

        if(fabs(temp) > 1000)  temp = 0;

        motor_pid.PID_Param.ki = temp;
       sprintf(reply_buff, "Set Ki OK: %.3f\r\n", pid_ki);
    }
    else if (strcmp(name_ptr, "kd") == 0)
    {
        float temp = 0;   temp = (float)atof(val_ptr);

        if(fabs(temp) > 1000)  temp = 0;

        motor_pid.PID_Param.kd = temp;
//       sprintf(reply_buff, "Set Kd OK: %.3f\r\n", pid_kd);
    }
    else if (strcmp(name_ptr, "l") == 0)
    {
        float temp = 0;   temp = (float)atof(val_ptr);

        if(fabs(temp) > 1000)  temp = 0;

        motor_pid.PID_Param.i_limit = temp;
       sprintf(reply_buff, "Set Kd OK: %.3f\r\n", pid_kd);
    }

    // 4. 回显结果到电脑，确认修改成功
    wireless_uart_send_string(reply_buff);
}

//================================================================================================
//  函数名称：Wireless_Command_Check
//  功能描述：优化的指令接收函数，支持批量读取，响应更快
//================================================================================================

void Wireless_Command_Check(void)
{
    static char rx_buffer[CMD_BUFFER_LEN];
    static uint8 rx_index = 0;
    uint8 ch = 0;

    // 使用 while 循环，只要硬件FIFO里有数据，就一直读，直到读空
    // 这样可以防止数据积压
    while (wireless_uart_read_buffer(&ch, 1))
    {
        if (ch == '\n' || ch == '\r') // 遇到换行符，说明一条指令结束
        {
            if (rx_index > 0)
            {
                rx_buffer[rx_index] = '\0';
                Execute_Command(rx_buffer); // 解析并执行
                rx_index = 0;
            }
        }
        else
        {
            // 存入缓冲区
            if (rx_index < CMD_BUFFER_LEN - 1)
            {
                rx_buffer[rx_index++] = (char)ch;
            }
            else
            {
                // 如果缓冲区满了还没有换行符，说明数据错乱了，强制清空
                rx_index = 0;
            }
        }
    }
}
