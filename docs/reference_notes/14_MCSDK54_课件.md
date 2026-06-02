# MC SDK 5.4 课件

- Source: `/mnt/e/BaiduNetdiskDownload/控制课程设计/华科电机课件/MC SDK5.4_2020-03.pptx`
- Note: 补充 Workbench/SDK 使用与参数配置背景。

## Extracted Text
## Slide 1

STM32 MC SDK5.4
意法半导体
微控制器部门

## Slide 2

电机实时交互调试
10
实时参数调整
仪表盘
速度示波器

## Slide 3

100
其他功能

## Slide 4

101
欠压过压检测及保护

## Slide 5

102
用户接口

## Slide 6

103
Workbench
中不能选择
HSI
MCU
及时钟源选择

## Slide 7

104
数字
I/O

## Slide 8

105
DAC
功能

## Slide 9

106
模拟输入和保护
(1)

## Slide 10

107
模拟输入和保护
(2)

## Slide 11

108
模拟输入和保护
(3)

## Slide 12

109
模拟输入和保护
(4)

## Slide 13

MC SDK5.x
电机控制代码自动生成
11
Motor Control
Workbench

## Slide 14

110
所有的参数配置完成后，点击生成图标，可根据所选的
IDE
生成
MC
应用工程：
用户不再需要将
MC
Workbench
输出文件拷贝到其工程下。
STM32CubeM
X
作为接口被
MC Workbench
后台调用，来生成选择的
IDE
项目工作框架。
MC
工程的生成
(1)

## Slide 15

111
如果当前工程尚未保存，会显示一个文件管理器窗口，询问是否将当前工程设置保存为一个新项目。
如果当前工程已保存过，会弹出右侧的窗口，进行
CubeMX
版本，
IDE
的选择和
HAL/LL
的选择。
MC
工程的生成
(2)

## Slide 16

112
开始生成时，会显示一个进程窗口，告知用户脚本在执行，完成后的提示窗口如下图所示，可关闭该窗口。用户信息表也随之更新。
MC
工程的生成
(3)

## Slide 17

113
生成的
IAR
工程文件保存在如下路径：
$
ProjectFolder
\EWARM
如果不需要配置其他外设，可直接打开
.
eww
文件。
如果需要配置其他外设，请用
CubeMX
打开
.
ioc
文件进行添加。
MC
工程的生成
(4)

## Slide 18

114
打开
.
ioc
文件，
STM32CubeMX
将被打开。
图形化的引脚分配配置
CubeMX
(1)

## Slide 19

115
时钟树结构
CubeMX
(2)

## Slide 20

116
配置
CubeMX
(3)

## Slide 21

117
STM32Fxx
Adv. Timer
ADC
Inverter
DMA
OPAMP
在
CubeMX
中请不要修改电机控制相关外设的设定
!
Aux. Timer
COMP
CubeMX
(4)

## Slide 22

118
代码生成：
如果对于其他外设有进行修改时，才进行重新生成操作
CubeMX
(5)

## Slide 23

119
通信链路
状态一览
电机控制按钮
Dashboard area
电机控制和监控
(1)

## Slide 24

ST
MC SDK 5.
x
–
性能测试结果
12

## Slide 25

120
电机控制和监控
(2)

## Slide 26

121
电机控制和监控
(3)

## Slide 27

122
电机控制和监控
(4)

## Slide 28

123
程序架构
代码生成流程

## Slide 29

124
程序架构
用户界面库
UI
Library
电机驾驶舱
Motor
Control
Cockpit
电机控制库
Motor
Control
Library
整个软件库由三部分组成

## Slide 30

125
程序架构
电机控制动态
实现对电机动的动态控制：
FOC
控制环路
(
高频任务
)
电机控制环路
(
中频任务
)
安全控制环路
(
安全任务
)
电机控制接口
通过
MC API
来实现
电机控制配置
实例化并配置所有需要的组件。
电机驾驶舱
Motor Control Cockpit
由三部分构成

## Slide 31

126
程序架构
是诸多组件的集合。每一个组件实现电机控制的一个功能例如，速度和位置检测
,
电流检测
,
PID
算法等等
…
电机控制库

## Slide 32

127
程序架构
X-CUBE-MCSDK_5.x
中下列组件均以库的形式提供。用户无法看到源码，但可以根据提供的头文件从库里调用相应的函数。
组件名称
描述
feed_forward_ctrl.c
前馈控制
flux_weakening_ctrl.c
弱磁控制
max_torque_per_ampere.c
最大转矩控制
sto_cordic_speed_pos_fdbk.c
速度和位置反馈
cordic
sto_pll_speed_pos_fdbk.c
速度和位置反馈
PLL
revup_ctrl.c
启动控制

## Slide 33

128
程序架构
用户界面库
包含负责通讯的组件。电机控制代码通过这些组件控制串口和
DAC
与外界通讯。通过这个库我们可以连接
MCU
和
Workbench
。在
Workbench
中实现对电机运行状态的监控。
用户界面库

## Slide 34

129
程序架构
FOC
控制环路
电机控制环路
安全控制环路

## Slide 35

双环控制
13
速度环
+
电流环
位置环
+
电流环
V5.4
新增位置环

## Slide 36

程序整体框架图
130
电机库控制过程都发生在中断中
电机控制实时控制
如果添加其他中断，注意中断优先级设定

## Slide 37

两个重要中断
—ADC
转换完成中断
131
高频任务执行于
ADC
采样转换完成中断
ADC
采样开始由
TIM1
硬件触发
中断中执行
FOC
坐标变换
SVPWM
的执行
TIM1
的
PWM
占空比调整输出

## Slide 38

两个重要中断
—
Systick
中断
132
中断默认为
500us
的定时
安全任务执行在这个中断中
中频任务（速度环）也执行在这个中断中

## Slide 39

高频任务
133
高频任务函数位于
mc_task.c
的
TSK_HighFrequencyTask
()
函数
执行的是核心
FOC
算法

## Slide 40

安全任务
134
这个任务对于过温，过流，欠压，过压保护进行判断
如果触发了上述保护，则会关闭
PWM
输出
这个任务是温度
ADC
采样，母线电压采样，用户
ADC
采样的执行地方

