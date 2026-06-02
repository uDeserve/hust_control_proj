# 队友与 Agent 快速上手

## 1. 你现在接手的是什么

这是一个基于 STM32 MCSDK 5.Y.4 的 PMSM 控制课设工程。

当前已经完成的主要内容：

- 主工程可以正常编译、下载、运行
- 已实现多种速度滤波方法
- 已有串口自动采集 CSV
- 已有本地 HTML 可视化对比工具
- 已有 MATLAB 仿真框架
- 已整理了较多实验记录和路线说明

你当前新增负责的是：

> 速度估计方法本体的代码层学习、理解、缺陷思考和改进探索

## 2. 你不应该一上来做什么

- 不要一上来就重写整套无感观测器
- 不要先改 `first_try.ioc` 或 `first_try.wb_def`
- 不要先碰 CubeMX 重新生成
- 不要先改 `Drivers/` 这种底层公共库
- 不要只看 `mc_tasks.c` 就以为速度估计逻辑都在那里

## 3. 你应该先做什么

建议按下面顺序：

1. 确认当前工程基线是 `STO + PLL`
2. 找出速度估计值是从哪来的
3. 看这个估计值怎么进入速度环
4. 再看我们后来是怎么在这个值上叠加滤波的
5. 最后才考虑“估计方法本体”可改哪里

## 4. 先看哪些文件

### A. 先看配置证据

- `firmware/current_project/first_try.wb_def`

这里已经能证明：

- `STATE_OBSERVER_PLL = TRUE`
- `SPEED_SENSOR_SELECTION = STATE_OBSERVER_PLL`
- `HALL_SENSORS_AVAILABLE = FALSE`
- `ENCODER_AVAILABLE = FALSE`

### B. 再看主流程

- `firmware/current_project/Src/mc_tasks.c`
- `firmware/current_project/Src/mc_config.c`

要重点找这些符号：

- `STO_PLL_M1`
- `STO_PLL_Init`
- `STO_PLL_CalcAvrgMecSpeedUnit`
- `STC_SetSpeedSensor`
- `SpeedFilterSensorM1`

### C. 再看速度估计库本体

- `firmware/current_project/MCSDK_v5.Y.4-Full/MotorControl/MCSDK/MCLib/Any/Src/sto_pll_speed_pos_fdbk.c`
- `firmware/current_project/MCSDK_v5.Y.4-Full/MotorControl/MCSDK/MCLib/Any/Inc/sto_pll_speed_pos_fdbk.h`
- `firmware/current_project/MCSDK_v5.Y.4-Full/MotorControl/MCSDK/MCLib/Any/Inc/sto_speed_pos_fdbk.h`
- `firmware/current_project/MCSDK_v5.Y.4-Full/MotorControl/MCSDK/MCLib/Any/Src/speed_pos_fdbk.c`
- `firmware/current_project/MCSDK_v5.Y.4-Full/MotorControl/MCSDK/MCLib/Any/Inc/speed_pos_fdbk.h`

## 5. 你和 agent 一起工作时的正确方法

推荐把任务拆成四步：

1. 先让 agent 只做“代码结构定位”
2. 再让 agent 只做“原理翻译 + 变量意义解释”
3. 再让 agent 只做“缺陷分析，不改代码”
4. 最后才让 agent 提具体补丁

这样做的原因是：

- 不容易一上来就大改
- 你自己会真的理解路径
- 补丁更容易 review

## 6. 给 agent 的任务边界建议

可以明确告诉 agent：

- 当前权威源码在 `firmware/current_project`
- 目前主工程必须保持可回退、可编译
- 优先做阅读、分析、局部验证
- 若要改速度估计本体，先做最小改动版本
- 每次只改一个问题，不要把滤波、估计、UI、仿真混在一个 PR

## 7. 你现在最值得产出的东西

不是马上提交一个大补丁，而是先产出这三类内容：

1. 一份“速度估计代码链路说明”
2. 一份“当前 `STO + PLL` 可能缺陷列表”
3. 一份“可尝试的小改进点清单”

## 8. 最后提醒

当前课设拿高分靠三件事：

- 代码真实
- 理论能讲清
- 有工作量和实验闭环

所以你的任务不是“写最炫的代码”，而是做出：

- 真看懂
- 真能解释
- 真能被合入主工程
