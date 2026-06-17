# 验收版代码导读：`mc_tasks.c` 与 `mc_speed_viewer`

这份文档不是报告正文，也不是实验记录。

它的用途很直接：

1. 帮你快速找到这次课设真正改动的代码位置
2. 帮你按“验收时能讲清楚”的顺序去读代码
3. 帮你把“滤波 / PLL 参数 / 切换保护”三种演示模式区分开

建议你打开源码和这份文档一起看，而不是只看文档。

---

## 1. 先看哪几个文件

这次现场演示最核心的代码文件就两个：

1. `firmware/current_project/Src/mc_tasks.c`
2. `tools/mc_speed_viewer/app.js`

如果只想先把固件主流程看懂，优先看：

1. `mc_tasks.c` 开头的宏和配置表
2. `mc_tasks.c` 的实验配置生效函数
3. `mc_tasks.c` 的启动切换逻辑
4. `mc_tasks.c` 的串口 CSV 输出逻辑

---

## 2. 先建立一个总认识

当前工程底层的无感速度估计方法，仍然是 MCSDK 自带的 `STO + PLL`。

我们这次自己加的东西，主要在它后面：

```text
STO + PLL 输出原始速度
    -> 我们实现的滤波层
    -> 我们实现的 PLL 分程参数切换
    -> 我们实现的启动切换保护
    -> 速度环正式使用的速度反馈
    -> 串口导出 CSV
    -> HTML 可视化对比
```

所以你讲的时候一定要区分：

- `STO + PLL` 是原有基线
- 我们改的是“速度估计结果怎么处理、怎么接管、怎么对比展示”

---

## 3. 从 `mc_tasks.c` 开头开始看

### 3.1 第一眼看演示总开关

文件最开头最值得记住的是这一段：

- `MC_DEMO_MODE_FILTER_COMPARE`
- `MC_DEMO_MODE_PLL_COMPARE`
- `MC_DEMO_MODE_SWITCH_COMPARE`
- `MC_DEMO_MODE`

这里的含义很简单：

> 现场演示时，只需要改 `MC_DEMO_MODE`，就能切到三种不同实验。

三种模式分别是什么：

1. `FILTER_COMPARE`
   只看不同滤波器效果
2. `PLL_COMPARE`
   只看不同 PLL 参数组合对速度估计的影响
3. `SWITCH_COMPARE`
   只看启动切换保护开和关的差别

### 3.2 滤波模式为什么还单独有一个保护开关宏

你会看到还有一个宏：

- `MC_FILTER_COMPARE_STARTUP_PROTECT_ENABLE`

这个宏是专门给滤波演示准备的。

因为你后来提了一个很合理的要求：

> 滤波对比时，默认最好把切换保护关掉，保证滤波差异更纯；但现场如果想顺带看“滤波 + 保护”的效果，也要保留这个权力。

所以现在的设计是：

- 滤波演示模式下，PLL 固定
- 启动保护默认关
- 但你可以只改这一处宏，把它切成开

这样比去改七个配置项都稳很多。

### 3.3 启动保护相关门限在哪里

开头还有三个和启动切换保护有关的宏：

1. `MC_STARTUP_SWITCH_MIN_SPEED_RPM`
2. `MC_STARTUP_SWITCH_STABLE_MS`
3. `MC_STARTUP_ESTIMATE_CONFIRM_SAMPLES`

它们的作用分别是：

1. 最小速度门限
   速度没过这个门限，不允许太早切换
2. 稳定等待时间
   不是一过门限就切，而是要稳定一小段时间
3. 连续确认拍数
   避免只因为某一两个采样点看起来正常就误判

讲的时候可以概括成一句话：

> 我们不是简单延后切换，而是让速度估计结果“连续稳定、且满足门限”之后再接管。

---

## 4. 看实验配置表：这是整份代码最重要的一块

在 `mc_tasks.c` 里，最值得你熟悉的就是三个配置表。

### 4.1 `g_mcFilterCompareConfigs[]`

这一组是滤波演示表。

这里每一行就是一组实验。

你要会看每行里最关键的字段：

1. `mode`
   选的是哪种滤波器
2. `lpfShift`
   对低通滤波有意义
3. `methodName`
   导出到 CSV 后给页面显示的算法名
