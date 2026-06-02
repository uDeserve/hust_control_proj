# X-CUBE-MCSDK v6.0 课件

- Source: `/mnt/e/BaiduNetdiskDownload/控制课程设计/华科电机课件/新功能，新体验——X-CUBE-MCSDK（v6.0）.pptx`
- Note: 虽然版本偏新，但可用于理解 ST 电机控制软件架构演进。

## Extracted Text
## Slide 1

新功能，新体验
——
X-CUBE-MCSDK
陈旭恩
Shawn CHEN
意法半导体
中国区微控制器部门

## Slide 2

自X-CUBE-MCSDK5.4.4以来的更新信息
10
5.4.4
5.4.5
5.4.6
5.4.7
5.Y.0
5.Y.1
5.Y.2
结束了对Web
发布的各版本
I
AR EWARM 7.x的支持。
新增了对STM32G4 Cut 2.2的支持
更新了Workbench，以适用于STM32CubeMx v6.0.1
新增了对新电源板的支持
修复一些问题
增加了对STM32CubeMx版本6.2.0的支持
修复一些问题
新增了对STM32CubeMx版本6.3.0的支持
修复一些问题
5.Y.3
5.4.
8
5.Y.
4
新增了对STM32CubeMx版本6.
4
.0的支持
修复一些问题
6.0

## Slide 3

X-CUBE-MCSDK 5.Y(1/3)
11
5.4.6
5.4.7
5.Y.0
引入了ST Motor
Pilot版本，这是X-CUBE
-
MCSDK新的监控工具
。
从Workbench删除了旧的监控器
已实施全新的通信协议，并替代了旧版本
。
对于PMSM/BLDC电机：
非连续PWM（即两相调制）
可通过Workbench中的固件驱动管理/附加功能选项卡激活过调制
支持相移的单电阻
。
Circle Limitation VD:
Circle limitation算法的更好变体
新增了对STSPIN32G4器件的支持
新增了对新电路板的支持
新增了对EVALSTDRIVE101电源板的支持
新增了对STM32L452和STM32L476器件的HSI时钟源的支持。
删除了对STM32F1器件的支持（相应地不再支持使用F1的PFC）
六步示例改进
对于异步电机
：
作为采用FOC无传感器和V/f（标量）模式的两个示例，支持ACIM电机。图形化PC工具随SDK一起提供，以帮助配置此示例：ACIM GUI。两个示例专为NUCLEO-G431RB + STEVAL-IHM023V3配置而设计。

## Slide 4

X-CUBE-MCSDK 5.Y(2/3)
12
5.4.6
5.4.7
5.Y.0
5.Y.1
5.Y.2
修复一些问题
修复一些问题
STM32CubeM
X
中的参数可见性，以使六步算法可配置
5.Y.3
重新引入了在5.Y.2中消失的pdf文档。
Motor Pilot改进
5.Y.
4
重新设计和简化了电机运行的状态机
符合
MISRAC2012
规范
增加了
CPU load
测量机制
更改了一些示例文件的名称
在马达控制
API
中添加了浮点
API
添加了得到辅助传感器的速度和位置反馈信息的
API
函数
删除了编码器对齐的
API
添加了一个偏移量测量
API
程序及函数
对于使用霍尔传感器作为速度和位置反馈的配置，可以使能非静止启动功能。

## Slide 5

X-CUBE-MCSDK 5.Y(3/3)
13

## Slide 6

X-CUBE-MCSDK 6.0
14
5.Y.
4
6.0
全新的
Workbench
6
步换相现在可以在
Workbench
里进行配置
目前只支持单电机控制
当前版本只支持
G0
和
G4,
后续版本会更新对
F0,F3,F4,F7,L4
等系列
MCU
的支持

## Slide 7

电机控制 – SDK路线图概览
SDK
产品支持
FW解决方案
（
在SDK中
）
SW工具
最新的主要特性
5.Y.4
2022年1月18日
Q1 22
更晚
SDK V5.4
SDK V6.0
新的Workbench v2.0
位置控制
6步V1
（STM32G4，STSPIN32F0）
双驱动器G4
新Motor Pilot
（变量监控）
新的MC工作台
+
新的固件库架构
：
FOC和
6-STEP
STM32
G4
STM32
H7
新的FOC无传感器算法
SDK 6.1
B类
(G4)
Q2 22
SDK 6.x
SwR
Motor
新的MC Profiler v2.0
（MC FW
内
）
SDK V5.Y
ACIM
(G4)
固件示例
（外部SDK）
非连续
PWM
单电阻
–
相移
过调制
3x MC-FOC
(G4)
产品支持
核心产品
高性价比
高性能
低功耗
STM32F3
STM32G4
STSPIN32
STM32H7
STM32F4/
F7
STM32L4
STM32F1
STM32G0
STM32F0
6步V2
ACIM
3x MC-FOC
其他SW
IP
STSPIN32G4
15

## Slide 8

X-CUBE-MCSDK
演示

## Slide 9

