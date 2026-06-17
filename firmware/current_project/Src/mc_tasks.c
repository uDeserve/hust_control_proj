
/**
  ******************************************************************************
  * @file    mc_tasks.c
  * @author  Motor Control SDK Team, ST Microelectronics
  * @brief   This file implements tasks definition
  *
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
  */

/* Includes ------------------------------------------------------------------*/
//cstat -MISRAC2012-Rule-21.1
#include "main.h"
//cstat +MISRAC2012-Rule-21.1
#include "mc_type.h"
#include "mc_math.h"
#include "motorcontrol.h"
#include "regular_conversion_manager.h"
#include "mc_interface.h"
#include "digital_output.h"
#include "pwm_common.h"

#include "mc_tasks.h"
#include "parameters_conversion.h"
#include "mcp_config.h"
#include "dac_ui.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
extern UART_HandleTypeDef huart2;

/* USER CODE END Includes */

/* USER CODE BEGIN Private define */
/* 现场演示就改这一个参数。
   1) 滤波方法对比：其他东西都固定，只看滤波差异
   2) PLL 参数对比：滤波固定，不把别的因素掺进来
   3) 切换保护对比：滤波和 PLL 都固定，只看保护开关前后 */
#define MC_DEMO_MODE_FILTER_COMPARE          1U
#define MC_DEMO_MODE_PLL_COMPARE             2U
#define MC_DEMO_MODE_SWITCH_COMPARE          3U
#define MC_DEMO_MODE                         MC_DEMO_MODE_FILTER_COMPARE

/* 滤波演示时，是否顺带启用启动切换保护。
   默认关掉，保证“滤波对比”尽量干净；如果你想现场演示“同一批滤波器 + 开保护”，
   就把这里改成 1U，不用去每个配置项里逐个改。 */
#define MC_FILTER_COMPARE_STARTUP_PROTECT_ENABLE 0U

#define MC_EXPERIMENT_BASE_SPEED_RPM         ((int16_t)500)
#define MC_EXPERIMENT_BOOST_SPEED_RPM        ((int16_t)1000)
#define MC_EXPERIMENT_SPEED_HOLD_MS          ((uint16_t)6000)
#define MC_EXPERIMENT_RAMP_TIME_MS           ((uint16_t)1200)
#define MC_EXPERIMENT_SAMPLE_PERIOD_MS       ((uint16_t)20)
#define MC_EXPERIMENT_STOP_CAPTURE_MS        ((uint16_t)1200)
#define MC_EXPERIMENT_STOP_SETTLE_MS         ((uint16_t)2200)
#define MC_EXPERIMENT_TOTAL_RUN_MS           ((uint16_t)18000)
#define MC_EXPERIMENT_START_TIMEOUT_MS       ((uint16_t)2500)
#define MC_EXPERIMENT_INTER_SESSION_WAIT_MS  ((uint16_t)1000)

#define MC_STARTUP_SWITCH_MIN_SPEED_RPM      ((int16_t)120)
#define MC_STARTUP_SWITCH_STABLE_MS          ((uint16_t)120)
#define MC_STARTUP_ESTIMATE_CONFIRM_SAMPLES  ((uint16_t)5)

#define MC_EXPERIMENT_TX_BUFFER_SIZE         256U
#define MC_EXPERIMENT_QUEUE_DEPTH           96U
#define MC_EXPERIMENT_METHOD_NAME_LEN       20U
#define MC_EXPERIMENT_PARAM_NAME_LEN        40U
#define MC_SPEED_FILTER_MOVAVG_DEPTH        8U
#define MC_SPEED_FILTER_WMA_DEPTH           4U
#define MC_SPEED_FILTER_ADAPTIVE_FAST_SHIFT 2U
#define MC_SPEED_FILTER_ADAPTIVE_SLOW_SHIFT 4U
#define MC_SPEED_FILTER_ADAPTIVE_ENTER_RPM  ((int16_t)18)
#define MC_SPEED_FILTER_ADAPTIVE_EXIT_RPM   ((int16_t)40)
#define MC_SPEED_FILTER_ADAPTIVE_CONFIRM_N  64U
#define MC_EXPERIMENT_STOP_SPEED_EPS_RPM    ((int16_t)30)

/* Private define ------------------------------------------------------------*/
/* Un-Comment this macro define in order to activate the smooth
   braking action on over voltage */
/* #define  MC.SMOOTH_BRAKING_ACTION_ON_OVERVOLTAGE */

  #define CHARGE_BOOT_CAP_MS  ((uint16_t)10)
  #define CHARGE_BOOT_CAP_MS2 ((uint16_t)10)
  #define OFFCALIBRWAIT_MS    ((uint16_t)0)
  #define OFFCALIBRWAIT_MS2   ((uint16_t)0)
  #define STOPPERMANENCY_MS   ((uint16_t)400)
  #define STOPPERMANENCY_MS2  ((uint16_t)400)
  #define CHARGE_BOOT_CAP_TICKS  (uint16_t)((SYS_TICK_FREQUENCY * CHARGE_BOOT_CAP_MS) / ((uint16_t)1000))
  #define CHARGE_BOOT_CAP_TICKS2 (uint16_t)((SYS_TICK_FREQUENCY * CHARGE_BOOT_CAP_MS2)/ ((uint16_t)1000))
  #define OFFCALIBRWAITTICKS     (uint16_t)((SYS_TICK_FREQUENCY * OFFCALIBRWAIT_MS)   / ((uint16_t)1000))
  #define OFFCALIBRWAITTICKS2    (uint16_t)((SYS_TICK_FREQUENCY * OFFCALIBRWAIT_MS2)  / ((uint16_t)1000))
  #define STOPPERMANENCY_TICKS   (uint16_t)((SYS_TICK_FREQUENCY * STOPPERMANENCY_MS)  / ((uint16_t)1000))
  #define STOPPERMANENCY_TICKS2  (uint16_t)((SYS_TICK_FREQUENCY * STOPPERMANENCY_MS2) / ((uint16_t)1000))

/* USER CODE END Private define */
#define VBUS_TEMP_ERR_MASK (MC_OVER_VOLT| MC_UNDER_VOLT| MC_OVER_TEMP)
/* Private variables----------------------------------------------------------*/
static FOCVars_t FOCVars[NBR_OF_MOTORS];

static PWMC_Handle_t *pwmcHandle[NBR_OF_MOTORS];
static CircleLimitation_Handle_t *pCLM[NBR_OF_MOTORS];
//cstat !MISRAC2012-Rule-8.9_a
static RampExtMngr_Handle_t *pREMNG[NBR_OF_MOTORS];   /*!< Ramp manager used to modify the Iq ref
                                                    during the start-up switch over.*/

static uint16_t hMFTaskCounterM1 = 0; //cstat !MISRAC2012-Rule-8.9_a
static volatile uint16_t hBootCapDelayCounterM1 = ((uint16_t)0);
static volatile uint16_t hStopPermanencyCounterM1 = ((uint16_t)0);

static volatile uint8_t bMCBootCompleted = ((uint8_t)0);

/* USER CODE BEGIN Private Variables */
/* 这里单独拷一份速度传感器句柄，把滤波后的速度再送回速度环 */
static SpeednPosFdbk_Handle_t SpeedFilterSensorM1;
static MC_SpeedFilterMode_t g_mcSpeedFilterMode = MC_SPEED_FILTER_LPF1;
static uint8_t g_mcSpeedFilterLpfShift = 3U;
static int16_t g_mcRawMecSpeedUnit = 0;
static int16_t g_mcFilteredMecSpeedUnit = 0;
/* 这些状态量都是为了做不同滤波方法轮换实验 */
static int16_t g_mcMovingAvgBuffer[MC_SPEED_FILTER_MOVAVG_DEPTH];
static uint8_t g_mcMovingAvgIndex = 0U;
static int32_t g_mcMovingAvgSum = 0;
static int16_t g_mcWeightedMovingAvgBuffer[MC_SPEED_FILTER_WMA_DEPTH];
static uint8_t g_mcWeightedMovingAvgIndex = 0U;
static uint8_t g_mcAdaptiveFastShift = MC_SPEED_FILTER_ADAPTIVE_FAST_SHIFT;
static uint8_t g_mcAdaptiveSlowShift = MC_SPEED_FILTER_ADAPTIVE_SLOW_SHIFT;
static int16_t g_mcAdaptiveEnterRpm = MC_SPEED_FILTER_ADAPTIVE_ENTER_RPM;
static int16_t g_mcAdaptiveExitRpm = MC_SPEED_FILTER_ADAPTIVE_EXIT_RPM;
static uint16_t g_mcAdaptiveConfirmN = MC_SPEED_FILTER_ADAPTIVE_CONFIRM_N;
static uint8_t g_mcAdaptiveCurrentShift = MC_SPEED_FILTER_ADAPTIVE_FAST_SHIFT;
static uint16_t g_mcAdaptiveSteadyCounter = 0U;
static uint8_t g_mcPllSplitEnable = 0U;
static int16_t g_mcPllFastKp = PLL_KP_GAIN;
static int16_t g_mcPllFastKi = PLL_KI_GAIN;
static int16_t g_mcPllSlowKp = PLL_KP_GAIN;
static int16_t g_mcPllSlowKi = PLL_KI_GAIN;
static int16_t g_mcPllSplitEnterRpm = 0;
static int16_t g_mcPllSplitExitRpm = 0;
static uint16_t g_mcPllSplitConfirmN = 0U;
static uint8_t g_mcPllCurrentStage = 0U;
static uint16_t g_mcPllStableCounter = 0U;
static int16_t g_mcPllActiveKp = PLL_KP_GAIN;
static int16_t g_mcPllActiveKi = PLL_KI_GAIN;

typedef struct
{
  MC_SpeedFilterMode_t mode;
  uint8_t lpfShift;
  const char *methodName;
  const char *paramName;
  uint8_t adaptiveFastShift;
  uint8_t adaptiveSlowShift;
  int16_t adaptiveEnterRpm;
  int16_t adaptiveExitRpm;
  uint16_t adaptiveConfirmN;
  uint8_t pllSplitEnable;
  int16_t pllFastKp;
  int16_t pllFastKi;
  int16_t pllSlowKp;
  int16_t pllSlowKi;
  int16_t pllSplitEnterRpm;
  int16_t pllSplitExitRpm;
  uint16_t pllSplitConfirmN;
  uint8_t startupProtectEnable;
} MC_ExperimentConfig_t;

typedef struct
{
  uint16_t sessionId;
  uint16_t configIndex;
  uint32_t sampleIndex;
  uint32_t timeMs;
  int16_t targetSpeedRpm;
  int16_t rawSpeedRpm;
  int16_t filteredSpeedRpm;
  int16_t finalSpeedRpm;
  int16_t pllKp;
  int16_t pllKi;
  uint8_t pllStage;
  uint8_t pllSplitEnable;
  uint8_t phase;
  uint8_t stopReason;
  char methodName[MC_EXPERIMENT_METHOD_NAME_LEN];
  char paramName[MC_EXPERIMENT_PARAM_NAME_LEN];
} MC_ExperimentSample_t;

typedef enum
{
  MC_EXPERIMENT_PHASE_IDLE = 0,
  MC_EXPERIMENT_PHASE_START = 1,
  MC_EXPERIMENT_PHASE_RUN = 2,
  MC_EXPERIMENT_PHASE_STOP = 3,
  MC_EXPERIMENT_PHASE_DONE = 4
} MC_ExperimentPhase_t;

typedef enum
{
  MC_EXPERIMENT_STOP_BY_USER = 0,
  MC_EXPERIMENT_STOP_BY_FAULT = 1
} MC_ExperimentStopReason_t;

/* 这一组是给老师现场演示“滤波方法对比”用的。
   PLL 固定住；启动保护默认跟着总宏走，平时建议先关掉，这样更容易把滤波差异单独讲清楚。 */