4. `paramName`
   导出到 CSV 后给页面显示的参数标签
5. `pll...`
   这一整组在滤波模式下是固定的
6. `startupProtectEnable`
   这里统一由总宏 `MC_FILTER_COMPARE_STARTUP_PROTECT_ENABLE` 控制

这一组配置表的讲法很简单：

> 同一套 PLL、同一套速度轨迹，只换滤波器，这样能把不同滤波方法的差异直接对比出来。

### 4.2 `g_mcPllCompareConfigs[]`

这一组是 PLL 演示表。

这里的设计重点是：

- 滤波固定为 `NONE`
- 启动保护固定关掉

这样做的原因你要记住：

> 因为 PLL 模式是专门看估计器本体的动态差异，如果再叠加滤波或保护，老师就看不清到底是谁在起作用。

你在表里会看到两类名字：

1. `PLL_FIX`
   固定一套 `Kp/Ki`
2. `PLL_SPLIT`
   低速和较稳定阶段使用不同参数

参数串例如：

- `kp165_ki4`
- `fast245_5_slow165_3_e17x38_n32`

讲的时候可以翻译成：

- 固定参数：`Kp=165, Ki=4`
- 分程参数：快段一套，慢段一套，带切换门限和确认拍数

### 4.3 `g_mcSwitchCompareConfigs[]`

这一组是切换保护演示表。

这里只有两行，故意设计成很像：

1. `protect_off`
2. `protect_on`

而且方法名都保留成 `ADALPF`。

这一步非常重要，因为它体现了我们现在的数据语义是干净的：

- 算法本身还是 `ADALPF`
- 真正比较的是“启动切换保护是否开启”

所以你可以这样讲：

> 切换保护模式下，我们把滤波器和 PLL 参数都固定成同一套，只比较启动保护开关前后对切换平顺性的影响。

---

## 5. 配置表是怎么真正生效的

只看配置表还不够，还要知道这些参数怎么进入系统。

### 5.1 `MC_EXPERIMENT_GetConfigTable()`

这个函数就是模式分发器。

逻辑很简单：

- 当前模式是 `FILTER_COMPARE`，就返回滤波配置表
- 当前模式是 `PLL_COMPARE`，就返回 PLL 配置表
- 当前模式是 `SWITCH_COMPARE`，就返回切换保护配置表

它的意义就是：

> 同一套实验状态机，不同模式只换数据表，不需要复制三套流程代码。

### 5.2 `MC_EXPERIMENT_GetCurrentConfig()`

这个函数返回“当前这一轮实验”要使用的配置。

这里现在还加了一个小兜底：

- 如果 `g_mcExperimentConfigIndex` 越界，就自动回到 0

这个改动虽然不显眼，但很实用。

因为现场演示代码最怕状态机跑乱后数组越界。

### 5.3 `MC_EXPERIMENT_ApplyConfig()`

这个函数是整套实验配置真正生效的入口。

你要重点记住它干了什么：

1. 设置滤波模式 `MC_SPEED_SetFilterMode(...)`
2. 设置低通参数 `MC_SPEED_SetLpfShift(...)`
3. 设置自适应低通参数
4. 设置启动保护开关 `g_mcStartupProtectEnable`
5. 清零启动保护内部计数器
6. 调用 `MC_PLL_TuningApply(...)` 把 PLL 参数应用进去

讲的时候一句话总结：

> 配置表不是只存在于表里，而是通过 `MC_EXPERIMENT_ApplyConfig()` 在每轮实验开始时真正写进运行状态。

---

## 6. 速度估计结果是在哪里进入我们这层处理的

这部分你一定要会讲，因为它是“滤波为什么会影响控制效果”的关键。

在中频任务里，原始 `STO + PLL` 速度估计会先被算出来，然后送到我们加的滤波层里。

核心链路可以概括成：

1. `STO_PLL_CalcAvrgMecSpeedUnit(...)`
2. `MC_SPEED_FilterUpdateMecSpeed(...)`
3. 后续切换到 `SpeedFilterSensorM1`

这意味着：

> 我们做的滤波不是只为了画图，而是真的会进入速度环反馈。

这也是为什么滤波会影响：

- 启动过程
- 稳态抖动
- 调速过程
- 停转尾段

---

## 7. 启动切换保护到底加在了哪里

