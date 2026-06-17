/**
  ******************************************************************************
  * @file    mc_tasks.h
  * @author  Motor Control SDK Team, ST Microelectronics
  * @brief   This file implementes tasks definition.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  * @ingroup MCTasks
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MCTASKS_H
#define MCTASKS_H

/* Includes ------------------------------------------------------------------*/
#include "mc_interface.h"
/** @addtogroup MCSDK
  * @{
  */

/** @defgroup MCTasks Motor Control Tasks
  *
  * @brief Motor Control subsystem configuration and operation routines.
  *
  * @{
  */

/**
  * @brief  初始化电机控制核心模块。
  * @param  pMCIList 指向 MCI 句柄数组的指针，后面初始化完会把各电机的控制句柄放进去。
  * @retval None
  */
void MCboot(MCI_Handle_t* pMCIList[]);
/**
  * @brief  统一调度电机控制相关任务。
  * @note   一般放在系统周期任务里调用，比如 SysTick 或中断/轮询调度入口。
  * @retval None
  */
void MC_RunMotorControlTasks(void);

/**
  * @brief  执行中频任务调度。
  * @note   这里主要负责速度环、状态机、实验模式串口输出等中频逻辑。
  * @retval None
  */
void MC_Scheduler(void);

/**
  * @brief  执行安全检查任务。
  * @note   主要检查母线电压、温度等安全条件，必要时会触发保护。
  * @retval None
  */
void TSK_SafetyTask(void);
/**
  * @brief  执行高频任务。
  * @retval uint8_t 通常返回 1 表示本次任务有执行成功，具体含义由工程内部约定。
  */
uint8_t TSK_HighFrequencyTask(void);
/**
  * @brief  处理启动/停止按键回调。
  * @retval None
  */
void UI_HandleStartStopButton_cb (void);
/**
  * @brief  双电机场景下更新 FIFO，提前安排 FOC 执行。
  * @param  Motor 电机编号。
  * @retval None
  */
void TSK_DualDriveFIFOUpdate(uint8_t Motor);

typedef enum
{
  MC_SPEED_FILTER_NONE = 0,
  MC_SPEED_FILTER_LPF1 = 1,
  MC_SPEED_FILTER_MOVING_AVG = 2,
  MC_SPEED_FILTER_WEIGHTED_MOVING_AVG = 3,
  MC_SPEED_FILTER_ADAPTIVE_LPF = 4
} MC_SpeedFilterMode_t;

/**
  * @brief  设置速度滤波模式。
  * @param  mode 速度滤波模式枚举值。
  * @retval None
  */
void MC_SPEED_SetFilterMode(MC_SpeedFilterMode_t mode);
/**
  * @brief  获取当前速度滤波模式。
  * @retval 当前滤波模式。
  */
MC_SpeedFilterMode_t MC_SPEED_GetFilterMode(void);
/**
  * @brief  设置一阶低通滤波的移位参数。
  * @param  shift 移位值，越大代表滤波越重。
  * @retval None
  */
void MC_SPEED_SetLpfShift(uint8_t shift);
/**
  * @brief  获取当前低通滤波移位参数。
  * @retval 当前移位值。
  */
uint8_t MC_SPEED_GetLpfShift(void);
/**
  * @brief  获取原始速度值（内部单位）。
  * @retval 原始机械速度，内部单位。
  */
int16_t MC_SPEED_GetRawSpeedUnit(void);
/**
  * @brief  获取滤波后的速度值（内部单位）。
  * @retval 滤波后的机械速度，内部单位。
  */
int16_t MC_SPEED_GetFilteredSpeedUnit(void);
/**
  * @brief  获取原始速度值（rpm）。
  * @retval 原始机械速度，单位 rpm。
  */
int16_t MC_SPEED_GetRawSpeedRpm(void);
/**
  * @brief  获取滤波后的速度值（rpm）。
  * @retval 滤波后的机械速度，单位 rpm。
  */
int16_t MC_SPEED_GetFilteredSpeedRpm(void);
/**
  * @brief  运行实验模式的后台任务。
  * @note   主要负责串口文本/CSV 输出、自动切换下一组参数等。
  * @retval None
  */
void MC_EXPERIMENT_BackgroundTask(void);

/**
  * @brief  硬故障时将电机控制系统切到安全状态。
  * @retval None
  */
void TSK_HardwareFaultTask(void);

/**
  * @brief  锁定电机控制相关 GPIO，防止后续被误改配置。
  * @retval None
  */
void mc_lock_pins (void);
/**
  * @}
  */

/**
  * @}
  */

#endif /* MCTASKS_H */

/******************* (C) COPYRIGHT 2019 STMicroelectronics *****END OF FILE****/