## Slide 41

中频任务
135
中频任务执行于
systick
中断中
实际为速度环以及状态机执行地方
可根据需要需要增加实际应用状态机

## Slide 42

API
函数在
MC SDK5.x
中的位置
136
一般的电机操作调用
API
就足够控制基本的电机运行
更像是行为描述操作
基于电机库，用于用户调用

## Slide 43

API
函数列表（
1/2
）
137
函数名称
函数参量
函数返回
函数功能
MC_StartMotor1
void
bool
启动电机
MC_StopMotor1
void
bool
停止电机
MC_ProgramSpeedRampMotor1
speed,time
void
设定速度以及时间
MC_ProgramTorqueRampMotor1
torque,time
void
设定力矩以及时间
MC_SetCurrentReferenceMotor1
Iqref，Idref
void
设定
Iq，Id
参考
MC_GetCommandStateMotor1
void
MCI_CommandState_t
返回指令执行状态
MC_StopSpeedRampMotor1
void
bool
停止速度指令执行
MC_HasRampCompletedMotor1
void
bool
指令是否执行完成
MC_GetMecSpeedReferenceMotor1
void
int16
返回机械参考速度
MC_GetMecSpeedAverageMotor1
void
int16
返回平均机械速度
MC_GetLastRampFinalSpeedMotor1
void
int16
返回上次指令速度
MC_GetControlModeMotor1
void
STC_Modality_t
返回控制模式
MC_GetImposedDirectionMotor1
void
int16
返回电机转动方向
MC_GetSpeedSensorReliabilityMotor1
void
bool
返回当前速度传感器是否可信
MC_GetPhaseCurrentAmplitudeMotor1
void
Is
返回电流值

## Slide 44

API
函数列表（
2/2
）
138
函数名称
函数参量
函数返回
函数功能
MC_GetPhaseVoltageAmplitudeMotor1
void
Vs
返回电压值
MC_GetIabMotor1
void
Ia，Ib
返回
a
，
b
相电流
MC_GetIalphabetaMotor1
void
I
α，
I
β
返回
clark
变换后的
Iα
，
Iβ
MC_GetIqdMotor1
void
Id，Iq
返回
park
变换后的
Id
，
Iq
MC_GetIqdrefMotor1
void
Idref，Iqref
返回
Id，Iq
参考
MC_GetVqdMotor1
void
Vd，Vq
返回变换电压量
Vd
，
Vq
MC_GetValphabetaMotor1
void
V
α，
V
β
返回变换电压量
Vα
，
Vβ
MC_GetElAngledppMotor1
void
Angle
dpp
返回电角度
DPP
数据
MC_GetTerefMotor1
void
Iqref
返回电流参考
MC_SetIdrefMotor1
Idref
void
设定电流
Id
参考
MC_Clear_IqdrefMotor1
void
void
Iq
，
Id
数据回到默认值
MC_AcknowledgeFaultMotor1
void
b
ool
清除异常状态
MC_GetOccurredFaultsMotor1
void
Fault
得到发生了的
Fault
状态
MC_GetCurrentFaultsMotor1
void
Fault
得到当前的
Fault
状态
MC_GetSTMStateMotor1
void
State
得到电机状态

## Slide 45

电机库在
MC SDK5.x
中的位置
139
涉及底层操作
可以调用在
API
中没有涉及的函数
修改需要熟悉电机运行框架
可以根据实际需求修改对应代码
给电机控制开发者更大发挥空间！！！

## Slide 46

软件架构
14
用户层
电机应用层
电机库
芯片外设库
UI
库
mc_task
mc_api
STM32xxx HAL/LL + CMSIS
PID Regulator
Speed & Position
Circle Limitation
Current Sensing
…
MC_StartMotor1
MC_StopMotor1
MC_GetIabMotor1
…

## Slide 47

电机库底层源文件
140
源文件
描述
bus_voltage_sensor.c
母线电压
circle_limitation.c
电压极限限制
enc_align_ctrl.c
编码器初始定位控制
encoder_speed_pos_fdbk.c
编码器传感器相关
hall_speed_pos_fdbk.c
Hall
传感器相关
mc_math.c
数学计算
motor_power_measurement.c
平均功率计算
ntc_temperature_sensor.c
NTC
温度传感
open_loop.c
开环控制
pid_regulator.c
PID
环路控制
pqd_motor_power_measurement.c
功率计算
pwm_common.c
TIMER
同步使能
pwm_curr_fdbk.c
SVPWM
，
ADC
设定相关接口
r_divider_bus_voltage_sensor.c
实际母线电压采集
virtual_bus_voltage_sensor.c
虚拟母线电压
ramp_ext_mngr.c
无传感开环转闭环控制
speed_pos_fdbk.c
速度传感接口
speed_torq_ctrl.c
速度力矩控制
state_machine.c
电机状态相关
virtual_speed_sensor.c
无传感开环运行相关

## Slide 48

电机控制状态机
141
无传感模式状态机
Encoder
模式状态机

## Slide 49

实际调试

## Slide 50

电机参数测定
143
电流环基于电机参数计算得到
无传感观测是也与电机参数相关
写入正确的参数，是稳定
FOC
控制的第一步
极对数
最大转速
最大电流
额定电压
电机相电阻
电机电感
电机发电常数
电机转动惯量
电机阻力系数

## Slide 51

电机参数测定
—
极对数测试
144
一般情况下电机厂商会提供极对数
需要测试时可以使用电源（给定电压，如
5V
；限流，如
0.5A
），转动电机一圈，应会感觉有阻力，稳定的位置个数即为极对数个数。
也可以使用示波器，旋转一圈对应完整波形个数即为极对数个数

## Slide 52

电机参数测定
—
电阻，电感测试
145
电阻
Rs
可以使用万用表测定，数据除以
2
电桥
f
=
1KHz
，
V=
1V
配置，测试电感
Ls(
或者
Ld
，
Lq
)
，数据除以
2
旋转一圈记录最大，最小电感值
分表贴电机（最大
-
最小
<
平均*
15%
）和嵌入电机（最大
-
最小
>
平均*
15%
）