这是这次代码导读里最关键的一段。

### 7.1 原版逻辑是什么

原版大概是：

```text
先按虚拟速度启动
    -> 观测器后台运行
    -> 如果看起来已经能闭环
    -> 就切换到 RUN
```

它的问题不是完全错误，而是低速时可能过早相信 observer。

### 7.2 我们现在怎么改的

你在 `SWITCH_OVER` 状态里能看到：

1. 先算 `LoopClosed`
2. 再调用 `MC_STARTUP_IsSwitchReady(...)`
3. 只有 `SwitchReady != 0U` 才真正切到 `RUN`

也就是说，`LoopClosed` 不再等于“立刻切换”。

它现在只是：

> 说明系统“有切换资格”，但还要再经过我们这一层可信度判断。

### 7.3 `MC_STARTUP_IsSwitchReady()` 具体在判断什么

这个函数建议你一行一行看。

它现在的判断顺序是：

1. 如果 `loopClosed == 0`
   说明原始闭环条件还没满足，直接不放行
2. 如果当前配置里保护关掉
   直接返回允许切换
3. 看 `filteredSpeedRpm` 是否过最小门限
4. 看 `rawSpeedRpm` 是否仍然出现反向毛刺
5. 连续确认若干拍
6. 再稳定等待一小段时间
7. 都满足后才返回允许切换

这里有一个你要特别记住的细节：

- 幅值门限看的是 `filteredSpeedRpm`
- 方向毛刺看的是 `rawSpeedRpm`

这是我们后来专门整理过的口径。

原因可以这样理解：

1. 是否“基本达到可切换速度”更适合看滤波后的速度
2. 是否“还存在反向毛刺”更适合看原始估计速度

这样讲老师一般是能接受的。

---

## 8. 真正接管速度环的是哪一句

在 `SWITCH_OVER` 里，真正值得你圈出来的一句是：

```c
STC_SetSpeedSensor(pSTC[M1], &SpeedFilterSensorM1);
```

这句是整套课设里最关键的“落地点”之一。

因为它说明：

> 最终接到速度环上的，不是原始 observer 速度，而是我们处理后的那份速度传感器句柄。

你讲“为什么滤波会影响控制”时，就可以直接指这句。

---

## 9. 自动实验和串口输出是怎么做的

现场演示不只是电机转起来，还要能自动跑完、自动导出 CSV。

这一部分也是 `mc_tasks.c` 里自己加的。

### 9.1 `MC_EXPERIMENT_StartSequence()`

它负责：

1. 取当前配置
2. 应用配置
3. 启动电机
4. 下发第一段目标速度
5. 初始化当前 session 的状态
6. 开始串口文本输出

### 9.2 `MC_EXPERIMENT_UpdateSpeedProfile()`

这个函数负责自动跑速度轨迹。

当前用的是：

1. 先到 `500 rpm`
2. 保持 `6 s`
3. 提到 `1000 rpm`
4. 保持 `6 s`
5. 再回到 `500 rpm`
6. 保持 `6 s`
7. 自动停机

所以你后面看到的 CSV 和 HTML 曲线，都是按照这套轨迹来的。

### 9.3 `MC_EXPERIMENT_PushSample()` 和 `MC_EXPERIMENT_FormatCsvLine()`

这两个函数负责把实验数据送到串口。

当前一行 CSV 里最重要的字段有：

1. `target_speed_rpm`
2. `raw_speed_rpm`
3. `filtered_speed_rpm`
4. `final_speed_rpm`
5. `pll_kp`
6. `pll_ki`
7. `pll_stage`
8. `method_name`
9. `param_tag`
10. `phase`

你讲的时候可以这样区分：

- `raw_speed_rpm`：原始速度估计
- `filtered_speed_rpm`：滤波后的速度
- `final_speed_rpm`：控制实际使用的速度

其中在 PLL 模式里，重点看的是 `raw_speed_rpm`。

---

## 10. `app.js` 要怎么看

如果说 `mc_tasks.c` 负责“跑实验和导数据”，那 `app.js` 就负责“把数据翻译成老师看得懂的东西”。

### 10.1 先看 `methodLabelMap`

这里是把 CSV 里的英文代码转成中文。

例如：