static const MC_ExperimentConfig_t g_mcFilterCompareConfigs[] =
{
  {MC_SPEED_FILTER_NONE,       3U, "NONE",    "none",              2U, 4U, 18, 40, 64U, 1U, 245, 5, 165, 3, 17, 38, 32U, MC_FILTER_COMPARE_STARTUP_PROTECT_ENABLE},
  {MC_SPEED_FILTER_LPF1,       2U, "LPF1",    "shift2",            2U, 4U, 18, 40, 64U, 1U, 245, 5, 165, 3, 17, 38, 32U, MC_FILTER_COMPARE_STARTUP_PROTECT_ENABLE},
  {MC_SPEED_FILTER_LPF1,       3U, "LPF1",    "shift3",            2U, 4U, 18, 40, 64U, 1U, 245, 5, 165, 3, 17, 38, 32U, MC_FILTER_COMPARE_STARTUP_PROTECT_ENABLE},
  {MC_SPEED_FILTER_LPF1,       4U, "LPF1",    "shift4",            2U, 4U, 18, 40, 64U, 1U, 245, 5, 165, 3, 17, 38, 32U, MC_FILTER_COMPARE_STARTUP_PROTECT_ENABLE},
  {MC_SPEED_FILTER_MOVING_AVG, 3U, "MOVAVG8", "n8",                2U, 4U, 18, 40, 64U, 1U, 245, 5, 165, 3, 17, 38, 32U, MC_FILTER_COMPARE_STARTUP_PROTECT_ENABLE},
  {MC_SPEED_FILTER_WEIGHTED_MOVING_AVG, 3U, "WMA4", "w1-2-3-4",    2U, 4U, 18, 40, 64U, 1U, 245, 5, 165, 3, 17, 38, 32U, MC_FILTER_COMPARE_STARTUP_PROTECT_ENABLE},
  {MC_SPEED_FILTER_ADAPTIVE_LPF, 3U, "ADALPF", "2to4_h18_40_n64",  2U, 4U, 18, 40, 64U, 1U, 245, 5, 165, 3, 17, 38, 32U, MC_FILTER_COMPARE_STARTUP_PROTECT_ENABLE}
};

/* 这一组专门看 PLL 估计器自身。
   所以滤波固定成 NONE，启动保护也先关掉，避免把别的改进一起算进去。 */
static const MC_ExperimentConfig_t g_mcPllCompareConfigs[] =
{
  {MC_SPEED_FILTER_NONE, 3U, "PLL_FIX",   "kp165_ki4",                       2U, 4U, 18, 40, 64U, 0U, 165, 4, 165, 4, 18, 40, 64U, 0U},
  {MC_SPEED_FILTER_NONE, 3U, "PLL_SPLIT", "fast245_5_slow165_3_e17x38_n32", 2U, 4U, 18, 40, 64U, 1U, 245, 5, 165, 3, 17, 38, 32U, 0U},
  {MC_SPEED_FILTER_NONE, 3U, "PLL_SPLIT", "fast245_5_slow170_3_e18x40_n32", 2U, 4U, 18, 40, 64U, 1U, 245, 5, 170, 3, 18, 40, 32U, 0U},
  {MC_SPEED_FILTER_NONE, 3U, "PLL_SPLIT", "fast245_5_slow165_4_e18x40_n32", 2U, 4U, 18, 40, 64U, 1U, 245, 5, 165, 4, 18, 40, 32U, 0U},
  {MC_SPEED_FILTER_NONE, 3U, "PLL_SPLIT", "fast250_5_slow165_3_e18x40_n32", 2U, 4U, 18, 40, 64U, 1U, 250, 5, 165, 3, 18, 40, 32U, 0U}
};

/* 这一组就是给“有无切换保护”现场演示用的。
   两组除了 startupProtectEnable 以外都一样，这样老师一眼就能明白在比什么。 */
static const MC_ExperimentConfig_t g_mcSwitchCompareConfigs[] =
{
  {MC_SPEED_FILTER_ADAPTIVE_LPF, 3U, "ADALPF", "protect_off", 2U, 4U, 18, 40, 64U, 1U, 245, 5, 165, 3, 17, 38, 32U, 0U},
  {MC_SPEED_FILTER_ADAPTIVE_LPF, 3U, "ADALPF", "protect_on",  2U, 4U, 18, 40, 64U, 1U, 245, 5, 165, 3, 17, 38, 32U, 1U}
};

static uint8_t g_mcStartupProtectEnable = 1U;
static uint32_t g_mcStartupSwitchStableTick = 0U;
static uint16_t g_mcStartupEstimateConfirmCounter = 0U;

static uint8_t g_mcExperimentConfigIndex = 0U;
static uint16_t g_mcExperimentSessionId = 0U;
static MC_ExperimentPhase_t g_mcExperimentPhase = MC_EXPERIMENT_PHASE_IDLE;
static uint8_t g_mcExperimentStopReason = MC_EXPERIMENT_STOP_BY_USER;
static uint32_t g_mcExperimentStartTick = 0U;
static uint32_t g_mcExperimentLastSampleTick = 0U;
static uint32_t g_mcExperimentStopTick = 0U;
static uint32_t g_mcExperimentSampleIndex = 0U;
static uint8_t g_mcExperimentHeaderPending = 0U;
static uint8_t g_mcExperimentFooterPending = 0U;
static uint8_t g_mcExperimentAdvanceConfigPending = 0U;
static uint8_t g_mcExperimentTextModeEnabled = 0U;
static uint8_t g_mcExperimentAutoStopIssued = 0U;
static uint8_t g_mcExperimentBoostIssued = 0U;
static uint8_t g_mcExperimentReturnIssued = 0U;
static uint8_t g_mcExperimentAutoBatchArmed = 0U;
static uint32_t g_mcExperimentNextStartTick = 0U;
static MC_ExperimentSample_t g_mcExperimentQueue[MC_EXPERIMENT_QUEUE_DEPTH];
static volatile uint16_t g_mcExperimentQueueWrite = 0U;
static volatile uint16_t g_mcExperimentQueueRead = 0U;
static volatile uint16_t g_mcExperimentQueueCount = 0U;
static char g_mcExperimentTxBuffer[MC_EXPERIMENT_TX_BUFFER_SIZE];

/* USER CODE END Private Variables */

/* Private functions ---------------------------------------------------------*/
void TSK_MediumFrequencyTaskM1(void);
void FOC_Clear(uint8_t bMotor);
void FOC_InitAdditionalMethods(uint8_t bMotor);
void FOC_CalcCurrRef(uint8_t bMotor);
void TSK_MF_StopProcessing(  MCI_Handle_t * pHandle, uint8_t motor);
MCI_Handle_t * GetMCI(uint8_t bMotor);
static uint16_t FOC_CurrControllerM1(void);
void TSK_SetChargeBootCapDelayM1(uint16_t hTickCount);
bool TSK_ChargeBootCapDelayHasElapsedM1(void);
void TSK_SetStopPermanencyTimeM1(uint16_t hTickCount);
bool TSK_StopPermanencyTimeHasElapsedM1(void);
void TSK_SafetyTask_PWMOFF(uint8_t motor);
static void MC_SPEED_FilterReset(void);
static void MC_SPEED_FilterInitFromSensor(const SpeednPosFdbk_Handle_t *pSource);
static void MC_SPEED_FilterUpdateMecSpeed(const SpeednPosFdbk_Handle_t *pSource, int16_t rawSpeedUnit);
static void MC_SPEED_FilterSyncInstantaneousState(const SpeednPosFdbk_Handle_t *pSource);
static void MC_PLL_TuningReset(void);
static void MC_PLL_TuningApply(const MC_ExperimentConfig_t *pConfig);
static void MC_PLL_TuningUpdate(void);

/* USER CODE BEGIN Private Functions */
static const MC_ExperimentConfig_t *MC_EXPERIMENT_GetConfigTable(void);
static uint8_t MC_EXPERIMENT_GetConfigCount(void);
static const char *MC_EXPERIMENT_GetSeriesName(void);
static const MC_ExperimentConfig_t *MC_EXPERIMENT_GetCurrentConfig(void);
static void MC_EXPERIMENT_ApplyConfig(const MC_ExperimentConfig_t *pConfig);
static void MC_EXPERIMENT_StartSequence(void);
static void MC_EXPERIMENT_RequestStop(uint8_t stopReason);
static void MC_EXPERIMENT_HandlePeriodicSample(void);
static void MC_EXPERIMENT_PushSample(uint8_t phase, uint8_t stopReason);
static void MC_EXPERIMENT_PushMetaFrame(uint8_t phase, uint8_t stopReason);
static uint8_t MC_EXPERIMENT_IsQueueFull(void);
static uint8_t MC_EXPERIMENT_IsQueueEmpty(void);
static void MC_EXPERIMENT_QueueWrite(const MC_ExperimentSample_t *pSample);
static uint8_t MC_EXPERIMENT_QueueRead(MC_ExperimentSample_t *pSample);
static void MC_EXPERIMENT_SendTextIfPossible(void);
static void MC_EXPERIMENT_FormatCsvLine(const MC_ExperimentSample_t *pSample, char *pBuffer, uint16_t bufferSize);
static void MC_EXPERIMENT_UpdateStateByMotorState(void);
static void MC_EXPERIMENT_UpdateSpeedProfile(void);
static uint32_t MC_EXPERIMENT_GetElapsedMs(void);
static const char *MC_EXPERIMENT_PhaseName(uint8_t phase);
static const char *MC_EXPERIMENT_StopReasonName(uint8_t stopReason);
static uint8_t MC_EXPERIMENT_IsStartAllowed(void);
static uint8_t MC_EXPERIMENT_IsStopFinished(MCI_State_t motorState);
static void MC_EXPERIMENT_AbortStartIfNeeded(MCI_State_t motorState);
static uint8_t MC_STARTUP_IsSwitchReady(uint8_t loopClosed);

/* USER CODE END Private Functions */
/**
  * @brief  It initializes the whole MC core according to user defined
  *         parameters.
  * @param  pMCIList pointer to the vector of MCInterface objects that will be
  *         created and initialized. The vector must have length equal to the
  *         number of motor drives.
  * @retval None
  */
__weak void MCboot( MCI_Handle_t* pMCIList[NBR_OF_MOTORS] )
{
  /* USER CODE BEGIN MCboot 0 */

  /* USER CODE END MCboot 0 */

  if (MC_NULL == pMCIList)
  {
    /* Nothing to do */
  }
  else
  {

    bMCBootCompleted = (uint8_t )0;
    pCLM[M1] = &CircleLimitationM1;

    /**********************************************************/
    /*    PWM and current sensing component initialization    */
    /**********************************************************/
    pwmcHandle[M1] = &PWM_Handle_M1._Super;
    R3_2_Init(&PWM_Handle_M1);
    ASPEP_start(&aspepOverUartA);

    /* USER CODE BEGIN MCboot 1 */

    /* USER CODE END MCboot 1 */

    /**************************************/
    /*    Start timers synchronously      */
    /**************************************/
    startTimers();

    /******************************************************/
    /*   PID component initialization: speed regulation   */
    /******************************************************/
    PID_HandleInit(&PIDSpeedHandle_M1);

    /******************************************************/
    /*   Main speed sensor component initialization       */
    /******************************************************/
    STO_PLL_Init (&STO_PLL_M1);
    MC_SPEED_FilterInitFromSensor(&STO_PLL_M1._Super);

    /******************************************************/
    /*   Speed & torque component initialization          */
    /******************************************************/
    STC_Init(pSTC[M1],&PIDSpeedHandle_M1, &SpeedFilterSensorM1);

    /****************************************************/
    /*   Virtual speed sensor component initialization  */
    /****************************************************/
    VSS_Init(&VirtualSpeedSensorM1);

    /**************************************/
    /*   Rev-up component initialization  */
    /**************************************/
    RUC_Init(&RevUpControlM1, pSTC[M1], &VirtualSpeedSensorM1, &STO_M1, pwmcHandle[M1]);

    /********************************************************/
    /*   PID component initialization: current regulation   */
    /********************************************************/
    PID_HandleInit(&PIDIqHandle_M1);
    PID_HandleInit(&PIDIdHandle_M1);

    /********************************************************/
    /*   Bus voltage sensor component initialization        */
    /********************************************************/
    RVBS_Init(&BusVoltageSensor_M1);

    /*************************************************/
    /*   Power measurement component initialization  */
    /*************************************************/
    pMPM[M1]->pVBS = &(BusVoltageSensor_M1._Super);
    pMPM[M1]->pFOCVars = &FOCVars[M1];

    /*******************************************************/
    /*   Temperature measurement component initialization  */
    /*******************************************************/
    NTC_Init(&TempSensor_M1);

    pREMNG[M1] = &RampExtMngrHFParamsM1;
    REMNG_Init(pREMNG[M1]);

    FOC_Clear(M1);
    FOCVars[M1].bDriveInput = EXTERNAL;
    FOCVars[M1].Iqdref = STC_GetDefaultIqdref(pSTC[M1]);
    FOCVars[M1].UserIdref = STC_GetDefaultIqdref(pSTC[M1]).d;
    MCI_Init(&Mci[M1], pSTC[M1], &FOCVars[M1],pwmcHandle[M1] );
    MCI_ExecSpeedRamp(&Mci[M1],
    STC_GetMecSpeedRefUnitDefault(pSTC[M1]),0); /*First command to STC*/

    pMCIList[M1] = &Mci[M1];

    DAC_Init(&DAC_Handle);

    /* USER CODE BEGIN MCboot 2 */

    /* USER CODE END MCboot 2 */

    bMCBootCompleted = 1U;
  }
}