## Slide 53

无
RLC
设备电感测试
1/2
146
Rs
使用万用表测试
增加
DC
电源电压到电流为
normal
电流
连接示波器，断开
DC
电源一端，快速上电
测试示波器上电流上升波形
上升为
63%
处时间为
t
=
Ld
/
Rs

## Slide 54

无
RLC
设备电感测试
2/2
147
Rs
使用万用表测试
按照第一张图连接电机，增加
DC
电源电压到电流为
normal
电流
不转动电机情况下按照图二连接电机
连接示波器，断开
DC
电源一端，快速上电
测试示波器上电流上升波形
上升为
63%
处时间为
Lq
= t * 2Rs/3

## Slide 55

反电动势测试
148
测量相邻峰谷电压差
测量相邻峰谷频率

## Slide 56

电机控制软件使用前准备
149
写控制软件前需要保证电机控制硬件的正常运作！
几个必要的检测：
电压供电是否正常，高压，驱动电压，单片机电压是否正常
当单片机无程序代码时，消耗电流情况
过电流保护是否正常
电压采集部分是否正常
以上都正常后，可以软件单独输出几路
PWM
控制，看单路
PWM
波形是否正常控制

## Slide 57

电机控制原理

## Slide 58

使用
V/F
开环控制调试硬件
可以用于检测硬件是否正常
只需要在
mc_task.c
中加入几行代码即可
控制合适时电机可开环转动
直接给出
Vd
，
Vq
，以及
θ
角度
无执行部分
150

## Slide 59

无传感使用案例（
1/6
）
151
单片机资源分配
控制策略，开闭环路配置

## Slide 60

无传感使用案例（
2/6
）
152
硬件参数配置
电机参数
驱动配置
电流采集电路参数配置

## Slide 61

无传感使用案例（
3/6
）
153
在
Driver Management
的速度传感选择中选择
Sensor-less

## Slide 62

无传感使用案例（
4/6
）
154
生成代码，代码下载到单片机中运行
可先将电机运行在力矩模式，检测电流环以及空载下电机运行情况
可以先设定
30%
的最大电流的力矩
动态调整，电流探头可做检测
力矩电流数字量
:

## Slide 63

无传感使用案例（
5/6
）
155
力矩环测试过程中
Start-up failure
报错：
电流环
Kp
，
Ki
调整（一般情况下，电机参数正确后，只需要调整环路带宽）
可调整启动曲线（
I/F
曲线），更符合电机特性
可适当增加无感定位时间
力矩环测试过程中
Speed feedback failure
报错：
电流环
Kp
，
Ki
配置
最低切闭环速度是否太低了
速度收敛阈值可相应增大，最大
400%
切入闭环判定条件调整

## Slide 64

无传感使用案例（
6/6
）
156
电流环测试成功后，可以进行速度环调整

## Slide 65

Hall
传感器使用案例（
1/3
）
案例：
使用
Hall
传感器模式使用案例
配置一：在
Motor Sensors
勾选
Hall Sensors
选择
Hall
安装方式
120
度或者
60
度
输入
Hall
相关的同步电角度数据，默认为
300
度
精确的同步电角度测试说明见
https://www.stmcu.com.cn/
上的文档说明

## Slide 66

Hall
传感器使用案例（
2/3
）
配置二：在
Driver Management
的速度传感选择中选择
Hall Sensors
平均速度计算缓冲深度
输入信号滤波时间
158

## Slide 67

Hall
传感器使用案例（
3/3
）
配置三：
Control Stage
中配置正确的
Hall
信号引脚接口
注意
:
电机的
Hall
信号（
H1,H2,H3
）要与三相电机输出控制线
OUT1
，
2
，
3
对应起来，
否则电机堵转或者过流
对于电机
Shinano
LA052-080E3NL1
，对应关系如下：
X-Nucleo-IHM07M1
Motor: Shinano LA052-080E3NL1
Note
OUT1
BLACK
W1
OUT2
WHITE
W2
OUT3
RED
W3
A.
ORANGE
HALL1
B.
GREEN
HALL2
Z.
YELLOW
HALL3
5V
BROWN
5V
GND
BLUE
GND
159

## Slide 68