STM32 MC Motor Pilot
17
通过串行端口连接至基于UI模块的电机控制应用。
允许控制、监控和调
试
电机控制应用
。
将替代STM32 MC工作台的监控器部分。
STM32电机控制应用的监控工具

## Slide 10

增强型绘图功能：用户现在可以绘制大多数寄存器
用户可轻松定制GUI
可满足特定需求或试用新的固件功能
为支持将来的固件功能奠定了坚实的基础
ACIM、6步、传感器零速度，增强型调试功能
附加价值
18
在全部三大MCD目标平台上运行：Windows、Mac和Linux

## Slide 11

多种Motor Pilot应用
19

## Slide 12

Agenda
1
用于电机控制的STM32
2
X-CUBE-
MCSDK更新
3
X-CUBE-
MCSDK演示
4
X-CUBE-
MCSDK的
要
点
5
Q&A
2

## Slide 13

快速启动
20

## Slide 14

控制电机
21
显示嵌入式固件的版本
显示当前状态和错误
使电机旋转
显示速度、电流和其他信息
拖
动旋钮，以设置速度
设置目标速度、持续时间，并点击应用斜坡
点击停止以使电机停止旋转
点击停止斜坡
，
以
在结束前使其停止
故障
清除

## Slide 15

电机控制应用参数
22
电机控制应用参数
STM32 MC Motor Pilot从电路板获取电机控制应用参数，并根据这些参数来调整小部件

## Slide 16

电机控制应用高级设置(1)
23
点击高级配置
速度PID
、
力
矩PID
、
磁
通量PID和
磁
通量参考
值

## Slide 17

电机控制应用高级设置(2)
24
速度PID
、
力
矩PID
、
磁
通量PID和
磁
通量参考
值

## Slide 18

电机控制应用高级设置(3)
25
状态观察器 + PLL参数
状态观察器 + CORDIC参数
DAC设置
状态观察器、PLL参数、CORDIC参数和DAC设置
CPU load
Rev-
up设置

## Slide 19

通过轮询来查看寄存器（低速绘图）
26
点击“寄存器”选项卡可显示所有寄存器的列表，该列表可流向绘图

## Slide 20

通过轮询来绘制寄存器（低速绘图）
27
用于异步绘图的寄存器选择可供使用

## Slide 21

通过轮询来绘制寄存器（低速绘图）
28

## Slide 22

数据记录服务
29
提供类似于示波器的显示，以绘制高频任务

## Slide 23

用于电机控制的
STM32

## Slide 24

监控
FW
里的变量
(1/2)
30
在
register_interface.h
里增加变量的宏定义
在
register_interface.c
里的函数“
in
RI_SetReg
”增加可写变量
在
register_interface.c
里的函数“
in
RI_GetReg
”增加可读变量
在
register_interface.c
里的函数“
in
RI_GetPtrReg
”增加指针

## Slide 25

监控
FW
里的变量
(2/2)
31
如果
Motor Pilot
使用默认的
GUI
，请修改
RegListSTMV2.json
，如下，注意
16
位的加在
16
位区，
32
位的加在
32
位区，
Id
要和
register_interface.h
文件中加入的宏定义一致，
Type
要写对，如果要在
High_Frequency_Plot
中监控，一定要有最后那句
：
然后就可以重新点击下面图标，开始监控操作了

## Slide 26

X-CUBE-MCSDK
的要点

## Slide 27

要
点 (1)
33
PC软件应用可自动测量PMSM电机的机电参数
(
STM32F3
,
STM32G4,STM32F4,STM32L4,STM32F7
)；
评估板的LCD屏幕
不支持
；
开发工具链：
面向ARM（IAR系统AB）v8.20.2的IAR嵌入式工作台（
不支持v7.x.x
）
面向Arm® (Keil® MDK) v5.24.2的μVision® IDE
STM32CubeIDE v1.6.1
STM32CubeProgrammer 2.6.0
同时驱动一个或两个不同的电机
（
对于
6步
只有
1
个
）

## Slide 28

要
点 (2)
34
X-CUBE-MCSDK版本5.Y不支持STM32F1系列
，目前
6.0
版本只支持
G0
和
G4
系统，
如果您使用的是STM32F1 MCU，请继续使用X-CUBE-MCSDK 5.4.x版本。
目前的X-CUBE-MCSDK版本5.Y
和
6.0
不支持双驱动器。
如果您使用的是双驱动器，请继续使用
X-CUBE-MCSDK 5.4.x版本。
之
前的X
-CUBE-
MCSDK版本生成的项目将不
能
载入
到
版本5.Y
和
6.0
。
如果您希望保持兼容性，请继续使用版本5.4.x。
X-CUBE-MCSDK版本5.Y
和
6.0
并未达到与旧的X-CUBE-MCSDK版本5.4.
x
相同的成熟度水平
X-CUBE-MCSDK版本5.Y/6.0的发布为电机控制SDK的开发指明了方向

## Slide 29