- `NONE` -> `无滤波`
- `LPF1` -> `一阶低通`
- `PLL_FIX` -> `固定参数锁相环`

这个映射很重要，因为没有它的话，老师看到的就只是一堆文件名和参数串。

### 10.2 三种页面模式

页面里也有三种模式：

1. `filter`
2. `pll`
3. `switch`

它们要和固件里的三种 `MC_DEMO_MODE` 对应起来理解。

固件和页面的对应关系你要记住：

1. `MC_DEMO_MODE_FILTER_COMPARE`
   对应前端 `filter`
2. `MC_DEMO_MODE_PLL_COMPARE`
   对应前端 `pll`
3. `MC_DEMO_MODE_SWITCH_COMPARE`
   对应前端 `switch`

### 10.3 切换保护模式为什么现在按 `param_tag` 识别

这是我们刚修过的一点。

现在在切换保护模式里：

- `methodName` 仍然保留算法名，例如 `ADALPF`
- `param_tag` 决定是 `protect_on` 还是 `protect_off`

这样做的好处是数据语义更干净。

也就是说：

> 算法还是同一个算法，只是保护策略不同。

这比把 `methodName` 直接写成 `SWITCH_ON / SWITCH_OFF` 更规范。

---

## 11. 建议你按什么顺序自己读代码

下面这个顺序是最适合现在你的。

### 第一轮：只建立地图

只看这些位置，不求全懂：

1. `MC_DEMO_MODE`
2. 三个配置表
3. `MC_EXPERIMENT_ApplyConfig()`
4. `SWITCH_OVER` 里的 `MC_STARTUP_IsSwitchReady()`
5. `STC_SetSpeedSensor(..., &SpeedFilterSensorM1)`

这一轮的目标是：

> 知道“我们到底改了哪里”。

### 第二轮：顺着一次实验跑流程

按这个流程读：

1. 按 USER 键
2. `UI_HandleStartStopButton_cb()`
3. `MC_EXPERIMENT_StartSequence()`
4. 自动进入速度轨迹
5. `MC_EXPERIMENT_PushSample()`
6. `MC_EXPERIMENT_SendTextIfPossible()`
7. CSV 导出

这一轮的目标是：

> 知道“从按键到最后出 CSV，中间经过了哪些函数”。

### 第三轮：专门讲启动切换保护

只盯着下面几处看：

1. `SWITCH_OVER`
2. `MC_STARTUP_IsSwitchReady()`
3. `STC_SetSpeedSensor(...)`

这一轮你要能讲出：

1. 原版大概怎么切
2. 我们为什么要加保护
3. 现在保护到底判断了什么
4. 为什么这样改能减少低速毛刺带来的误切换

---

## 12. 你现场最该会讲的 6 句话

如果时间很紧，你至少要把下面 6 句话讲顺：

1. 当前底层速度估计基线是 MCSDK 自带的 `STO + PLL`，不是我们从零重写的。
2. 我们的改动主要在速度估计结果后处理这一层，包括滤波、PLL 参数优化和启动切换保护。
3. `mc_tasks.c` 里用一个总 `Mode` 开关，把现场演示分成滤波对比、PLL 对比、切换保护对比三种模式。
4. 滤波不是只为了显示，因为最终接入速度环的是 `SpeedFilterSensorM1`。
5. 启动切换保护的核心思想不是简单延时，而是让速度估计满足门限、连续稳定后再接管。
6. 导出的 CSV 再进入 HTML 页面，就能把不同方法的曲线和指标做成本地可视化对比。

---

## 13. 你如果接下来还要继续学，下一步看什么

如果这份文档你已经能跟着源码看懂大半，下一步建议这样推进：

1. 再去看 `docs/03_代码入口与职责边界.md`
   这份更偏“整个工程地图”
2. 再看 `docs/04_速度估计任务学习与改进指南.md`
   这份更偏“STO + PLL 本体”
3. 再看 `tmp_notes/32_STO_PLL速度估计链路答辩讲解稿.md`
   这份更偏“老师问你原理时怎么回答”

---

## 14. 这份文档最重要的一句话

把这句话记住，后面很多问题都能回答：

> 我们没有重写底层 `STO + PLL` 估计器，但我们围绕速度估计结果的滤波、参数优化、可信度保护和工程化接管，做了完整的代码实现、实验验证和可视化对比。