三相永磁同步电动机
(
直流无刷电动机
)
16
永磁同步电动机
(
直流无刷电动机）
内转子
/
外转子
集中绕组
/
分布绕组
表面贴装磁石或内嵌式磁石
转子
绕组
(
定子
)
表贴磁石
内嵌磁石

## Slide 69

Encoder
使用案例（
1/4
）
配置一：在
Motor Sensors
勾选
Quadrature encoder
填入正确的
Encoder
线数参数
这边是旋转一圈编码器的脉冲个数
对于
Shinano
LA052-080E3NL1
，该参数为
400
旋转一圈的编码器单向脉冲个数
160

## Slide 70

Encoder
使用案例（
2/4
）
配置二：在
Driver Management
的速度传感选择中选择
Quadrature encoder
平均速度计算缓冲深度
输入信号滤波时间
161

## Slide 71

Encoder
使用案例（
3/4
）
配置三：在
Driver Management
的
Start-up
参数上根据电机调整定位参数
因为是增量编码器，需要将转子定位到知道角度的位置才可以正常启动运行
定位时间以及定位的电流可根据具体电机进行调整
转子定位时间
定位角度
定位电流
162

## Slide 72

Encoder
使用案例（
4/4
）
配置四：
Control Stage
中配置正确的
Encoder
信号引脚接口
注意
:
电机的
Encoder
信号（
EA
，
EB
）要与三相电机输出控制线
OUT1
，
2
，
3
对应起来，
否则电机堵转或者过流
对于电机
Shinano
LA052-080E3NL1
，对应关系如下：
X-Nucleo-IHM07M1
Motor: Shinano LA052-080E3NL1
Note
OUT1
BLACK
W1
OUT2
WHITE
W2
OUT3
RED
W3
A.
BLUE
EA
B.
YELLOW
EB
Z.
5V
RED
5V
GND
BLACK
GND
163

## Slide 73

资源

## Slide 74

培训资料
165
www.stmcu.com.cn

## Slide 75

AI
电堂
(We-Chat)
166
STM32
电动机应用系列课程
课程
QQ
群
课程答疑

## Slide 76

ST
官网关于
FOC
167
www.st.com
评估板，技术文档，设计笔记等

## Slide 77

试验环节（
Hands On
）

## Slide 78

试验项目
169
试验一：无位置传感器速度模式控制
试验二：旋钮控制电机运行速度
试验三：测试电机库高频运行时间以及
Fault
状态清除

## Slide 79

永磁同步电机矢量控制原理
(1/2)
17
电流产生的磁场方向
永磁力矩
Permanent magnetic torque
磁阻力矩
Reluctance torque

## Slide 80

试验一：无位置传感器速度模式控制
170
试验目标
MC-SDK V5.4
基本操作
实现电机无传感运行
体会开环启动到闭环运行
使用
API
实现电机变速运行，
300RPM
600RPM

## Slide 81

试验一
171
Workbench
创建工程

## Slide 82

试验一
172
生成电机工程

## Slide 83

试验一
173
调用
API
，实现电机变速运行
MC_ProgramSpeedRampMotor1()
MC_StartMotor1()
MC_StopMotor1()

## Slide 84

试验一
174
可通过示波器功能查看运行

## Slide 85

试验二：旋钮控制电机运行速度
175
试验目标
使用
CubeMx
修改已有电机工程
ADC
在电机工程中的使用
变速调整

## Slide 86

试验二
176
在第一个试验基础上修改
CubeMx
工程
添加
PC2
作为
ADC1_Channle8
重新生成电机工程

## Slide 87

试验二
177
包含头文件，以及
ADC
采样相关变量
初始化用户
ADC

## Slide 88

试验二
178
添加具体实现程序，用到函数
RCM_GetUserConvState
RCM_RequestUserConv
RCM_GetUserConv

## Slide 89

试验二
179
Workbench
中可以观测效果

## Slide 90

永磁同步电机矢量控制原理
(2/2)
A
X
B
C
Y
Z
18

## Slide 91

试验三：测试电机库
FOC
运行时间以及
Fault
状态清除
180
试验目标
使用
CubeMx
生成
Timer
初始化
了解高频任务的实际调用的地方
掌握如何得到电机状态
掌握如何清除报错状态，让电机重新运行

## Slide 92

试验三
181
为测试程序运行时间，配置
TIM3
最大溢出时间设定为
65536/170MHz = 385.5us

## Slide 93

试验三
182
在
ADC
中断中加入执行速度检测代码
函数在
stm32g4xx_mc_it.c
中

## Slide 94

试验三
183
实时查看电机运行状态
当发生
Fault
的时候进行清除，清除后运行
用到函数
MC_GetSTMStateMotor1
MC_GetOccurredFaultsMotor1
MC_AcknowledgeFaultMotor1

## Slide 95

试验三
184
通过
Live watch
查看
FOC
高频运行时间
时间
19us
通过
Live watch
查看电机运行状态

## Slide 96

试验三
185
Fault
自动清除
Workbench
可查看运行状态，
Fault
自动清除

## Slide 97

Thank You
© STMicroelectronics - December 2019 - All rights reserved.
The STMicroelectronics corporate logo is a registered trademark of the STMicroelectronics
group of companies. All other names are the property of their respective owners
.

## Slide 98

永磁同步电机矢量控制原理
19
矢量控制的基本思想是将交流电动机等效为他励直流电动机，转矩和励磁分别做独立的控制。
定子电流被分解成
：
直轴电流Id：励磁电流
交轴电流Iq：转矩电流
直轴：永磁体磁场方向
交轴：与直轴正交
I
d
I
q
I
B
F
S
N
d
q
A
B
C
θ
q
β
α
ω
e

## Slide 99

总体概况
FOC
控制原理
硬件整体介绍
MC SDK5.x
软件介绍
实际调试步骤简介
试验环节（
Hands On
）
2
培训内容

## Slide 100

矢量变换公式以及示意图
Clarke
Park
Rev
Park
20

## Slide 101

矢量控制
—
控制器
21
K
P
K
I
/
s
Plant
+
-
S.V.
P.V.
ε
+
+
Noise
C.V.
+
-
控制器
设定值
S.V.
实际值
P.V.
控制值
C.V.
对象传递函数
速度控制器
f
r
-ref
(
f
r
*
)
f
r
-fb
(
f
r
)
τ
ref
(
τ
*
) or
I
q
-ref
(
I
q
*
)
电流控制器（忽略
dq
之间的耦合）
I
d/q-ref
(
I
d/q
*
)
I
d/q-fb
(
I
d/q
)
U
d
/q-ref
(
U
d
/q
*
)
控制器
设定值
S.V.
实际值
P.V.
控制值
C.V.
对象传递函数
速度控制器
f
r
-ref
(
f
r
*
)
f
r
-fb
(
f
r
)
τ
ref
(
τ
*
) or
I
q
-ref
(
I
q
*
)
电流控制器（忽略
dq
之间的耦合）
I
d/q-ref
(
I
d/q
*
)
I
d/q-fb
(
I
d/q
)
U
d
/q-ref
(
U
d
/q
*
)

## Slide 102

电流调节器
--
拉普拉斯域设定
22
9
K
P-ACR
K
I-ACR
/
s
+
-
I
d/q
*
ε
+
+
U
d
/q
*
+
I
d/q
电流调节器的开环增益为

## Slide 103

电流调节器
--ST MC SDK5.x WB
设定
23
9
变为一阶系统
令
KP/KI = LS/RS

## Slide 104

速度调节器
24
K
P-ASR
K
I-ASR
/
s
+
-
f
r
*
f
r
ε
+
+
τ
L
τ
e
+
-
f
r
J
:
转动惯量
[kg/m
2
];
F
:
阻力系数
[Nm/[rad/s]];
ω
B-ASR
:
速度调节器通带宽度
[rad/s];
K
P-ASR
:
速度调节器比例系数
;
K
I-ASR
:
速度调节器积分系数。
拉普拉斯域
PI
系数设定方法
WB
中
PI
系数的设定

## Slide 105

矢量控制
—
SVPWM(1/3)
25
uector
Q
1
(
Q
4
)
Q
3
(
Q
6
)
Q
5
(
Q
2
)
U
AN
[
u
]
U
BN
[
u
]
U
CN
[
u
]
U
AO
[
u
]
U
BO
[
u
]
U
CO
[
u
]
OFF(ON)
OFF(ON)
OFF(ON)
0
0
0
0
0
0
ON(OFF)
OFF(ON)
OFF(ON)
U
bus
0
0
2U
bus
/3
-
U
bus
/3
-
U
bus
/3
ON(OFF)
ON(OFF)
OFF(ON)
U
bus
U
bus
0
U
bus
/3
U
bus
/3
-
2U
bus
/3
OFF(ON)
ON(OFF)
OFF(ON)
0
U
bus
0
-
U
bus
/3
2U
bus
/3
-
U
bus
/3
OFF(ON)
ON(OFF)
ON(OFF)
0
U
bus
U
bus
-2U
bus
/3
U
bus
/3
U
bus
/3
OFF(ON)
OFF(ON)
ON(OFF)
0
0
U
bus
-
U
bus
/3
-
U
bus
/3
2U
bus
/3
ON(OFF)
OFF(ON)
ON(OFF)
U
bus
0
U
bus
U
bus
/3
-2U
bus
/3
-
U
bus
/3
ON(OFF)
ON(OFF)
ON(OFF)
U
bus
U
bus
U
bus
0
0
0
uector
Q
1
(
Q
4
)
Q
3
(
Q
6
)
Q
5
(
Q
2
)
U
AN
[
u
]
U
BN
[
u
]
U
CN
[
u
]
U
AO
[
u
]
U
BO
[
u
]
U
CO
[
u
]
OFF(ON)
OFF(ON)
OFF(ON)
0
0
0
0
0
0
ON(OFF)
OFF(ON)
OFF(ON)
U
bus
0
0
2U
bus
/3
-
U
bus
/3
-
U
bus
/3
ON(OFF)
ON(OFF)
OFF(ON)
U
bus
U
bus
0
U
bus
/3
U
bus
/3
-
2U
bus
/3
OFF(ON)
ON(OFF)
OFF(ON)
0
U
bus
0
-
U
bus
/3
2U
bus
/3
-
U
bus
/3
OFF(ON)
ON(OFF)
ON(OFF)
0
U
bus
U
bus
-
2U
bus
/3
U
bus
/3
U
bus
/3
OFF(ON)
OFF(ON)
ON(OFF)
0
0
U
bus
-
U
bus
/3
-
U
bus
/3
2U
bus
/3
ON(OFF)
OFF(ON)
ON(OFF)
U
bus
0
U
bus
U
bus
/3
-
2U
bus
/3
-
U
bus
/3
ON(OFF)
ON(OFF)
ON(OFF)
U
bus
U
bus
U
bus
0
0
0

## Slide 106

矢量控制
—
SVPWM(2/3)
26
60
°
θ
u
n
u
k
u
β
60
°
θ
u
n
u
k
II
III
IV
V
VI
I
Sector
I
II
III
IV
V
VI
n
1
3
3
5
5
1
k
2
2
4
4
6
6

## Slide 107

矢量控制
—
SVPWM(3/3)
27
sector
tA
tB
tC
I
T/2 + (X – Z )/2
tA
+ Z
tB
– X
II
T/2 + (Y – Z )/2
tA
+ Z
tA
– Y
III
T/2 + (Y – X )/2
tC
+ X
tA
– Y
IV
T/2 + (X – Z )/2
tA
+ Z
tB
– X
V
T/2 + (Y – Z )/2
tA
+ Z
tA
– Y
VI
T/2 + (Y – X )/2
tC
+ X
tA
– Y

## Slide 108

每安培最大转矩
—MTPA
（
1/2
）
28
上式为电磁转矩方程，当表贴电机
Ld
=
Lq
时是我们熟悉的
d
轴电流参考为
0
的控制方式
当使用内嵌电机时
Ld
<
Lq
，如果需要力矩最大的话，
Id
需要小于
0
Iq
输出由速度环得到，电流值
Is
可做为已知量，
MTPA
实现即求解
对
Id
的极小值，即
给出的电流值
Is
符合下面关系式：
+

## Slide 109

每安培最大转矩
—MTPA
（
2/2
）
29
Is
对应于最大转矩
Te
，求解方程，舍去正根后得到
π
/2

## Slide 110

总体概况

## Slide 111

弱磁控制
30
很多应用需要马达工作在高于额定速度的范围内，这里就需要弱磁控制来实现
电流的电压约束条件
电流幅值约束条件

## Slide 112

矢量控制
—
MTPA
与弱磁控制
31
Increasing speed
Increasing torque
MTPA:
弱磁
:

## Slide 113

矢量控制
—
电流前馈
32
∆
L
q
2
π
p
L
d
K
K
I
d
*
I
q
*
f
r
+
+
-1
+
+
∆
U
q
∆
U
d
U
q2
*
U
d2
*
U
q
*
U
d
*
k
E

## Slide 114

电流采样
— ICS
33
A
B
A
X
B
C
Y
Z
ACCT
DCCT
频率范围
>0
Hz ~ tens kHz
DC
~ 100kHz
退磁
Need
Need not
成本
low
High

## Slide 115

电流采样
—
三电阻
(1/2)
34
A
B
A
X
B
C
Y
Z
A
X
B
Y
C
Z
C

## Slide 116

电流采样
—
三电阻
(2/2)
35
CCRmax
CCRmid
CCRmin
∆
2
∆
1
∆
1
= 2*(
ARR
–
CCRmax
– DTG)
∆
2
=
CCRmax
–
CCRmid
ARR
0
DTG
情况
条件
采样点
1
∆
1
>max( 2*(
CNT_Ton+CNT_Trise+CNT_Ring+tdead
/2)
,
CNT_TADCsta+CNT_TADCSH
(COV)-
tdead
/2)
Middle of PWM
2
∆
1
>
∆
0
CCRmax
+
tdead+ton+tring
+
ε
3
∆
2
>
∆
0
>
∆
1
CCRmid
+
tdead+ton+tring
+
ε
4
∆
1
<
∆
0
and
∆
2
<
∆
0
Not available
case3
case4
case1
∆
0
=
CNT_Ton
+
CNT_
Tring
+CNT_TADCSH
(COV) ,
Tring >
TADCsta
∆
0
=
CNT_Ton
+
CNT_TADCsta+CNT_TADCSH
(COV),
TADCsta
>=Tring
case2
∆
0
∆
0
∆
0

## Slide 117

电流采样
—
单电阻
(1/8)
36
A
B
C
A
X
B
C
Y
Z
A
X
B
Y
C
Z

## Slide 118

电流采样
—
单电阻
(1/8)
37
A
B
C
A
X
B
C
Y
Z
A
X
B
Y
C
Z

## Slide 119

电流采样
—
单电阻
(2/8)
38
Vector
A
(
X
)
B
(
Y
)
C
(
Z
)
Ishunt
OFF(ON)
OFF(ON)
OFF(ON)
0
ON(OFF)
OFF(ON)
OFF(ON)
i
A
ON(OFF)
ON(OFF)
OFF(ON)
-
i
C
OFF(ON)
ON(OFF)
OFF(ON)
i
B
OFF(ON)
ON(OFF)
ON(OFF)
-
i
A
OFF(ON)
OFF(ON)
ON(OFF)
i
C
ON(OFF)
OFF(ON)
ON(OFF)
-
i
B
ON(OFF)
ON(OFF)
ON(OFF)
0
Vector
A
(
X
)
B
(
Y
)
C
(
Z
)
Ishunt
OFF(ON)
OFF(ON)
OFF(ON)
0
ON(OFF)
OFF(ON)
OFF(ON)
i
A
ON(OFF)
ON(OFF)
OFF(ON)
-
i
C
OFF(ON)
ON(OFF)
OFF(ON)
i
B
OFF(ON)
ON(OFF)
ON(OFF)
-
i
A
OFF(ON)
OFF(ON)
ON(OFF)
i
C
ON(OFF)
OFF(ON)
ON(OFF)
-
i
B
ON(OFF)
ON(OFF)
ON(OFF)
0
A
B
C
A
X
B
C
Y
Z
I
shunt
II
III
IV
V
VI
I

## Slide 120

电流采样
—
单电阻
(3/8)
39
CCRmax
CCRmid
CCRmin
MAX_HI
MAX_LO
MID_HI
MID_LO
MIN_HI
MIN_LO
if
T
ADCtrigger
delay
> T
ring
,
needless to add Tring available current signal detection pulse width is:
Min_pulsewidth
=
T
dead
+ T
on
+
T
ADCtrigger
delay
+ T
ADC s/h
else, available current signal detection pulse width is:
Min_pulsewidth
=
T
dead
+ T
on
+ T
ring
+T
ADC s/h
ADC trigger point:
Point 1)
CCRmid
-
T
ADCtrigger
delay
-T
ADC s/h
-
Point 2)
if
T
ADCtrigger
delay
> T
ring
CCRmid
+
T
dead
+ T
on
+ T
ring
-
T
ADCtrigger
delay
Else
CCRmid
+
T
dead
+ T
on
+
T
ADCtrigger
delay