要
点 (3)
35
之前，设定或
返回速度的API函数
（例如
MC_ProgramSpeedRampMotor1()
或
MC_GetMecSpeedAverageMotor1()
）
所使用的速度单位
为
01Hz（十分之一赫兹）
现在可以为这些函数使用其他单位。版本5.4.0中提供了两个新的速度单位：
RPM（每分钟转数）
001Hz（百分之一赫兹）
在编译时通过在
mc_stm_types.h
文件中将SPEED_UNIT符号设为适当的值来选择速度单位：
_RPM
_001HZ
_01HZ
SPEED_UNIT
定义放在用户部分中，并在项目
重新生成后仍可
保留
。
SPEED_UNIT（从版本v5.4.0开始）

## Slide 30

要
点 (4)
36
无传感器的电角度补偿
（从版本v5.4.0开始）
PID
PID
反
PARK
和电压矢量幅值限制
SVPWM
读取电流
CLARK
PARK
转子速度
/
位置反馈
i
qs
*
i
ds
*
IPM
M
v
qs
v
ds
v
αβ
i
αβ
i
abc
i
qs
i
ds
ω
Θ
e
[n-1]
Θ
e
[n]
Θ
d[n-1]
Θ
d[n]
Θ
e
[n-1] +
ω
dpp
*
REV_
PARK_ANGLE_COMPENSATION_FACTOR
Θ
e
[n-1] +
ω
dpp
*
PARK_ANGLE_COMPENSATION_FACTOR

## Slide 31

要
点 (5)
新增电压极限圆限制方法
（从版本v5.4.0开始）
新增的
Circle limitation
MAX_MODULATION
MAX_Vd = MAX_MODULATION*0.95
超出
MAX_MODULATION
后新增的方法是尽量保持
的大小不变
(
不超过
MAX_Vd)
将
缩小到
。这样经过
Circle Limit
的矢量
更偏向
d
轴的方向，弱磁的效果更好。
37

## Slide 32

要
点 (6)
相移方式的单电阻采样
（从版本v5.Y.0开始）
Boundary zone 1:
The firmware shifts on the right the smallest duty cycle by TMIN (minimum window time for sampling)
Boundary zone 2:
The firmware shifts on the right the most middle duty cycle by TMIN (minimum window time for sampling)
Boundary zone 3:
The firmware shifts on the right the smallest duty cycle and on the left the highest duty cycle by TMIN (minimum window time for sampling)
38

## Slide 33

要
点 (7)
可选的过调制算法
（从版本v5.Y.0开始）
Linear mode
OVM1
OVM2
39

## Slide 34

用于电机控制的
STM32
4

## Slide 35

要
点 (8)
可选的
5
段式
SVPWM
（从版本v5.Y.0开始）
40

## Slide 36

Find out more at
www.st.com

## Slide 37

32位Arm Cortex-M4
内
核
，
带FPU
ART + CCM-SRAM + 数学加速器
带ECC的单存储区闪存
带奇偶校验位的SRAM
+/- 1%内部时钟
1.72至3.6V电源
高达 125°C
STM32G491框图
基本型[32KB ..512KB]
高级电机控制定时器
丰富、先进的模拟
CAN灵活数据速率
USB-C Power Delivery3.0
高级安全与安全特性
稳健性：最高级别5/ FTB/ESD - IEC 61000-4-4
5

## Slide 38

STM32 G0超值型-STM32G030
6
32位Arm Cortex-M0+内核
2.0至3.6V电源
RAM最大化
1%内部时钟
直接存储器访问（DMA）
通信外设
定时器
实时时钟
I/O端口最大化
12位超快速ADC
安全特性

## Slide 39

用于电机控制的STM32H7功能
7
特性
STM32H723/733/725/735/730
优势
内核
Cortex M7
性能与效率
FPU
有
性能与效率
MPU
有
安全
CPU最大频率
550MHz
性能与效率
DMIPS
1177
性能与效率
Flash/SRAM数据大小
128KB至1MB/564KB
性能与集成/成本
包括：ICTM/DCTM RAM
高达256KB（可配置）/128KB
性能与效率
错误代码校正
全存储器映射上的SECDED
安全
ADC SAR
2x16
-
bit 3.6Msps，1x12
-
bit 5Msps
效率
其他模拟
2xcomp, 2xPGA, 2xDAC, 1xDFSDM
集成/成本
高级电机控制定时器
2x（275MHz）
性能与效率
缓存与加速器
32KB+32KB L1缓存
显卡、CORDIC、FMAC、Cypro*
性能与效率
安全服务
（SFI与SB-SFU）
有*
系统完整性
封装
VFQFPN68
LQFP100/144/176
BGA100/144/169/176
WLCSP11
成本/集成/灵活性
最大温度范围°C
[-40 ..+125]
Tj max 140°C
集成与成本

## Slide 40

X-CUBE-
MCSDK
更新

## Slide 41

STM32电机控制生态系统
9
X-CUBE-MCSDK
ST-MC-SUITE
电机分析仪
STM32 Cube生态系统
STM32电机控制Wiki