/**
 * @brief Runs all the Tasks of the Motor Control cockpit
 *
 * This function is to be called periodically at least at the Medium Frequency task
 * rate (It is typically called on the Systick interrupt). Exact invokation rate is
 * the Speed regulator execution rate set in the Motor Contorl Workbench.
 *
 * The following tasks are executed in this order:
 *
 * - Medium Frequency Tasks of each motors
 * - Safety Task
 * - Power Factor Correction Task (if enabled)
 * - User Interface task.
 */
__weak void MC_RunMotorControlTasks(void)
{
  if (0U == bMCBootCompleted)
  {
    /* Nothing to do */
  }
  else
  {
    /* ** Medium Frequency Tasks ** */
    MC_Scheduler();

    /* Safety task is run after Medium Frequency task so that
     * it can overcome actions they initiated if needed. */
    TSK_SafetyTask();

  }
}

/**
 * @brief Performs stop process and update the state machine.This function
 *        shall be called only during medium frequency task
 */
void TSK_MF_StopProcessing(  MCI_Handle_t * pHandle, uint8_t motor)
{
  R3_2_SwitchOffPWM(pwmcHandle[motor]);

  FOC_Clear(motor);
  MPM_Clear((MotorPowMeas_Handle_t*) pMPM[motor]);
  TSK_SetStopPermanencyTimeM1(STOPPERMANENCY_TICKS);
  Mci[motor].State = STOP;
  return;
}

/**
 * @brief  Executes the Medium Frequency Task functions for each drive instance.
 *
 * It is to be clocked at the Systick frequency.
 */
__weak void MC_Scheduler(void)
{
/* USER CODE BEGIN MC_Scheduler 0 */
  MC_EXPERIMENT_UpdateStateByMotorState();
  MC_EXPERIMENT_HandlePeriodicSample();

/* USER CODE END MC_Scheduler 0 */

  if (((uint8_t)1) == bMCBootCompleted)
  {
    if(hMFTaskCounterM1 > 0u)
    {
      hMFTaskCounterM1--;
    }
    else
    {
      TSK_MediumFrequencyTaskM1();

      if (g_mcExperimentTextModeEnabled != 0U)
      {
        /* 实验模式下串口改成纯文本输出，这里不处理 Motor Pilot 协议 */
      }
      else
      {
        MCP_Over_UartA.rxBuffer = MCP_Over_UartA.pTransportLayer->fRXPacketProcess(MCP_Over_UartA.pTransportLayer,
                                                                                  &MCP_Over_UartA.rxLength);
        if ( 0U == MCP_Over_UartA.rxBuffer)
        {
          /* Nothing to do */
        }
        else
        {
          /* Synchronous answer */
          if (0U == MCP_Over_UartA.pTransportLayer->fGetBuffer(MCP_Over_UartA.pTransportLayer,
                                                       (void **) &MCP_Over_UartA.txBuffer, //cstat !MISRAC2012-Rule-11.3
                                                       MCTL_SYNC))
          {
            /* no buffer available to build the answer ... should not occur */
          }
          else
          {
            MCP_ReceivedPacket(&MCP_Over_UartA);
            MCP_Over_UartA.pTransportLayer->fSendPacket(MCP_Over_UartA.pTransportLayer, MCP_Over_UartA.txBuffer,
                                                        MCP_Over_UartA.txLength, MCTL_SYNC);
            /* no buffer available to build the answer ... should not occur */
          }
        }
      }

      /* USER CODE BEGIN MC_Scheduler 1 */

      /* USER CODE END MC_Scheduler 1 */
      hMFTaskCounterM1 = (uint16_t)MF_TASK_OCCURENCE_TICKS;
    }
    if(hBootCapDelayCounterM1 > 0U)
    {
      hBootCapDelayCounterM1--;
    }
    if(hStopPermanencyCounterM1 > 0U)
    {
      hStopPermanencyCounterM1--;
    }
  }
  else
  {
    /* Nothing to do */
  }
  /* USER CODE BEGIN MC_Scheduler 2 */

  /* USER CODE END MC_Scheduler 2 */
}

/**
  * @brief Executes medium frequency periodic Motor Control tasks
  *
  * This function performs some of the control duties on Motor 1 according to the
  * present state of its state machine. In particular, duties requiring a periodic
  * execution at a medium frequency rate (such as the speed controller for instance)
  * are executed here.
  */
__weak void TSK_MediumFrequencyTaskM1(void)
{
  /* USER CODE BEGIN MediumFrequencyTask M1 0 */

  /* USER CODE END MediumFrequencyTask M1 0 */

  int16_t wAux = 0;

  bool IsSpeedReliable = STO_PLL_CalcAvrgMecSpeedUnit(&STO_PLL_M1, &wAux);
  MC_SPEED_FilterUpdateMecSpeed(&STO_PLL_M1._Super, wAux);
  MC_PLL_TuningUpdate();
  PQD_CalcElMotorPower(pMPM[M1]);

  if (MCI_GetCurrentFaults(&Mci[M1]) == MC_NO_FAULTS)
  {
    if (MCI_GetOccurredFaults(&Mci[M1]) == MC_NO_FAULTS)
    {
      switch (Mci[M1].State)
      {
        case IDLE:
        {
          if ((MCI_START == Mci[M1].DirectCommand) || (MCI_MEASURE_OFFSETS == Mci[M1].DirectCommand))
          {
            RUC_Clear(&RevUpControlM1, MCI_GetImposedMotorDirection(&Mci[M1]));

           if (pwmcHandle[M1]->offsetCalibStatus == false)
           {
             PWMC_CurrentReadingCalibr(pwmcHandle[M1], CRC_START);
             Mci[M1].State = OFFSET_CALIB;
           }
           else
           {
             /* calibration already done. Enables only TIM channels */
             pwmcHandle[M1]->OffCalibrWaitTimeCounter = 1u;
             PWMC_CurrentReadingCalibr(pwmcHandle[M1], CRC_EXEC);
             R3_2_TurnOnLowSides(pwmcHandle[M1]);
             TSK_SetChargeBootCapDelayM1(CHARGE_BOOT_CAP_TICKS);
             Mci[M1].State = CHARGE_BOOT_CAP;

           }

          }
          else
          {
            /* nothing to be done, FW stays in IDLE state */
          }
          break;
        }

        case OFFSET_CALIB:
          {
            if (MCI_STOP == Mci[M1].DirectCommand)
            {
              TSK_MF_StopProcessing(&Mci[M1], M1);
            }
            else
            {
              if (PWMC_CurrentReadingCalibr(pwmcHandle[M1], CRC_EXEC))
              {
                if (MCI_MEASURE_OFFSETS == Mci[M1].DirectCommand)
                {
                  FOC_Clear(M1);
                  MPM_Clear((MotorPowMeas_Handle_t*) pMPM[M1]);
                  Mci[M1].DirectCommand = MCI_NO_COMMAND;
                  Mci[M1].State = IDLE;
                }
                else
                {
                  R3_2_TurnOnLowSides(pwmcHandle[M1]);
                  TSK_SetChargeBootCapDelayM1(CHARGE_BOOT_CAP_TICKS);
                  Mci[M1].State = CHARGE_BOOT_CAP;

                }
              }
              else
              {
                /* nothing to be done, FW waits for offset calibration to finish */
              }
            }
            break;
          }

        case CHARGE_BOOT_CAP:
        {
          if (MCI_STOP == Mci[M1].DirectCommand)
          {
            TSK_MF_StopProcessing(&Mci[M1], M1);
          }
          else
          {
            if (TSK_ChargeBootCapDelayHasElapsedM1())
            {
              R3_2_SwitchOffPWM(pwmcHandle[M1]);
             FOCVars[M1].bDriveInput = EXTERNAL;
             STC_SetSpeedSensor( pSTC[M1], &VirtualSpeedSensorM1._Super );
              STO_PLL_Clear(&STO_PLL_M1);
              FOC_Clear( M1 );

              Mci[M1].State = START;

              PWMC_SwitchOnPWM(pwmcHandle[M1]);
            }
            else
            {
              /* nothing to be done, FW waits for bootstrap capacitor to charge */
            }
          }
          break;
        }

        case START:
        {
          if (MCI_STOP == Mci[M1].DirectCommand)
          {
            TSK_MF_StopProcessing(&Mci[M1], M1);
          }
          else
          {
            /* Mechanical speed as imposed by the Virtual Speed Sensor during the Rev Up phase. */
            int16_t hForcedMecSpeedUnit;
            qd_t IqdRef;
            bool ObserverConverged = false;

            /* Execute the Rev Up procedure */
            if(! RUC_Exec(&RevUpControlM1))

            {
            /* The time allowed for the startup sequence has expired */
              MCI_FaultProcessing(&Mci[M1], MC_START_UP, 0);

           }
           else
           {
             /* Execute the torque open loop current start-up ramp:
              * Compute the Iq reference current as configured in the Rev Up sequence */
             IqdRef.q = STC_CalcTorqueReference( pSTC[M1] );
             IqdRef.d = FOCVars[M1].UserIdref;
             /* Iqd reference current used by the High Frequency Loop to generate the PWM output */
             FOCVars[M1].Iqdref = IqdRef;
           }

           (void) VSS_CalcAvrgMecSpeedUnit(&VirtualSpeedSensorM1, &hForcedMecSpeedUnit);

           /* check that startup stage where the observer has to be used has been reached */
           if (true == RUC_FirstAccelerationStageReached(&RevUpControlM1))

            {
             ObserverConverged = STO_PLL_IsObserverConverged(&STO_PLL_M1, &hForcedMecSpeedUnit);
             STO_SetDirection(&STO_PLL_M1, (int8_t)MCI_GetImposedMotorDirection(&Mci[M1]));

              (void)VSS_SetStartTransition(&VirtualSpeedSensorM1, ObserverConverged);
            }

            if (ObserverConverged)
            {
              qd_t StatorCurrent = MCM_Park(FOCVars[M1].Ialphabeta, SPD_GetElAngle(&STO_PLL_M1._Super));

              /* Start switch over ramp. This ramp will transition from the revup to the closed loop FOC. */
              REMNG_Init(pREMNG[M1]);
              (void)REMNG_ExecRamp(pREMNG[M1], FOCVars[M1].Iqdref.q, 0);
              (void)REMNG_ExecRamp(pREMNG[M1], StatorCurrent.q, TRANSITION_DURATION);

              Mci[M1].State = SWITCH_OVER;
            }
          }
          break;
        }

        case SWITCH_OVER:
        {
          if (MCI_STOP == Mci[M1].DirectCommand)
          {
            TSK_MF_StopProcessing(&Mci[M1], M1);
          }
          else
          {
              bool LoopClosed;
              uint8_t SwitchReady;
              int16_t hForcedMecSpeedUnit;

            if(! RUC_Exec(&RevUpControlM1))

            {
              /* The time allowed for the startup sequence has expired */
              MCI_FaultProcessing(&Mci[M1], MC_START_UP, 0);

            }
            else

            {
              /* Compute the virtual speed and positions of the rotor.
                 The function returns true if the virtual speed is in the reliability range */
              LoopClosed = VSS_CalcAvrgMecSpeedUnit(&VirtualSpeedSensorM1, &hForcedMecSpeedUnit);
              /* Check if the transition ramp has completed. */
              bool tempBool;
              tempBool = VSS_TransitionEnded(&VirtualSpeedSensorM1);
              LoopClosed = LoopClosed || tempBool;

              /* 原版这里基本上是“看起来能闭环了就切过去”。
                 我们这里多包一层判断：
                 1. 如果演示的是无保护基线，就直接放行；
                 2. 如果开了保护，就要求速度先过最小门限，并且连续稳定一小段时间。 */
              SwitchReady = MC_STARTUP_IsSwitchReady((uint8_t)((true == LoopClosed) ? 1U : 0U));

              if (SwitchReady != 0U)
              {
                #if ( PID_SPEED_INTEGRAL_INIT_DIV == 0 )
                PID_SetIntegralTerm(&PIDSpeedHandle_M1, 0);
                #else
                PID_SetIntegralTerm(&PIDSpeedHandle_M1,
                                    (((int32_t)FOCVars[M1].Iqdref.q * (int16_t)PID_GetKIDivisor(&PIDSpeedHandle_M1))
                                    / PID_SPEED_INTEGRAL_INIT_DIV));
                #endif

              /* USER CODE BEGIN MediumFrequencyTask M1 1 */

              /* USER CODE END MediumFrequencyTask M1 1 */
                /* 这里就是我们这次课设真正接管速度环的位置。
                   前面做的滤波、PLL 调整、启动保护，最后都会体现在这个速度反馈源上。 */
                STC_SetSpeedSensor(pSTC[M1], &SpeedFilterSensorM1); /* Filtered observer speed */
                FOC_InitAdditionalMethods(M1);
                FOC_CalcCurrRef( M1 );
                STC_ForceSpeedReferenceToCurrentSpeed(pSTC[M1]); /* Init the reference speed to current speed */
                MCI_ExecBufferedCommands(&Mci[M1]); /* Exec the speed ramp after changing of the speed sensor */
                Mci[M1].State = RUN;
              }
            }
          }
          break;
        }

        case RUN:
        {
          if (MCI_STOP == Mci[M1].DirectCommand)
          {
            TSK_MF_StopProcessing(&Mci[M1], M1);
          }
          else
          {
            /* USER CODE BEGIN MediumFrequencyTask M1 2 */

            /* USER CODE END MediumFrequencyTask M1 2 */

            MCI_ExecBufferedCommands(&Mci[M1]);
            FOC_CalcCurrRef(M1);

            if(!IsSpeedReliable)
            {
              MCI_FaultProcessing(&Mci[M1], MC_SPEED_FDBK, 0);
            }

          }
          break;
        }

        case STOP:
        {
          if (TSK_StopPermanencyTimeHasElapsedM1())
          {

            STC_SetSpeedSensor(pSTC[M1], &VirtualSpeedSensorM1._Super);  	/*  sensor-less */
            VSS_Clear(&VirtualSpeedSensorM1); /* Reset measured speed in IDLE */
            MC_SPEED_FilterReset();

            /* USER CODE BEGIN MediumFrequencyTask M1 5 */

            /* USER CODE END MediumFrequencyTask M1 5 */
            Mci[M1].DirectCommand = MCI_NO_COMMAND;
            Mci[M1].State = IDLE;

          }
          else
          {
            /* nothing to do, FW waits for to stop */
          }
          break;
        }

        case FAULT_OVER:
        {
          if (MCI_ACK_FAULTS == Mci[M1].DirectCommand)
          {
            Mci[M1].DirectCommand = MCI_NO_COMMAND;
            Mci[M1].State = IDLE;

          }
          else
          {
            /* nothing to do, FW stays in FAULT_OVER state until acknowledgement */
          }
        }
        break;

        case FAULT_NOW:
        {
          Mci[M1].State = FAULT_OVER;
        }

        default:
          break;
       }
    }
    else
    {
      Mci[M1].State = FAULT_OVER;
    }
  }
  else
  {
    Mci[M1].State = FAULT_NOW;
  }

  /* USER CODE BEGIN MediumFrequencyTask M1 6 */

  /* USER CODE END MediumFrequencyTask M1 6 */
}