## Slide 121

MC SDK 5.x
控制的电机种类
4
永磁同步电机
PMSM
直流无刷电机
BLDC
六步方波控制
FOC
矢量控制
MC SDK 5.4

## Slide 122

电流采样
—
单电阻
(4/8)
40
T
0
/ 2
T
1
/ 2
T
2
/ 2
T
7
T
2
/ 2
T
1
/ 2
T
0
/ 2
T
0
/ 2
T
1
/ 2
T
2
/ 2
T
7
T
2
/ 2
T
1
/ 2
T
0
/ 2
T
0
/ 2
T
1
/ 2
T
2
/ 2
T
7
T
2
/ 2
T
1
/ 2
T
0
/ 2

## Slide 123

电流采样
—
单电阻
(5/8)
41
ST
专利
(Pat. Pub. No.: US20090284194 A1)
解决单电阻无法采样问题
T
0
/ 2
T
1
/ 2
T
2
/ 2
T
7
T
2
/ 2
T
1
/ 2
T
0
/ 2
T
0
/ 2
T
1
/ 2
T
2
/ 2
T
7
T
2
/ 2
T
1
/ 2
T
0
/ 2
T
7
T
MIN
ST
专利
US20090284194 A1

## Slide 124

电流采样
—
单电阻
(6/8)
42
ST
专利
(Pat. Pub. No.: US20090284194 A1)
解决单电阻无法采样问题
T
0
/ 2
T
1
/ 2
T
2
/ 2
T
7
T
2
/ 2
T
1
/ 2
T
0
/ 2
T
0
/ 2
T
1
/ 2
T
2
/ 2
T
7
T
2
/ 2
T
1
/ 2
T
0
/ 2
T
7
T
MIN
ST
专利
US20090284194 A1

