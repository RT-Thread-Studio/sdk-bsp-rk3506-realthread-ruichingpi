/**
 * @file motor_control.h
 * @brief 
 * @author HLY (1425075683@qq.com)
 * @version 1.0
 * @date 2022-11-17
 * @copyright Copyright (c) 2022
 * @attention 
 * @par 修改日志:
 * Date       Version Author  Description
 * 2022-11-17 1.0     HLY     first version
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MOTOR_CONTROL_H
#define __MOTOR_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "canfestival.h"
#include <stdbool.h>
/* Exported types ------------------------------------------------------------*/
/*0x6040控制指令 Controlword 状态位定义*/
typedef enum
{
  WRITE_SWITCH_ON             = 1 << 0,//按下开关
  EN_VOLTAGE                  = 1 << 1,//使能电源
  QUICK_STOP                  = 1 << 2,//急停
  EN_OPERATION                = 1 << 3,//操作使能
  FAULT_RESET                 = 1 << 7,//错误重置
  HALT                        = 1 << 8,//暂停
}CONTROL_WORD;
/*0x6041状态位 Statusword定义*/
typedef enum
{
  READY_TO_SWITCH_ON          = 0,//准备功能启动
  READ_SWITCH_ON              = 1,//伺服准备完成
  OPERATION_ENABLED           = 2,//伺服使能
  FAULT                       = 3,//异常信号
  VOLTAGE_ENABLED             = 4,//伺服输入侧已供电
  READ_QUICK_STOP             = 5,//紧急停止 [0:开启急停 1:关闭急停]
  SWITCH_ON_DISABLED          = 6,//伺服准备功能关闭
  WARNING                     = 7,//警告信号
  REMOTE                      = 9,//远程控制
  TARGET_REACHED              = 10,//目标到达
  POSITIVE_LIMIT              = 14,//正向运转禁止极限
  NEGATIVE_LIMIT              = 15,//负向运转禁止极限
}STATUS_WORD;
/*0X6060 模式设定Modes of operation定义*/
typedef enum
{
  PROFILE_POSITION_MODE       = 1,//位置规划模式
  PROFILE_VELOCITY_MODE       = 3,//速度规划模式
  PROFILE_TORQUE_MODE         = 4,//扭矩规划模式
  HOMING_MODE                 = 6,//原点复归模式
  INTERPOLATED_POSITION_MODE  = 7,//插补位置模式
}MODE_OPERATION;
/*位置规划模式下Controlword操作模式特定位*/
typedef enum
{
  NEW_SET_POINT               = 1 << 4,//命令触发(正缘触发)
  CHANGE_SET_IMMEDIATELY      = 1 << 5,//命令立即生效指令  设为 0，关闭命令立即生效指令
  ABS_REL                     = 1 << 6,//更改为绝对定位或相对定位  0，目标位置是一个绝对值 1，目标位置是一个相对值
}PROFILE_POSITION_CONTROLWORD;
/*位置规划模式下Statusword定义*/
typedef enum
{
  SET_POINT_ACKNOWLEDGE       = 12,//伺服收到命令信号
  FOLLOWING_ERROR             = 13,//追随错误
}PROFILE_POSITION_STATUSWORD;
/*插补位置模式下Controlword操作模式特定位
  Name          Value   Description
Enable ip mode    0     Interpolated position mode inactive 
                  1     Interpolated position mode active*/
typedef enum
{
   ENABLE_IP_MODE             = 1 << 4,//使能IP模式
}Interpolated_Position_CONTROLWORD;
/*插补位置模式下Statusword定义
    Name        Value   Description
ip mode active    1     Interpolated position mode active
*/
typedef enum
{
  IP_MODE_ACTIVE              = 12,//插补位置模式是否生效
}Interpolated_Position_STATUSWORD;
/*原点复归模式下Controlword操作模式特定位
  Name          Value   Description
Homing          0       Homing mode inactive
operation       0 → 1  Start homing mode
star            1       Homing mode active
                1 → 0  Interrupt homing mode*/
typedef enum
{
  HOMING_OPERATION_STAR       = 1 << 4,//使能IP模式
}HOMING_CONTROLWORD;
/*原点复归模式下Statusword定义
    Name          Value   Description
Homing attained    0      Homing mode not yet completed.
                   1      Homing mode carried out successfully

Homing error       0      No homing error
                   1      Homing error occurred;Homing mode carried out not successfully;The error cause is found by reading the error code
*/
typedef enum
{
  HOMING_ATTAINED              = 12,//回到原点
  HOMING_ERROR                 = 13,//回原错误
}HOMING_STATUSWORD;

/*
 * INTEGER8地图变量结构体
*/
typedef struct
{
  INTEGER8 *map_val; //变量地址
  UNS16 index;   //变量索引
}Map_Val_INTEGER8;
/*
 * UNS16地图变量结构体
*/
typedef struct
{
  UNS16 *map_val; //变量地址
  UNS16 index;   //变量索引
}Map_Val_UNS16;
/*
 * UNS32地图变量结构体
*/
typedef struct
{
  UNS32 *map_val; //变量地址
  UNS16 index;   //变量索引
}Map_Val_UNS32;
/*
 * INTEGER32地图变量结构体
*/
typedef struct
{
  INTEGER32 *map_val; //变量地址
  UNS16 index;   //变量索引
}Map_Val_INTEGER32;
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variables ---------------------------------------------------------*/
extern Map_Val_UNS16 Controlword_Node[];
extern Map_Val_INTEGER8 Modes_of_operation_Node[];
/* Exported functions prototypes ---------------------------------------------*/
extern UNS8 motor_on_profile_position(UNS8 nodeId);
extern UNS8 motor_on_interpolated_position(UNS8 nodeId);
extern UNS8 motor_on_homing_mode(int32_t offset,uint8_t method,float switch_speed,float zero_speed,UNS8 nodeId);
extern UNS8 motor_on_profile_velocity(UNS8 nodeId);
/******************************运动模式操作函数******************************************************************/
extern UNS8 motor_profile_position(int32_t position,float speed,bool abs_rel,bool immediately,UNS8 nodeId);
extern UNS8 motor_interpolation_position (UNS8 nodeId);
extern UNS8 motor_homing_mode (bool zero_flag,int16_t speed,UNS8 nodeId);
extern UNS8 motor_profile_velocity(int16_t speed,UNS8 nodeId);
/******************************运动关闭及查询函数******************************************************************/
extern UNS8 motor_off(UNS8 nodeId);
extern UNS16 motor_get_controlword(UNS8 nodeId);
extern UNS16 motor_get_statusword(UNS8 nodeId);
extern INTEGER32 *motor_get_position(INTEGER32* des,UNS8 nodeId);
extern INTEGER32 *motor_get_velocity(INTEGER32* des,UNS8 nodeId);

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_CONTROL_H */