/**
  * @brief  It re-initializes the current and voltage variables. Moreover
  *         it clears qd currents PI controllers, voltage sensor and SpeednTorque
  *         controller. It must be called before each motor restart.
  *         It does not clear speed sensor.
  * @param  bMotor related motor it can be M1 or M2
  * @retval none
  */
__weak void FOC_Clear(uint8_t bMotor)
{
  /* USER CODE BEGIN FOC_Clear 0 */

  /* USER CODE END FOC_Clear 0 */
  ab_t NULL_ab = {((int16_t)0), ((int16_t)0)};
  qd_t NULL_qd = {((int16_t)0), ((int16_t)0)};
  alphabeta_t NULL_alphabeta = {((int16_t)0), ((int16_t)0)};

  FOCVars[bMotor].Iab = NULL_ab;
  FOCVars[bMotor].Ialphabeta = NULL_alphabeta;
  FOCVars[bMotor].Iqd = NULL_qd;
  FOCVars[bMotor].Iqdref = NULL_qd;
  FOCVars[bMotor].hTeref = (int16_t)0;
  FOCVars[bMotor].Vqd = NULL_qd;
  FOCVars[bMotor].Valphabeta = NULL_alphabeta;
  FOCVars[bMotor].hElAngle = (int16_t)0;
  if (M1 == bMotor)
  {
    MC_SPEED_FilterReset();
  }

  PID_SetIntegralTerm(pPIDIq[bMotor], ((int32_t)0));
  PID_SetIntegralTerm(pPIDId[bMotor], ((int32_t)0));

  STC_Clear(pSTC[bMotor]);

  PWMC_SwitchOffPWM(pwmcHandle[bMotor]);

  /* USER CODE BEGIN FOC_Clear 1 */

  /* USER CODE END FOC_Clear 1 */
}

/**
  * @brief  Use this method to initialize additional methods (if any) in
  *         START_TO_RUN state
  * @param  bMotor related motor it can be M1 or M2
  * @retval none
  */
__weak void FOC_InitAdditionalMethods(uint8_t bMotor) //cstat !RED-func-no-effect
{
    if (M_NONE == bMotor)
    {
      /* Nothing to do */
    }
    else
    {
  /* USER CODE BEGIN FOC_InitAdditionalMethods 0 */

  /* USER CODE END FOC_InitAdditionalMethods 0 */
    }
}

/**
  * @brief  It computes the new values of Iqdref (current references on qd
  *         reference frame) based on the required electrical torque information
  *         provided by oTSC object (internally clocked).
  *         If implemented in the derived class it executes flux weakening and/or
  *         MTPA algorithm(s). It must be called with the periodicity specified
  *         in oTSC parameters
  * @param  bMotor related motor it can be M1 or M2
  * @retval none
  */
__weak void FOC_CalcCurrRef(uint8_t bMotor)
{

  /* USER CODE BEGIN FOC_CalcCurrRef 0 */

  /* USER CODE END FOC_CalcCurrRef 0 */
  if (INTERNAL == FOCVars[bMotor].bDriveInput)
  {
    FOCVars[bMotor].hTeref = STC_CalcTorqueReference(pSTC[bMotor]);
    FOCVars[bMotor].Iqdref.q = FOCVars[bMotor].hTeref;

  }
  else
  {
    /* Nothing to do */
  }
  /* USER CODE BEGIN FOC_CalcCurrRef 1 */

  /* USER CODE END FOC_CalcCurrRef 1 */
}

/**
  * @brief  It set a counter intended to be used for counting the delay required
  *         for drivers boot capacitors charging of motor 1
  * @param  hTickCount number of ticks to be counted
  * @retval void
  */
__weak void TSK_SetChargeBootCapDelayM1(uint16_t hTickCount)
{
   hBootCapDelayCounterM1 = hTickCount;
}

/**
  * @brief  Use this function to know whether the time required to charge boot
  *         capacitors of motor 1 has elapsed
  * @param  none
  * @retval bool true if time has elapsed, false otherwise
  */
__weak bool TSK_ChargeBootCapDelayHasElapsedM1(void)
{
  bool retVal = false;
  if (((uint16_t)0) == hBootCapDelayCounterM1)
  {
    retVal = true;
  }
  return (retVal);
}

/**
  * @brief  It set a counter intended to be used for counting the permanency
  *         time in STOP state of motor 1
  * @param  hTickCount number of ticks to be counted
  * @retval void
  */
__weak void TSK_SetStopPermanencyTimeM1(uint16_t hTickCount)
{
  hStopPermanencyCounterM1 = hTickCount;
}

/**
  * @brief  Use this function to know whether the permanency time in STOP state
  *         of motor 1 has elapsed
  * @param  none
  * @retval bool true if time is elapsed, false otherwise
  */
__weak bool TSK_StopPermanencyTimeHasElapsedM1(void)
{
  bool retVal = false;
  if (((uint16_t)0) == hStopPermanencyCounterM1)
  {
    retVal = true;
  }
  return (retVal);
}

#if defined (CCMRAM)
#if defined (__ICCARM__)
#pragma location = ".ccmram"
#elif defined (__CC_ARM) || defined(__GNUC__)
__attribute__((section (".ccmram")))
#endif
#endif

/**
  * @brief  Executes the Motor Control duties that require a high frequency rate and a precise timing
  *
  *  This is mainly the FOC current control loop. It is executed depending on the state of the Motor Control
  * subsystem (see the state machine(s)).
  *
  * @retval Number of the  motor instance which FOC loop was executed.
  */
__weak uint8_t TSK_HighFrequencyTask(void)
{
  /* USER CODE BEGIN HighFrequencyTask 0 */

  /* USER CODE END HighFrequencyTask 0 */

  uint16_t hFOCreturn;
  uint8_t bMotorNbr = 0;

  Observer_Inputs_t STO_Inputs; /*  only if sensorless main*/

  STO_Inputs.Valfa_beta = FOCVars[M1].Valphabeta;  /* only if sensorless*/
  if (SWITCH_OVER == Mci[M1].State)
  {
    if (!REMNG_RampCompleted(pREMNG[M1]))
    {
      FOCVars[M1].Iqdref.q = (int16_t)REMNG_Calc(pREMNG[M1]);
    }
  }
  /* USER CODE BEGIN HighFrequencyTask SINGLEDRIVE_1 */

  /* USER CODE END HighFrequencyTask SINGLEDRIVE_1 */
  hFOCreturn = FOC_CurrControllerM1();
  /* USER CODE BEGIN HighFrequencyTask SINGLEDRIVE_2 */

  /* USER CODE END HighFrequencyTask SINGLEDRIVE_2 */
  if(hFOCreturn == MC_FOC_DURATION)
  {
    MCI_FaultProcessing(&Mci[M1], MC_FOC_DURATION, 0);
  }
  else
  {
    bool IsAccelerationStageReached = RUC_FirstAccelerationStageReached(&RevUpControlM1);
    STO_Inputs.Ialfa_beta = FOCVars[M1].Ialphabeta; /*  only if sensorless*/
    STO_Inputs.Vbus = VBS_GetAvBusVoltage_d(&(BusVoltageSensor_M1._Super)); /*  only for sensorless*/
    (void)( void )STO_PLL_CalcElAngle(&STO_PLL_M1, &STO_Inputs);
    STO_PLL_CalcAvrgElSpeedDpp(&STO_PLL_M1); /*  Only in case of Sensor-less */
    MC_SPEED_FilterSyncInstantaneousState(&STO_PLL_M1._Super);
	 if (false == IsAccelerationStageReached)
    {
      STO_ResetPLL(&STO_PLL_M1);
      MC_SPEED_FilterReset();
    }
    /*  only for sensor-less*/
    if(((uint16_t)START == Mci[M1].State) || ((uint16_t)SWITCH_OVER == Mci[M1].State))
    {
      int16_t hObsAngle = SPD_GetElAngle(&STO_PLL_M1._Super);
      (void)VSS_CalcElAngle(&VirtualSpeedSensorM1, &hObsAngle);
    }
    /* USER CODE BEGIN HighFrequencyTask SINGLEDRIVE_3 */

    /* USER CODE END HighFrequencyTask SINGLEDRIVE_3 */
  }
  DAC_Exec(&DAC_Handle);
  /* USER CODE BEGIN HighFrequencyTask 1 */

  /* USER CODE END HighFrequencyTask 1 */

  GLOBAL_TIMESTAMP++;
  if (0U == MCPA_UART_A.Mark)
  {
    /* Nothing to do */
  }
  else
  {
    MCPA_dataLog (&MCPA_UART_A);
  }

  return (bMotorNbr);
}