## Slide 125

电流采样
—
单电阻
(7/8)
43
ST
专利
(Pat. Pub. No.: US20090284194 A1)
解决单电阻无法采样问题
T
0
/ 2
T
1
/ 2
T
2
/ 2
T
7
T
2
/ 2
T
1
/ 2
T
0
/ 2
T
0
/ 2
T
1
/ 2
T
7
T
1
/ 2
T
0
/ 2
T
7
T
MIN
ST
专利
US20090284194 A1

## Slide 126

电流采样
—
单电阻
(8/8)
44
ST
专利
(Pat. Pub. No.: US20090284194 A1)
解决单电阻无法采样问题
T
0
/ 2
T
1
/ 2
T
2
/ 2
T
7
T
2
/ 2
T
1
/ 2
T
0
/ 2
T
0
/ 2
T
1
/ 2
T
2
/ 2
T
7
T
2
/ 2
T
1
/ 2
T
0
/ 2
T
0
/ 2
T
1
/ 2
T
7
T
1
/ 2
T
0
/ 2
T
7
T
MIN
T
MIN
T
7
ST
专利
US20090284194 A1

## Slide 127

位置速度检测
—
Hall
传感器
45
对于
60
度的
Hall
信号，可以任意调换三个信号中的任意一个即可得到和
120
度的处理相似，我们可以很方便使用软件处理。

