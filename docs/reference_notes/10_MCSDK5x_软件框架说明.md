# MCSDK 5.x 电机库软件框架说明

- Source: `/mnt/e/BaiduNetdiskDownload/控制课程设计/华科电机课件/MC SDK5.x电机库软件框架说明20190513.pdf`
- Note: 后续实现速度估计/滤波算法时，需要知道工作台生成代码和控制环的位置。

## Extracted Text
MC SDK5.x

MC SDK5.x
ST STM32F30x

1.

MC SDK5.x ST HAL/LL
FOC
MC SDK5.x UI Workbench UI
FOC

SDK MC workbench MC
Workbench CubeMx + + + +
API MC_StartMotor1

1.1

Cube
TIMERADCGPIO
1.2

API

bus_voltage_sensor.c
circle_limitation.c
enc_align_ctrl.c
encoder_speed_pos_fdbk.c
fast_div.c
hall_speed_pos_fdbk.c Hall
inrush_current_limiter.c
mc_math.c
mc_interface.c
motor_power_measurement.c
ntc_temperature_sensor.c NTC
open_loop.c
pid_regulator.c PID
pqd_motor_power_measurement.c
pwm_common.c TIMER
pwm_curr_fdbk.c SVPWMADC
r_divider_bus_voltage_sensor.c
virtual_bus_voltage_sensor.c
ramp_ext_mngr.c
speed_pos_fdbk.c
speed_torq_ctrl.c
state_machine.c
virtual_speed_sensor.c

ics_f30x_pwm_curr_fdbk.c STM32F3 ICS

r1_f30x_pwm_curr_fdbk.c STM32F3
r3_1_f30x_pwm_curr_fdbk.c STM32F3 1 ADC
r3_2_f30x_pwm_curr_fdbk.c STM32F3 2 ADC
r3_4_f30x_pwm_curr_fdbk.c STM32F3 4 ADC
1.3

API API

Motor2
MC_StartMotor2(),

MC_StartMotor1 void bool
MC_StopMotor1 void bool
MC_ProgramSpeedRampMotor1 speed,time void
MC_ProgramTorqueRampMotor1 torque,time void
MC_SetCurrentReferenceMotor1 IqrefIdref void IqId
MC_GetCommandStateMotor1 void MCI_CommandState_t
,

MC_StopSpeedRampMotor1 void bool
bool
MC_HasRampCompletedMotor1 void int16
int16
MC_GetMecSpeedReferenceMotor1 void int16
STC_Modality_t
MC_GetMecSpeedAverageMotor1 void int16
bool
MC_GetLastRampFinalSpeedMotor1 void Is
Vs
MC_GetControlModeMotor1 void IaIb ab

MC_GetImposedDirectionMotor1 void

MC_GetSpeedSensorReliabilityMotor1 void

MC_GetPhaseCurrentAmplitudeMotor1 void

MC_GetPhaseVoltageAmplitudeMotor1 void

MC_GetIabMotor1 void

MC_GetIalphabetaMotor1 void II clark II

MC_GetIqdMotor1 void IdIq park IdIq

MC_GetIqdrefMotor1 void IdrefIqref IdIq

MC_GetVqdMotor1 void VdVq VdVq

MC_GetValphabetaMotor1 void VV VV

MC_GetElAngledppMotor1 void Angledpp DPP
Iqref
MC_GetTerefMotor1 void void Id
void IqId
MC_SetIdrefMotor1 Idref Bool

MC_Clear_IqdrefMotor1 void

MC_AcknowledgeFaultMotor1 void
MC_GetOccurredFaultsMotor1 void Fault Fault
Fault Fault
MC_GetCurrentFaultsMotor1 void State

MC_GetSTMStateMotor1 void

2.

FOC

3.

STM32 FOC
3.1 Main

main.c main

3.2

3.2.1 ADC

ADC ADC TIM1 FOC
SVPWM TIM1 PWM

XXX STM32F302R8 xxx r3_1_f30x
r3_1_f30x_pwm_curr_fdbk.c R3_1_F30X_WriteTIMRegisters()
3.2.2 Systick

500us 500us
2ms 500us*4 systick
4.

4.1

mc_task.c TSK_HighFrequencyTask() FOC
4.2

PWM ADC ADC

4.3

systick

5.

5.1

TIM1 TIMER ADC ADC

Ia+Ib+Ic=0 IaIb

5.1.1 API

API MC_GetIabMotor1() Curr_Components

Curr_Components MC_GetIabMotor1(void)
{

return MCI_GetIab( pMCI[M1] );
}

typedef struct
{

int16_t qI_Component1;
int16_t qI_Component2;
} Curr_Components;

IaIb int16

Curr_Components Iab_Value;
Iab_Value = MC_GetIabMotor1() ;
Ia = Iab_Value. qI_Component1 ;
Ib = Iab_Value. qI_Component2 ;
5.1.2

mc_task.c FOCVars IaIb

Curr_Components Iab_Value;
Iab_Value = FOCVars[0].Iab;
Ia = Iab_Value. qI_Component1 ;
Ib = Iab_Value. qI_Component2 ;
ADC ADC ST IaIb
Inject JDR1JDR2
JDR1 Ia Ib
5.2

API TSK_SafetyTask()

5.2.1

bus_voltage_sensor.c VBS_GetAvBusVoltage_d()
VBS_GetAvBusVoltage_V()V
MCT mc_task.c

uint16_t Bus_Voltage_D, Bus_Voltage_V;
Bus_Voltage_D = VBS_GetAvBusVoltage_d(MCT[0].pBusVoltageSensor);
Bus_Voltage_V = VBS_GetAvBusVoltage_V(MCT[0].pBusVoltageSensor);
5.2.2

RDivider_Handle_t
BusVoltageSensor_Handle_t _Super;

uint16_t Bus_Voltage_D;
Bus_Voltage_D = pBusSensorM1->_Super.AvBusVoltage_d;
5.3

START RUN RUN

state_machine.h State_t USER_DEFINE_STATE = 21,

mc_task.c TSK_MediumFrequencyTaskM1()START RUN USER_DEFINE_STATE
RUN USER_DEFINE_STATE ;
state_machine.c

STM_NextState()
state_machine.c

6.

MC SDK
API ST

ADC Systick