#if defined (CCMRAM)
#if defined (__ICCARM__)
#pragma location = ".ccmram"
#elif defined (__CC_ARM) || defined(__GNUC__)
__attribute__((section (".ccmram")))
#endif
#endif
/**
  * @brief It executes the core of FOC drive that is the controllers for Iqd
  *        currents regulation. Reference frame transformations are carried out
  *        accordingly to the active speed sensor. It must be called periodically
  *        when new motor currents have been converted
  * @param this related object of class CFOC.
  * @retval int16_t It returns MC_NO_FAULTS if the FOC has been ended before
  *         next PWM Update event, MC_FOC_DURATION otherwise
  */
inline uint16_t FOC_CurrControllerM1(void)
{
  qd_t Iqd, Vqd;
  ab_t Iab;
  alphabeta_t Ialphabeta, Valphabeta;

  int16_t hElAngle;
  uint16_t hCodeError;
  SpeednPosFdbk_Handle_t *speedHandle;

  speedHandle = STC_GetSpeedSensor(pSTC[M1]);
  hElAngle = SPD_GetElAngle(speedHandle);
  hElAngle += SPD_GetInstElSpeedDpp(speedHandle)*PARK_ANGLE_COMPENSATION_FACTOR;
  PWMC_GetPhaseCurrents(pwmcHandle[M1], &Iab);
  RCM_ReadOngoingConv();
  RCM_ExecNextConv();
  Ialphabeta = MCM_Clarke(Iab);
  Iqd = MCM_Park(Ialphabeta, hElAngle);
  Vqd.q = PI_Controller(pPIDIq[M1], (int32_t)(FOCVars[M1].Iqdref.q) - Iqd.q);
  Vqd.d = PI_Controller(pPIDId[M1], (int32_t)(FOCVars[M1].Iqdref.d) - Iqd.d);

  Vqd = Circle_Limitation(pCLM[M1], Vqd);
  hElAngle += SPD_GetInstElSpeedDpp(speedHandle)*REV_PARK_ANGLE_COMPENSATION_FACTOR;
  Valphabeta = MCM_Rev_Park(Vqd, hElAngle);
  hCodeError = PWMC_SetPhaseVoltage(pwmcHandle[M1], Valphabeta);

  FOCVars[M1].Vqd = Vqd;
  FOCVars[M1].Iab = Iab;
  FOCVars[M1].Ialphabeta = Ialphabeta;
  FOCVars[M1].Iqd = Iqd;
  FOCVars[M1].Valphabeta = Valphabeta;
  FOCVars[M1].hElAngle = hElAngle;

  return(hCodeError);
}

/**
  * @brief  Executes safety checks (e.g. bus voltage and temperature) for all drive instances.
  *
  * Faults flags are updated here.
  */
__weak void TSK_SafetyTask(void)
{
  /* USER CODE BEGIN TSK_SafetyTask 0 */

  /* USER CODE END TSK_SafetyTask 0 */
  if (1U == bMCBootCompleted)
  {
    TSK_SafetyTask_PWMOFF(M1);
    /* User conversion execution */
    RCM_ExecUserConv();
  /* USER CODE BEGIN TSK_SafetyTask 1 */

  /* USER CODE END TSK_SafetyTask 1 */
  }
}

/**
  * @brief  Safety task implementation if  MC.ON_OVER_VOLTAGE == TURN_OFF_PWM
  * @param  bMotor Motor reference number defined
  *         \link Motors_reference_number here \endlink
  * @retval None
  */
__weak void TSK_SafetyTask_PWMOFF(uint8_t bMotor)
{
  /* USER CODE BEGIN TSK_SafetyTask_PWMOFF 0 */

  /* USER CODE END TSK_SafetyTask_PWMOFF 0 */
  uint16_t CodeReturn = MC_NO_ERROR;
  uint16_t errMask[NBR_OF_MOTORS] = {VBUS_TEMP_ERR_MASK};

  CodeReturn |= errMask[bMotor] & NTC_CalcAvTemp(pTemperatureSensor[bMotor]); /* check for fault if FW protection is activated. It returns MC_OVER_TEMP or MC_NO_ERROR */
  CodeReturn |= PWMC_CheckOverCurrent(pwmcHandle[bMotor]);                    /* check for fault. It return MC_BREAK_IN or MC_NO_FAULTS
                                                                                 (for STM32F30x can return MC_OVER_VOLT in case of HW Overvoltage) */
  if(M1 == bMotor)
  {
    CodeReturn |= errMask[bMotor] & RVBS_CalcAvVbus(&BusVoltageSensor_M1);
  }
  MCI_FaultProcessing(&Mci[bMotor], CodeReturn, ~CodeReturn); /* process faults */
  if (MCI_GetFaultState(&Mci[bMotor]) != (uint32_t)MC_NO_FAULTS)
  {
    PWMC_SwitchOffPWM(pwmcHandle[bMotor]);
    if (MCPA_UART_A.Mark != 0)
    {
      MCPA_flushDataLog (&MCPA_UART_A);
    }
    FOC_Clear(bMotor);
    MPM_Clear((MotorPowMeas_Handle_t*)pMPM[bMotor]); //cstat !MISRAC2012-Rule-11.3
    /* USER CODE BEGIN TSK_SafetyTask_PWMOFF 1 */

    /* USER CODE END TSK_SafetyTask_PWMOFF 1 */
  }
  else
  {
    /* no errors */
  }

  /* USER CODE BEGIN TSK_SafetyTask_PWMOFF 3 */

  /* USER CODE END TSK_SafetyTask_PWMOFF 3 */
}

/**
  * @brief  This function returns the reference of the MCInterface relative to
  *         the selected drive.
  * @param  bMotor Motor reference number defined
  *         \link Motors_reference_number here \endlink
  * @retval MCI_Handle_t * Reference to MCInterface relative to the selected drive.
  *         Note: it can be MC_NULL if MCInterface of selected drive is not
  *         allocated.
  */
__weak MCI_Handle_t * GetMCI(uint8_t bMotor)
{
  MCI_Handle_t * retVal = MC_NULL;
  if (bMotor < (uint8_t)NBR_OF_MOTORS)
  {
    retVal = &Mci[bMotor];
  }
  return (retVal);
}

/**
  * @brief  Puts the Motor Control subsystem in in safety conditions on a Hard Fault
  *
  *  This function is to be executed when a general hardware failure has been detected
  * by the microcontroller and is used to put the system in safety condition.
  */
__weak void TSK_HardwareFaultTask(void)
{
  /* USER CODE BEGIN TSK_HardwareFaultTask 0 */

  /* USER CODE END TSK_HardwareFaultTask 0 */
  R3_2_SwitchOffPWM(pwmcHandle[M1]);
  MCI_FaultProcessing(&Mci[M1], MC_SW_ERROR, 0);
  /* USER CODE BEGIN TSK_HardwareFaultTask 1 */

  /* USER CODE END TSK_HardwareFaultTask 1 */
}

__weak void UI_HandleStartStopButton_cb (void)
{
/* USER CODE BEGIN START_STOP_BTN */
  if (MC_EXPERIMENT_IsStartAllowed() != 0U)
  {
    MC_EXPERIMENT_StartSequence();
  }
  else
  {
    /* 运行中或者收尾中先不响应，避免把自动实验流程打乱 */
  }
/* USER CODE END START_STOP_BTN */
}

 /**
  * @brief  Locks GPIO pins used for Motor Control to prevent accidental reconfiguration
  */
__weak void mc_lock_pins (void)
{
LL_GPIO_LockPin(M1_BUS_VOLTAGE_GPIO_Port, M1_BUS_VOLTAGE_Pin);
LL_GPIO_LockPin(M1_TEMPERATURE_GPIO_Port, M1_TEMPERATURE_Pin);
LL_GPIO_LockPin(M1_CURR_AMPL_W_GPIO_Port, M1_CURR_AMPL_W_Pin);
LL_GPIO_LockPin(M1_PWM_UH_GPIO_Port, M1_PWM_UH_Pin);
LL_GPIO_LockPin(M1_PWM_VH_GPIO_Port, M1_PWM_VH_Pin);
LL_GPIO_LockPin(M1_OCP_GPIO_Port, M1_OCP_Pin);
LL_GPIO_LockPin(M1_PWM_WH_GPIO_Port, M1_PWM_WH_Pin);
LL_GPIO_LockPin(M1_PWM_EN_V_GPIO_Port, M1_PWM_EN_V_Pin);
LL_GPIO_LockPin(M1_PWM_EN_U_GPIO_Port, M1_PWM_EN_U_Pin);
LL_GPIO_LockPin(M1_PWM_EN_W_GPIO_Port, M1_PWM_EN_W_Pin);
LL_GPIO_LockPin(M1_CURR_AMPL_U_GPIO_Port, M1_CURR_AMPL_U_Pin);
LL_GPIO_LockPin(M1_CURR_AMPL_V_GPIO_Port, M1_CURR_AMPL_V_Pin);
}

static void MC_SPEED_FilterReset(void)
{
  uint8_t i;

  g_mcRawMecSpeedUnit = 0;
  g_mcFilteredMecSpeedUnit = 0;
  g_mcMovingAvgIndex = 0U;
  g_mcMovingAvgSum = 0;
  for (i = 0U; i < MC_SPEED_FILTER_MOVAVG_DEPTH; i++)
  {
    g_mcMovingAvgBuffer[i] = 0;
  }

  g_mcWeightedMovingAvgIndex = 0U;
  for (i = 0U; i < MC_SPEED_FILTER_WMA_DEPTH; i++)
  {
    g_mcWeightedMovingAvgBuffer[i] = 0;
  }

  /* 自适应低通每次开新实验都先回到“响应快”的状态 */
  g_mcAdaptiveCurrentShift = g_mcAdaptiveFastShift;
  g_mcAdaptiveSteadyCounter = 0U;

  SpeedFilterSensorM1.hAvrMecSpeedUnit = 0;
}

static void MC_SPEED_FilterInitFromSensor(const SpeednPosFdbk_Handle_t *pSource)
{
  SpeedFilterSensorM1 = *pSource;
  MC_SPEED_FilterReset();
}