## Slide 128

位置速度检测
—
Encoder
46
使用增量编码器时，在第一次电机启动，任意保护停止或者
MCU
复位后都要进行预定位操作。
Z
信号（一圈一个）可以使用外部中断或者外部
Timer
捕捉模式，代表编码器的
0
度位置，可以用于校准角度位置，可以使用
DMA
模式对编码器模块赋值；

## Slide 129

位置速度检测
—
观测器
(1/1
3
)
47
u
1
u
2
u
p
…..
…..
x
1
, x
2
,……,
x
n
…..
y
1
y
2
y
m
u
α
u
β
i
α
i
β
u
A,B,C
i
A,B,C
e
A,B,C
r
L,M
A,B,C
L,M
α
,
β
u
A,B,C
i
A,B,C
e
A,B,C
O
u
α
,
β
i
αβ
e
α
,
β
u
α
,
β
i
αβ
e
α
,
β
e
α
,e
β
,
i
α
,
i
β
u
->
i
(
τ
e
) ->
ω
r
,
ω
e
=p∙
ω
r
- > e

## Slide 130

位置速度检测
—
观测器
(2/1
3
)
48
u
α
u
β
i
α
i
β
r
L
s
u
α
,
β
i
αβ
e
α
,
β
e
α
,e
β
,
i
α
,
i
β
S
N
q
d
β
α
θ
d
ω
e
k
E
A linear model

## Slide 131

位置速度检测
—
观测器
(3/1
3
)
49
B
∫
A
C
+
+

## Slide 132

BLDC/PMSM
电机热门应用
扫地机器人
吸尘器
冰箱压缩机
变频空调
变频洗衣机
空气净化器
排气扇
落地扇
风筒
水泵
抽油烟机
电动工具
机器人
伺服
变频器
体育器材
个人护理
5

## Slide 133

位置速度检测
—
观测器
(4/1
3
)
50
B
∫
A
C
+
+
∫
A
C
+
+
+
-
H
-

## Slide 134

位置速度检测
—
观测器
(5/1
3
)
51
离散化

## Slide 135

位置速度检测
—
观测器
(6/1
3
)
52
去耦
(
认为
ω
e
=0)
将简化马达模型
特征值

## Slide 136

位置速度检测
—
观测器
(7/1
3
)
53

## Slide 137

位置速度检测
—
观测器
(8/1
3
)
54
去耦
(
设定
ω
e
=0)
简化观测器模型

## Slide 138

位置速度检测
—
观测器
(
9
/1
3
)
55
e=
k
E
ω
e
=√e
α
^
2
+ e
β
^
2
e∙
sin
(
θ
q
^
-
θ
q
)
=e
β
cos(
θ
q
^
) -
e
α
sin(
θ
q
^
)
K
P-PLL
K
I-PLL
/s
1/s
+
-
+
+
θ
q
*
θ
q
^
ω
e
^
θ
q
^
-
θ
q
≈ (e
β
cos(
θ
q
^
) -
e
α
sin(
θ
q
^
))/(
k
E
ω
e
)
如果
θ
q
^
-
θ
q
比较小
e
β
^
e
α
^
φ
q
φ
q
^
q
^
θ
q
^
θ
q
q

## Slide 139

56
e
β
^
e
α
^
φ
q
φ
q
^
e=√e
α
^
2
+ e
β
^
2
=
k
E
ω
e
*
q
^
θ
q
^
θ
q
e
β
^
cos(
θ
q
^
) –
e
α
^
sin(
θ
q
^
)
K
P-PLL
K
I-PLL
/s
1/s
+
-
+
+
θ
q
*
θ
q
^
ω
e
^
1/
(32767
k
E
ω
e
*
)
K
P-PLL
K
I-PLL
/s
+
+
ω
e
^
65536/
(
2
π
f
c
)
ω
e[
dpp
]
^
位置速度检测
—
观测器
(
10
/
13
)
q

## Slide 140

位置速度检测
—
观测器
(
11
/
13
)
57
+
+
+
+
+
-
-
-
Motor Model

## Slide 141

位置速度检测
—
观测器
(1
2
/
13
)
58
+
+
-
-
-
+
+
+
+
+
-
+
+
+
+
-
+
+
+
OBSERVER
PLL

## Slide 142

位置速度检测
—
观测器
(1
3
/1
3
)
59

## Slide 143

ST MC SDK
发展路线图
6
2002
2005
2008
2009
2011
2013
2014
2016
2018
ST9
ST7FMC
ACIM with dedicated
library
6-step
control
SW kit
FOC
library
v2.0
MC kit
6-step control
FOC SDK
v3.0
FOC SDK
v3.4
FOC SDK
v4.0
FOC SDK
v4.3
FOC SDK
v5.
x

## Slide 144

硬件篇

## Slide 145

MCU
资源选择
STM32
资源评估
频率需求？
ADC
需要几个？
互补输出的
Timer
需求几个？
Hall
或者
Encoder
接口需求？
其他
I/O
口需求？
61
STM32F0
STM32F1
STM32F3
STM32F4
STM32F7
STM32L4
STM32G0
STM32G4

## Slide 146

MCU
资源
—
电机
Timer
查看预驱部分，是否需要互补输出的
3
对
PWM
（
6
路）
根据后级
MOS/IGBT
进行死区时间的配置
62

## Slide 147

电流采集部分
63

## Slide 148

传感器连接
--STM32
硬件
Encoder
接口
64
在全系列
STM32
中都有硬件增量编码器
Encoder
接口
每个正交沿都可有加
/
减计数
CubeMx
中的配置

## Slide 149

传感器连接
--STM32
硬件
Hall
接口
65
在全系列
STM32
中都有硬件
Hall
接口
(XOR
输入
)
可以每个
Hall
跳变沿都产生中断
CubeMx
中的配置

## Slide 150

ST Demo
板传感电路问题
当
Hall
或者
Encoder
传感器内部无上拉电阻
传感器波形将是个灾难！
此问题存在于很多型号的
Demo
上
比如
IHM07M1
，
IHM08M1
等等
改
0
欧姆电阻为
10K
欧姆
66