static void MC_SPEED_FilterUpdateMecSpeed(const SpeednPosFdbk_Handle_t *pSource, int16_t rawSpeedUnit)
{
  int32_t filtered = rawSpeedUnit;
  int16_t targetSpeedUnit = (int16_t)MC_GetMecSpeedReferenceMotor1();

  g_mcRawMecSpeedUnit = rawSpeedUnit;

  switch (g_mcSpeedFilterMode)
  {
    case MC_SPEED_FILTER_NONE:
      filtered = rawSpeedUnit;
      break;

    case MC_SPEED_FILTER_MOVING_AVG:
      /* 普通滑动平均：窗口更长，抖动更小，但动态会更慢 */
      g_mcMovingAvgSum -= g_mcMovingAvgBuffer[g_mcMovingAvgIndex];
      g_mcMovingAvgBuffer[g_mcMovingAvgIndex] = rawSpeedUnit;
      g_mcMovingAvgSum += rawSpeedUnit;
      g_mcMovingAvgIndex++;
      if (g_mcMovingAvgIndex >= MC_SPEED_FILTER_MOVAVG_DEPTH)
      {
        g_mcMovingAvgIndex = 0U;
      }
      filtered = g_mcMovingAvgSum / (int32_t)MC_SPEED_FILTER_MOVAVG_DEPTH;
      break;

    case MC_SPEED_FILTER_WEIGHTED_MOVING_AVG:
    {
      uint8_t sampleCount;
      uint8_t offset;
      uint8_t idx;
      int32_t weightedSum = 0;
      int32_t weightTotal = 0;

      /* 加权滑动平均：越新的点权重越大，尽量兼顾平滑和响应 */
      g_mcWeightedMovingAvgBuffer[g_mcWeightedMovingAvgIndex] = rawSpeedUnit;
      g_mcWeightedMovingAvgIndex++;
      if (g_mcWeightedMovingAvgIndex >= MC_SPEED_FILTER_WMA_DEPTH)
      {
        g_mcWeightedMovingAvgIndex = 0U;
      }

      for (sampleCount = 0U; sampleCount < MC_SPEED_FILTER_WMA_DEPTH; sampleCount++)
      {
        offset = (uint8_t)(MC_SPEED_FILTER_WMA_DEPTH - 1U - sampleCount);
        idx = (uint8_t)((g_mcWeightedMovingAvgIndex + offset) % MC_SPEED_FILTER_WMA_DEPTH);
        weightedSum += (int32_t)g_mcWeightedMovingAvgBuffer[idx] * (int32_t)(sampleCount + 1U);
        weightTotal += (int32_t)(sampleCount + 1U);
      }

      if (weightTotal > 0)
      {
        filtered = weightedSum / weightTotal;
      }
      break;
    }

    case MC_SPEED_FILTER_ADAPTIVE_LPF:
    {
      int16_t speedErrorRpm = SPEED_UNIT_2_RPM((int16_t)(targetSpeedUnit - rawSpeedUnit));
      int16_t absSpeedErrorRpm = (speedErrorRpm >= 0) ? speedErrorRpm : (int16_t)(-speedErrorRpm);
      uint8_t activeShift = g_mcAdaptiveCurrentShift;

      if (absSpeedErrorRpm >= g_mcAdaptiveExitRpm)
      {
        /* 扰动一旦变大，立刻回到响应更快的弱滤波 */
        g_mcAdaptiveSteadyCounter = 0U;
        activeShift = g_mcAdaptiveFastShift;
      }
      else if (absSpeedErrorRpm <= g_mcAdaptiveEnterRpm)
      {
        /* 进入稳态不能只看一个点，连续满足一段时间再切强滤波 */
        if (g_mcAdaptiveSteadyCounter < g_mcAdaptiveConfirmN)
        {
          g_mcAdaptiveSteadyCounter++;
        }
        if (g_mcAdaptiveSteadyCounter >= g_mcAdaptiveConfirmN)
        {
          activeShift = g_mcAdaptiveSlowShift;
        }
      }
      else
      {
        /* 中间区间保持当前模式，避免来回抖动切换 */
      }

      /* 启动和大扰动阶段少滤一点，稳速以后再加大滤波力度 */
      g_mcAdaptiveCurrentShift = activeShift;
      filtered = (int32_t)g_mcFilteredMecSpeedUnit;
      filtered += ((int32_t)rawSpeedUnit - filtered) >> activeShift;
      break;
    }

    case MC_SPEED_FILTER_LPF1:
    default:
      /* 一阶低通是最基础的版本，shift 越大，滤波越强，滞后也越明显 */
      filtered = (int32_t)g_mcFilteredMecSpeedUnit;
      filtered += ((int32_t)rawSpeedUnit - filtered) >> g_mcSpeedFilterLpfShift;
      break;
  }

  g_mcFilteredMecSpeedUnit = (int16_t)filtered;

  SpeedFilterSensorM1.hAvrMecSpeedUnit = g_mcFilteredMecSpeedUnit;
  SpeedFilterSensorM1.bSpeedErrorNumber = pSource->bSpeedErrorNumber;
  SpeedFilterSensorM1.hMecAccelUnitP = pSource->hMecAccelUnitP;
}

static void MC_SPEED_FilterSyncInstantaneousState(const SpeednPosFdbk_Handle_t *pSource)
{
  SpeedFilterSensorM1.hElAngle = pSource->hElAngle;
  SpeedFilterSensorM1.hMecAngle = pSource->hMecAngle;
  SpeedFilterSensorM1.wMecAngle = pSource->wMecAngle;
  SpeedFilterSensorM1.hElSpeedDpp = pSource->hElSpeedDpp;
  SpeedFilterSensorM1.InstantaneousElSpeedDpp = pSource->InstantaneousElSpeedDpp;
}

void MC_SPEED_SetFilterMode(MC_SpeedFilterMode_t mode)
{
  g_mcSpeedFilterMode = mode;
  MC_SPEED_FilterReset();
}

MC_SpeedFilterMode_t MC_SPEED_GetFilterMode(void)
{
  return g_mcSpeedFilterMode;
}

void MC_SPEED_SetLpfShift(uint8_t shift)
{
  if (shift > 6U)
  {
    shift = 6U;
  }
  else if (shift < 1U)
  {
    shift = 1U;
  }
  else
  {
    /* Nothing to do */
  }

  g_mcSpeedFilterLpfShift = shift;
  MC_SPEED_FilterReset();
}

uint8_t MC_SPEED_GetLpfShift(void)
{
  return g_mcSpeedFilterLpfShift;
}

int16_t MC_SPEED_GetRawSpeedUnit(void)
{
  return g_mcRawMecSpeedUnit;
}

int16_t MC_SPEED_GetFilteredSpeedUnit(void)
{
  return g_mcFilteredMecSpeedUnit;
}

int16_t MC_SPEED_GetRawSpeedRpm(void)
{
  return SPEED_UNIT_2_RPM(g_mcRawMecSpeedUnit);
}

int16_t MC_SPEED_GetFilteredSpeedRpm(void)
{
  return SPEED_UNIT_2_RPM(g_mcFilteredMecSpeedUnit);
}
/* USER CODE BEGIN mc_task 0 */

static const MC_ExperimentConfig_t *MC_EXPERIMENT_GetConfigTable(void)
{
#if (MC_DEMO_MODE == MC_DEMO_MODE_PLL_COMPARE)
  return g_mcPllCompareConfigs;
#elif (MC_DEMO_MODE == MC_DEMO_MODE_SWITCH_COMPARE)
  return g_mcSwitchCompareConfigs;
#else
  return g_mcFilterCompareConfigs;
#endif
}

static uint8_t MC_EXPERIMENT_GetConfigCount(void)
{
#if (MC_DEMO_MODE == MC_DEMO_MODE_PLL_COMPARE)
  return (uint8_t)(sizeof(g_mcPllCompareConfigs) / sizeof(g_mcPllCompareConfigs[0]));
#elif (MC_DEMO_MODE == MC_DEMO_MODE_SWITCH_COMPARE)
  return (uint8_t)(sizeof(g_mcSwitchCompareConfigs) / sizeof(g_mcSwitchCompareConfigs[0]));
#else
  return (uint8_t)(sizeof(g_mcFilterCompareConfigs) / sizeof(g_mcFilterCompareConfigs[0]));
#endif
}

static const char *MC_EXPERIMENT_GetSeriesName(void)
{
#if (MC_DEMO_MODE == MC_DEMO_MODE_PLL_COMPARE)
  return "pll_compare_demo";
#elif (MC_DEMO_MODE == MC_DEMO_MODE_SWITCH_COMPARE)
  return "switch_compare_demo";
#else
  return "filter_compare_demo";
#endif
}

static const MC_ExperimentConfig_t *MC_EXPERIMENT_GetCurrentConfig(void)
{
  const MC_ExperimentConfig_t *pTable = MC_EXPERIMENT_GetConfigTable();
  uint8_t configCount = MC_EXPERIMENT_GetConfigCount();

  if (g_mcExperimentConfigIndex >= configCount)
  {
    g_mcExperimentConfigIndex = 0U;
  }

  return &pTable[g_mcExperimentConfigIndex];
}

static void MC_EXPERIMENT_ApplyConfig(const MC_ExperimentConfig_t *pConfig)
{
  if (pConfig == MC_NULL)
  {
    return;
  }

  MC_SPEED_SetFilterMode(pConfig->mode);
  MC_SPEED_SetLpfShift(pConfig->lpfShift);
  g_mcAdaptiveFastShift = pConfig->adaptiveFastShift;
  g_mcAdaptiveSlowShift = pConfig->adaptiveSlowShift;
  g_mcAdaptiveEnterRpm = pConfig->adaptiveEnterRpm;
  g_mcAdaptiveExitRpm = pConfig->adaptiveExitRpm;
  g_mcAdaptiveConfirmN = pConfig->adaptiveConfirmN;
  g_mcStartupProtectEnable = pConfig->startupProtectEnable;
  g_mcStartupSwitchStableTick = 0U;
  g_mcStartupEstimateConfirmCounter = 0U;
  MC_PLL_TuningApply(pConfig);
}

static void MC_EXPERIMENT_StartSequence(void)
{
  const MC_ExperimentConfig_t *pConfig = MC_EXPERIMENT_GetCurrentConfig();

  MC_EXPERIMENT_ApplyConfig(pConfig);
  g_mcExperimentBoostIssued = 0U;
  g_mcExperimentReturnIssued = 0U;
  /* 先把第一次目标速度设成 500rpm，后续再按时间切到 1000rpm 再回到 500rpm */
  if (MC_StartMotor1() == false)
  {
    /* 启动命令没接收成功时，直接放弃本轮，避免卡在“只发表头”的假启动状态 */
    return;
  }

  (void)MCI_ExecSpeedRamp(&Mci[M1], RPM_2_SPEED_UNIT(MC_EXPERIMENT_BASE_SPEED_RPM), MC_EXPERIMENT_RAMP_TIME_MS);

  g_mcExperimentSessionId++;
  g_mcExperimentSampleIndex = 0U;
  g_mcExperimentStartTick = HAL_GetTick();
  g_mcExperimentLastSampleTick = g_mcExperimentStartTick;
  g_mcExperimentStopTick = 0U;
  g_mcExperimentHeaderPending = 1U;
  g_mcExperimentFooterPending = 0U;
  g_mcExperimentAdvanceConfigPending = 0U;
  g_mcExperimentStopReason = MC_EXPERIMENT_STOP_BY_USER;
  g_mcExperimentPhase = MC_EXPERIMENT_PHASE_START;
  g_mcExperimentTextModeEnabled = 1U;
  g_mcExperimentAutoStopIssued = 0U;
  g_mcExperimentAutoBatchArmed = 1U;
  g_mcExperimentNextStartTick = 0U;

  MC_EXPERIMENT_PushMetaFrame(MC_EXPERIMENT_PHASE_START, MC_EXPERIMENT_STOP_BY_USER);
}

static void MC_EXPERIMENT_RequestStop(uint8_t stopReason)
{
  if ((g_mcExperimentPhase == MC_EXPERIMENT_PHASE_STOP) ||
      (g_mcExperimentPhase == MC_EXPERIMENT_PHASE_DONE) ||
      (g_mcExperimentTextModeEnabled == 0U))
  {
    return;
  }

  g_mcExperimentStopReason = stopReason;
  g_mcExperimentPhase = MC_EXPERIMENT_PHASE_STOP;
  g_mcExperimentAutoStopIssued = 1U;
  g_mcExperimentStopTick = HAL_GetTick();
  (void)MC_StopMotor1();
}