## Slide 151

驱动部分
67
OFF
ON
建议的驱动电路

## Slide 152

母线电压采集
68
电压分压电阻配置需要保证
Vbus
全范围内可以采集
注意根据外部电路配置
ADC
采样的时间（
Cycle
）
必要时，尤其高压电路需要增加二极管保护电路

## Slide 153

Break
电路设计
69
总电流保护
使用比较器输出连接到
TIM
的
BKIN
管脚
比较器输出接滤波以及上拉电阻

## Slide 154

MC SDK5.x
位置
7
STM32 MC SDK5.x
软件层
硬件层
应用层
电机控制程序
7

## Slide 155

反充电电路设计
70
在高压情况下，若有弱磁运行，则必须使用该电路
！
用于过电压保护
能量消耗到功率电阻上

## Slide 156

软件篇

## Slide 157

72
请预先安装下列
PC
软件工具：
X-CUBE-MCSDK
或
X-CUBE-MCSDK-FUL
STM32CubeM
X
(
v5
.3
以上
)
及
固件库
ST-LINK/V2
或者
ST-LINK/V3
IDE:
IAR Embedded Workbench for Arm (v8.
x
)
μ
Vision® IDE for Arm® (
Keil
® MDK) v5.
x
Atollic
TrueSTUDIO
for STM32 version 9.0.0
STM32CubeMX
请不要安装在中文路径下！！
软件工具安装

## Slide 158

73
下列方式可启动
ST MC Workbench
软件工具：
单击其图标
从安装文件夹路径直接启动
软件启动

## Slide 159

74
用户按钮区用于创建新项目，加载已有项目或启动
ST
电机参数测量工具。
最近的项目区用于加载近期的项目。
例程区用于加载项目示例
。
1
2
3
Workbench

## Slide 160

75
创建新工程

## Slide 161

76
图标和菜单区
硬件信息
用户信息
主要的硬件配置
硬件细节设定按钮区
硬件配置窗口

## Slide 162

77
New Project:
创建一个新项目
Open Project…:
打开一个现有的项目
Close Project:
关闭现在的项目
Save Project:
使用相同文件名保存打开的项目
Save Project As…:
将打开的项目保存为指定的文件名
Properties:
查看项目属性
Recent List:
从最近打开过的项目列表中加载一个现有项目
Recent List Delete:
删除最近的项目列表
Exit:
从硬件配置窗口退出
Workbench
菜单
(1)

## Slide 163

78
Pin assignment:
检查
MCU
的引脚分配和剩余的可用引脚
Generation:
根据所选的
IDE
，生成
MC
应用工程文件
Monitor:
监控并转动电机
Clear Log:
清除用户信息表
Export Log:
将用户信息表以文本格式导出到日志文件
Restore Info Message:
需要时显示用户信息表
Workbench
菜单
(2)

## Slide 164

79
Workbench
菜单
(3)

## Slide 165

STM32 MC SDK5.x
特点
8
结合
CubeMx
自动生成电机控制代码
支持有传感
/
无传感
完整的
FOC
控制代码
电机参数自动识别
电机实时交互调试
STM32F0
STM32F1
STM32F3
STM32F4
STM32F7
STM32L4
STM32G0
STM32G4
STM32 MC SDK5.4

## Slide 166

80
New
：创建一个新项目
Load
：打开一个现有的项目
Save
：使用相同文件名保存打开的项目
Clear
Log
：清除用户信息表
Pin assignment
：
检查
MCU
的引脚分配和剩余的可用引脚
Generation:
根据所选的
IDE
，生成
MC
应用工程文件
.
Click to open monitor:
监控并转动电机
Help:
提供在线帮助文件的访问接口
About:
显示该应用的版本号
工具栏图标

## Slide 167

81
根据客户需求快速设置电机库
电机参数
硬件驱动
驱动控制管理
单片机相关
硬件细节设定按钮区

## Slide 168

82
电机参数

## Slide 169

马达参数配置
极对数
最大转速
最大电流
额定电压
电机相电阻
电机电感
电机发电常数
电机转动惯量
电机阻力系数
电机
d
轴电感
电机
q
轴电感
d
轴电感和
q
轴电感比率
电机
83

## Slide 170

84
传感器
Hall
同步电角度：
电机
Hall A
的上升沿到电机
A
相反电动势最高点的延迟角度。
默认电机
A
相的反电动势最高点作为电角度的
0
度；
编码器参数
Resolution
：分辨率，单位：
Pulse/Round
旋转一圈的编码器脉冲个数

## Slide 171

85
电源输入信息

## Slide 172

86
直流母线电压采样拓扑

## Slide 173

87
浪涌电流限制器和耗散制动

## Slide 174

88
功率因数校正

## Slide 175

89
温度检测

## Slide 176

开环运行
Ke
参数测试
无传感器启动设定
切换闭环
闭环运行
动态系数测试
惯性，阻尼系数测试
速度校准
Motor Profiler
9
10 sec
5 sec
45 sec
电机静止
Rs
测量
Ls
测量
电流校准运行
一分钟电机矢量控制运行！

## Slide 177

90
功率驱动

## Slide 178

91
电流采样

## Slide 179

92
电流采样拓扑结构选择：
电流采样增益计算器

## Slide 180

93
过流保护

## Slide 181

94
驱动控制管理

## Slide 182

95
主传感器
(1)
可选择无传感，
Hall
传感，
Encoder
传感

## Slide 183

96
主传感器
(2)
BEMF
收敛：
一般用于堵转判断
设定为
100%
时，忽略此判断
根据参数计算得到的系数，一般情况下无需修改，保持为默认数据

## Slide 184

97
辅助传感器

## Slide 185

98
启动参数
顺逆风启动
检测使能
折线或曲线
启动选择
切入闭环条件判定
启动曲线配置，包含速度，电流，时间
启动曲线
配置无传感器开环启动参数
有传感器不用配置

## Slide 186

99
驱动设置
PWM
载波频率
速度环
Kp
& Ki
电流环
Kp
& Ki