static void MC_EXPERIMENT_HandlePeriodicSample(void)
{
  uint32_t nowTick;

  if (g_mcExperimentTextModeEnabled == 0U)
  {
    return;
  }

  if ((g_mcExperimentPhase != MC_EXPERIMENT_PHASE_START) &&
      (g_mcExperimentPhase != MC_EXPERIMENT_PHASE_RUN) &&
      (g_mcExperimentPhase != MC_EXPERIMENT_PHASE_STOP))
  {
    return;
  }

  nowTick = HAL_GetTick();
  if ((nowTick - g_mcExperimentLastSampleTick) < MC_EXPERIMENT_SAMPLE_PERIOD_MS)
  {
    return;
  }

  g_mcExperimentLastSampleTick = nowTick;
  MC_EXPERIMENT_PushSample((uint8_t)g_mcExperimentPhase, g_mcExperimentStopReason);

  MC_EXPERIMENT_UpdateSpeedProfile();
}

static void MC_EXPERIMENT_PushSample(uint8_t phase, uint8_t stopReason)
{
  MC_ExperimentSample_t sample;
  const MC_ExperimentConfig_t *pConfig = MC_EXPERIMENT_GetCurrentConfig();

  if (MC_EXPERIMENT_IsQueueFull() != 0U)
  {
    return;
  }

  (void)memset(&sample, 0, sizeof(sample));
  sample.sessionId = g_mcExperimentSessionId;
  sample.configIndex = (uint16_t)g_mcExperimentConfigIndex;
  sample.sampleIndex = g_mcExperimentSampleIndex++;
  sample.timeMs = MC_EXPERIMENT_GetElapsedMs();
  sample.targetSpeedRpm = (int16_t)MC_GetMecSpeedReferenceMotor1_F();
  sample.rawSpeedRpm = MC_SPEED_GetRawSpeedRpm();
  sample.filteredSpeedRpm = MC_SPEED_GetFilteredSpeedRpm();
  sample.finalSpeedRpm = (int16_t)MC_GetAverageMecSpeedMotor1_F();
  sample.pllKp = g_mcPllActiveKp;
  sample.pllKi = g_mcPllActiveKi;
  sample.pllStage = g_mcPllCurrentStage;
  sample.pllSplitEnable = g_mcPllSplitEnable;
  sample.phase = phase;
  sample.stopReason = stopReason;
  (void)snprintf(sample.methodName, sizeof(sample.methodName), "%s", pConfig->methodName);
  (void)snprintf(sample.paramName, sizeof(sample.paramName), "%s", pConfig->paramName);

  MC_EXPERIMENT_QueueWrite(&sample);
}

static void MC_EXPERIMENT_PushMetaFrame(uint8_t phase, uint8_t stopReason)
{
  MC_ExperimentSample_t sample;
  const MC_ExperimentConfig_t *pConfig = MC_EXPERIMENT_GetCurrentConfig();

  if (MC_EXPERIMENT_IsQueueFull() != 0U)
  {
    return;
  }

  (void)memset(&sample, 0, sizeof(sample));
  sample.sessionId = g_mcExperimentSessionId;
  sample.configIndex = (uint16_t)g_mcExperimentConfigIndex;
  sample.sampleIndex = 0U;
  sample.timeMs = MC_EXPERIMENT_GetElapsedMs();
  sample.targetSpeedRpm = (int16_t)MC_GetMecSpeedReferenceMotor1_F();
  sample.pllKp = g_mcPllActiveKp;
  sample.pllKi = g_mcPllActiveKi;
  sample.pllStage = g_mcPllCurrentStage;
  sample.pllSplitEnable = g_mcPllSplitEnable;
  sample.phase = phase;
  sample.stopReason = stopReason;
  (void)snprintf(sample.methodName, sizeof(sample.methodName), "%s", pConfig->methodName);
  (void)snprintf(sample.paramName, sizeof(sample.paramName), "%s", pConfig->paramName);

  MC_EXPERIMENT_QueueWrite(&sample);
}

static uint8_t MC_EXPERIMENT_IsQueueFull(void)
{
  return (g_mcExperimentQueueCount >= MC_EXPERIMENT_QUEUE_DEPTH) ? 1U : 0U;
}

static uint8_t MC_EXPERIMENT_IsQueueEmpty(void)
{
  return (g_mcExperimentQueueCount == 0U) ? 1U : 0U;
}

static void MC_EXPERIMENT_QueueWrite(const MC_ExperimentSample_t *pSample)
{
  if ((pSample == MC_NULL) || (MC_EXPERIMENT_IsQueueFull() != 0U))
  {
    return;
  }

  g_mcExperimentQueue[g_mcExperimentQueueWrite] = *pSample;
  g_mcExperimentQueueWrite++;
  if (g_mcExperimentQueueWrite >= MC_EXPERIMENT_QUEUE_DEPTH)
  {
    g_mcExperimentQueueWrite = 0U;
  }
  g_mcExperimentQueueCount++;
}

static uint8_t MC_EXPERIMENT_QueueRead(MC_ExperimentSample_t *pSample)
{
  if ((pSample == MC_NULL) || (MC_EXPERIMENT_IsQueueEmpty() != 0U))
  {
    return 0U;
  }

  *pSample = g_mcExperimentQueue[g_mcExperimentQueueRead];
  g_mcExperimentQueueRead++;
  if (g_mcExperimentQueueRead >= MC_EXPERIMENT_QUEUE_DEPTH)
  {
    g_mcExperimentQueueRead = 0U;
  }
  g_mcExperimentQueueCount--;
  return 1U;
}

static void MC_EXPERIMENT_SendTextIfPossible(void)
{
  MC_ExperimentSample_t sample;

  if (g_mcExperimentTextModeEnabled == 0U)
  {
    return;
  }

  if (g_mcExperimentHeaderPending != 0U)
  {
    int length = snprintf(g_mcExperimentTxBuffer,
                          sizeof(g_mcExperimentTxBuffer),
                          "time_ms,target_speed_rpm,raw_speed_rpm,filtered_speed_rpm,final_speed_rpm,pll_kp,pll_ki,pll_stage,pll_split_enable,session_id,config_index,method_name,param_tag,phase,stop_reason,sample_index\r\n");
    g_mcExperimentHeaderPending = 0U;
    if (length > 0)
    {
      (void)HAL_UART_Transmit(&huart2, (uint8_t *)g_mcExperimentTxBuffer, (uint16_t)length, 100U);
    }
    return;
  }

  if (MC_EXPERIMENT_QueueRead(&sample) == 0U)
  {
    return;
  }

  MC_EXPERIMENT_FormatCsvLine(&sample, g_mcExperimentTxBuffer, sizeof(g_mcExperimentTxBuffer));
  (void)HAL_UART_Transmit(&huart2, (uint8_t *)g_mcExperimentTxBuffer, (uint16_t)strlen(g_mcExperimentTxBuffer), 100U);
}

static void MC_EXPERIMENT_FormatCsvLine(const MC_ExperimentSample_t *pSample, char *pBuffer, uint16_t bufferSize)
{
  if ((pSample == MC_NULL) || (pBuffer == MC_NULL) || (bufferSize == 0U))
  {
    return;
  }

  (void)snprintf(pBuffer,
                 bufferSize,
                 "%lu,%d,%d,%d,%d,%d,%d,%u,%u,%u,%u,%s,%s,%s,%s,%lu\r\n",
                 (unsigned long)pSample->timeMs,
                 pSample->targetSpeedRpm,
                 pSample->rawSpeedRpm,
                 pSample->filteredSpeedRpm,
                 pSample->finalSpeedRpm,
                 pSample->pllKp,
                 pSample->pllKi,
                 pSample->pllStage,
                 pSample->pllSplitEnable,
                 pSample->sessionId,
                 pSample->configIndex,
                 pSample->methodName,
                 pSample->paramName,
                 MC_EXPERIMENT_PhaseName(pSample->phase),
                 MC_EXPERIMENT_StopReasonName(pSample->stopReason),
                 (unsigned long)pSample->sampleIndex);
}

static void MC_EXPERIMENT_UpdateStateByMotorState(void)
{
  MCI_State_t motorState = MC_GetSTMStateMotor1();

  if (g_mcExperimentTextModeEnabled == 0U)
  {
    return;
  }

  MC_EXPERIMENT_AbortStartIfNeeded(motorState);
  if (g_mcExperimentTextModeEnabled == 0U)
  {
    return;
  }

  if ((motorState == RUN) && (g_mcExperimentPhase == MC_EXPERIMENT_PHASE_START))
  {
    g_mcExperimentPhase = MC_EXPERIMENT_PHASE_RUN;
    MC_EXPERIMENT_PushMetaFrame(MC_EXPERIMENT_PHASE_RUN, g_mcExperimentStopReason);
  }

  if ((g_mcExperimentPhase == MC_EXPERIMENT_PHASE_STOP) &&
      (MC_EXPERIMENT_IsStopFinished(motorState) != 0U))
  {
    MC_EXPERIMENT_PushMetaFrame(MC_EXPERIMENT_PHASE_DONE, g_mcExperimentStopReason);
    g_mcExperimentPhase = MC_EXPERIMENT_PHASE_DONE;
    g_mcExperimentFooterPending = 1U;
    g_mcExperimentAdvanceConfigPending = 1U;
  }

  if ((motorState == FAULT_NOW) || (motorState == FAULT_OVER))
  {
    g_mcExperimentStopReason = MC_EXPERIMENT_STOP_BY_FAULT;
    if (g_mcExperimentPhase != MC_EXPERIMENT_PHASE_DONE)
    {
      MC_EXPERIMENT_PushMetaFrame(MC_EXPERIMENT_PHASE_DONE, g_mcExperimentStopReason);
      g_mcExperimentPhase = MC_EXPERIMENT_PHASE_DONE;
      g_mcExperimentFooterPending = 1U;
      g_mcExperimentAdvanceConfigPending = 1U;
    }
  }
}

static uint32_t MC_EXPERIMENT_GetElapsedMs(void)
{
  return HAL_GetTick() - g_mcExperimentStartTick;
}

static const char *MC_EXPERIMENT_PhaseName(uint8_t phase)
{
  switch (phase)
  {
    case MC_EXPERIMENT_PHASE_START:
      return "start";

    case MC_EXPERIMENT_PHASE_RUN:
      return "run";

    case MC_EXPERIMENT_PHASE_STOP:
      return "stop";

    case MC_EXPERIMENT_PHASE_DONE:
      return "done";

    case MC_EXPERIMENT_PHASE_IDLE:
    default:
      return "idle";
  }
}

static const char *MC_EXPERIMENT_StopReasonName(uint8_t stopReason)
{
  if (stopReason == MC_EXPERIMENT_STOP_BY_FAULT)
  {
    return "fault";
  }

  return "user";
}

static uint8_t MC_EXPERIMENT_IsStartAllowed(void)
{
  MCI_State_t motorState = MC_GetSTMStateMotor1();

  /* 这里把启动条件卡严一点，避免上一组结尾串口还没发完就立刻开始下一组 */
  if (g_mcExperimentPhase != MC_EXPERIMENT_PHASE_IDLE)
  {
    return 0U;
  }

  if (g_mcExperimentTextModeEnabled != 0U)
  {
    return 0U;
  }

  if (g_mcExperimentFooterPending != 0U)
  {
    return 0U;
  }

  if ((g_mcExperimentAutoBatchArmed != 0U) &&
      (HAL_GetTick() < g_mcExperimentNextStartTick))
  {
    return 0U;
  }

  if (g_mcExperimentAdvanceConfigPending != 0U)
  {
    return 0U;
  }

  if (motorState != IDLE)
  {
    return 0U;
  }

  return 1U;
}

static uint8_t MC_EXPERIMENT_IsStopFinished(MCI_State_t motorState)
{
  int16_t absFinalSpeedRpm = MC_SPEED_GetFilteredSpeedRpm();
  uint32_t stopElapsedMs = 0U;

  if (absFinalSpeedRpm < 0)
  {
    absFinalSpeedRpm = (int16_t)(-absFinalSpeedRpm);
  }

  if (g_mcExperimentStopTick != 0U)
  {
    stopElapsedMs = HAL_GetTick() - g_mcExperimentStopTick;
  }

  if ((motorState == IDLE) || (motorState == STOP))
  {
    if ((absFinalSpeedRpm <= MC_EXPERIMENT_STOP_SPEED_EPS_RPM) &&
        (stopElapsedMs >= (uint32_t)MC_EXPERIMENT_STOP_CAPTURE_MS))
    {
      return 1U;
    }
  }

  if (MC_EXPERIMENT_GetElapsedMs() >=
      ((uint32_t)MC_EXPERIMENT_TOTAL_RUN_MS + (uint32_t)MC_EXPERIMENT_STOP_SETTLE_MS))
  {
    return 1U;
  }

  return 0U;
}

static void MC_EXPERIMENT_AbortStartIfNeeded(MCI_State_t motorState)
{
  if (g_mcExperimentPhase != MC_EXPERIMENT_PHASE_START)
  {
    return;
  }

  if (motorState == RUN)
  {
    return;
  }

  if (MC_EXPERIMENT_GetElapsedMs() < (uint32_t)MC_EXPERIMENT_START_TIMEOUT_MS)
  {
    return;
  }

  g_mcExperimentHeaderPending = 0U;
  g_mcExperimentFooterPending = 0U;
  g_mcExperimentAdvanceConfigPending = 0U;
  g_mcExperimentTextModeEnabled = 0U;
  g_mcExperimentPhase = MC_EXPERIMENT_PHASE_IDLE;
  g_mcExperimentAutoStopIssued = 0U;
  g_mcExperimentBoostIssued = 0U;
  g_mcExperimentReturnIssued = 0U;
  g_mcExperimentAutoBatchArmed = 0U;
}

void MC_EXPERIMENT_BackgroundTask(void)
{
  MC_EXPERIMENT_SendTextIfPossible();

  if ((g_mcExperimentFooterPending != 0U) &&
      (MC_EXPERIMENT_IsQueueEmpty() != 0U))
  {
    uint8_t nextConfigIndex = g_mcExperimentConfigIndex;
    uint8_t shouldAutoStart = 0U;
    int length = snprintf(g_mcExperimentTxBuffer,
                          sizeof(g_mcExperimentTxBuffer),
                          "#session_end,%u,%u,%s,%s\r\n",
                          g_mcExperimentSessionId,
                          g_mcExperimentConfigIndex,
                          MC_EXPERIMENT_StopReasonName(g_mcExperimentStopReason),
                          MC_EXPERIMENT_GetSeriesName());

    if (g_mcExperimentAdvanceConfigPending != 0U)
    {
      nextConfigIndex++;
      if (nextConfigIndex >= MC_EXPERIMENT_GetConfigCount())
      {
        nextConfigIndex = g_mcExperimentConfigIndex;
        shouldAutoStart = 0U;
      }
      else
      {
        shouldAutoStart = 1U;
      }
    }

    /* 这里先把状态切回空闲，避免串口返回值不稳定时一直狂刷 session_end */
    g_mcExperimentFooterPending = 0U;
    g_mcExperimentTextModeEnabled = 0U;
    g_mcExperimentPhase = MC_EXPERIMENT_PHASE_IDLE;
    g_mcExperimentAdvanceConfigPending = 0U;
    g_mcExperimentAutoStopIssued = 0U;
    g_mcExperimentBoostIssued = 0U;
    g_mcExperimentReturnIssued = 0U;
    /* 最后一组跑完后必须明确撤掉自动连跑标志，
       否则会在 cfg 保持不变时重复启动最后一组。 */
    g_mcExperimentAutoBatchArmed = 0U;
    g_mcExperimentNextStartTick = 0U;
    g_mcExperimentConfigIndex = nextConfigIndex;
    if (shouldAutoStart != 0U)
    {
      g_mcExperimentAutoBatchArmed = 1U;
      g_mcExperimentNextStartTick = HAL_GetTick() + MC_EXPERIMENT_INTER_SESSION_WAIT_MS;
    }

    if (length > 0)
    {
      (void)HAL_UART_Transmit(&huart2, (uint8_t *)g_mcExperimentTxBuffer, (uint16_t)length, 100U);
    }
  }

  if ((g_mcExperimentAutoBatchArmed != 0U) &&
      (g_mcExperimentPhase == MC_EXPERIMENT_PHASE_IDLE) &&
      (g_mcExperimentTextModeEnabled == 0U) &&
      (g_mcExperimentFooterPending == 0U) &&
      (g_mcExperimentAdvanceConfigPending == 0U) &&
      (MC_GetSTMStateMotor1() == IDLE) &&
      (HAL_GetTick() >= g_mcExperimentNextStartTick))
  {
    g_mcExperimentAutoBatchArmed = 0U;
    MC_EXPERIMENT_StartSequence();
  }
}

static void MC_EXPERIMENT_UpdateSpeedProfile(void)
{
  uint32_t elapsedMs;

  if ((g_mcExperimentTextModeEnabled == 0U) ||
      ((g_mcExperimentPhase != MC_EXPERIMENT_PHASE_START) &&
       (g_mcExperimentPhase != MC_EXPERIMENT_PHASE_RUN)))
  {
    return;
  }

  elapsedMs = MC_EXPERIMENT_GetElapsedMs();

  if ((g_mcExperimentBoostIssued == 0U) &&
      (elapsedMs >= (uint32_t)MC_EXPERIMENT_SPEED_HOLD_MS))
  {
    (void)MCI_ExecSpeedRamp(&Mci[M1], RPM_2_SPEED_UNIT(MC_EXPERIMENT_BOOST_SPEED_RPM), MC_EXPERIMENT_RAMP_TIME_MS);
    g_mcExperimentBoostIssued = 1U;
  }

  if ((g_mcExperimentReturnIssued == 0U) &&
      (elapsedMs >= ((uint32_t)MC_EXPERIMENT_SPEED_HOLD_MS + (uint32_t)MC_EXPERIMENT_SPEED_HOLD_MS)))
  {
    (void)MCI_ExecSpeedRamp(&Mci[M1], RPM_2_SPEED_UNIT(MC_EXPERIMENT_BASE_SPEED_RPM), MC_EXPERIMENT_RAMP_TIME_MS);
    g_mcExperimentReturnIssued = 1U;
  }

  if ((g_mcExperimentAutoStopIssued == 0U) &&
      (elapsedMs >= (uint32_t)MC_EXPERIMENT_TOTAL_RUN_MS))
  {
    MC_EXPERIMENT_RequestStop(MC_EXPERIMENT_STOP_BY_USER);
  }
}

static void MC_PLL_TuningReset(void)
{
  g_mcPllCurrentStage = 0U;
  g_mcPllStableCounter = 0U;
  g_mcPllActiveKp = g_mcPllFastKp;
  g_mcPllActiveKi = g_mcPllFastKi;
  STO_SetPLLGains(&STO_PLL_M1, g_mcPllActiveKp, g_mcPllActiveKi);
  STO_ResetPLL(&STO_PLL_M1);
}

static void MC_PLL_TuningApply(const MC_ExperimentConfig_t *pConfig)
{
  if (pConfig == MC_NULL)
  {
    return;
  }

  g_mcPllSplitEnable = pConfig->pllSplitEnable;
  g_mcPllFastKp = pConfig->pllFastKp;
  g_mcPllFastKi = pConfig->pllFastKi;
  g_mcPllSlowKp = pConfig->pllSlowKp;
  g_mcPllSlowKi = pConfig->pllSlowKi;
  g_mcPllSplitEnterRpm = pConfig->pllSplitEnterRpm;
  g_mcPllSplitExitRpm = pConfig->pllSplitExitRpm;
  g_mcPllSplitConfirmN = pConfig->pllSplitConfirmN;
  MC_PLL_TuningReset();
}

static void MC_PLL_TuningUpdate(void)
{
  int16_t absSpeedRpm;
  uint8_t nextStage;

  if (g_mcPllSplitEnable == 0U)
  {
    return;
  }

  absSpeedRpm = MC_SPEED_GetFilteredSpeedRpm();
  if (absSpeedRpm < 0)
  {
    absSpeedRpm = (int16_t)(-absSpeedRpm);
  }

  nextStage = g_mcPllCurrentStage;

  if (g_mcPllCurrentStage == 0U)
  {
    if (absSpeedRpm >= g_mcPllSplitEnterRpm)
    {
      if (g_mcPllStableCounter < g_mcPllSplitConfirmN)
      {
        g_mcPllStableCounter++;
      }
      if (g_mcPllStableCounter >= g_mcPllSplitConfirmN)
      {
        nextStage = 1U;
      }
    }
    else
    {
      g_mcPllStableCounter = 0U;
    }
  }
  else
  {
    if (absSpeedRpm <= g_mcPllSplitExitRpm)
    {
      if (g_mcPllStableCounter < g_mcPllSplitConfirmN)
      {
        g_mcPllStableCounter++;
      }
      if (g_mcPllStableCounter >= g_mcPllSplitConfirmN)
      {
        nextStage = 0U;
      }
    }
    else
    {
      g_mcPllStableCounter = 0U;
    }
  }

  if (nextStage != g_mcPllCurrentStage)
  {
    g_mcPllCurrentStage = nextStage;
    g_mcPllStableCounter = 0U;
    if (g_mcPllCurrentStage == 0U)
    {
      g_mcPllActiveKp = g_mcPllFastKp;
      g_mcPllActiveKi = g_mcPllFastKi;
    }
    else
    {
      g_mcPllActiveKp = g_mcPllSlowKp;
      g_mcPllActiveKi = g_mcPllSlowKi;
    }
    STO_SetPLLGains(&STO_PLL_M1, g_mcPllActiveKp, g_mcPllActiveKi);
  }
}

static uint8_t MC_STARTUP_IsSwitchReady(uint8_t loopClosed)
{
  int16_t filteredSpeedRpm;
  int16_t rawSpeedRpm;
  uint32_t nowTick;

  if (loopClosed == 0U)
  {
    g_mcStartupSwitchStableTick = 0U;
    g_mcStartupEstimateConfirmCounter = 0U;
    return 0U;
  }

  if (g_mcStartupProtectEnable == 0U)
  {
    return 1U;
  }

  filteredSpeedRpm = MC_SPEED_GetFilteredSpeedRpm();
  if (filteredSpeedRpm < MC_STARTUP_SWITCH_MIN_SPEED_RPM)
  {
    g_mcStartupSwitchStableTick = 0U;
    g_mcStartupEstimateConfirmCounter = 0U;
    return 0U;
  }

  rawSpeedRpm = MC_SPEED_GetRawSpeedRpm();
  if (rawSpeedRpm < 0)
  {
    g_mcStartupSwitchStableTick = 0U;
    g_mcStartupEstimateConfirmCounter = 0U;
    return 0U;
  }

  if (g_mcStartupEstimateConfirmCounter < MC_STARTUP_ESTIMATE_CONFIRM_SAMPLES)
  {
    g_mcStartupEstimateConfirmCounter++;
  }

  if (g_mcStartupEstimateConfirmCounter < MC_STARTUP_ESTIMATE_CONFIRM_SAMPLES)
  {
    return 0U;
  }

  nowTick = HAL_GetTick();
  if (g_mcStartupSwitchStableTick == 0U)
  {
    g_mcStartupSwitchStableTick = nowTick;
    return 0U;
  }

  if ((nowTick - g_mcStartupSwitchStableTick) < (uint32_t)MC_STARTUP_SWITCH_STABLE_MS)
  {
    return 0U;
  }

  return 1U;
}

/* USER CODE END mc_task 0 */

/******************* (C) COPYRIGHT 2019 STMicroelectronics *****END OF FILE****/
