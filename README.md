# PMSM Speed Estimation and Filter Course Project

这个仓库用于多人协作维护当前课程设计工程，目标不是只保存代码，而是把下面几件事一起固定下来：

- 当前可运行工程
- 基线工程
- 关键补丁快照
- 串口采集与本地可视化工具
- MATLAB 仿真
- 面向队友和 agent 的上手文档
- 后续 Git/PR 协作规范

## 当前项目定位

当前项目不是“从零重写速度估计器”，而是：

> 基于 MCSDK 现有 `State Observer + PLL` 速度估计结果，设计和验证速度滤波改进方法，并为后续“速度估计方法本体”的代码层改进留出研究入口。

这一定义非常重要，因为后续队友的任务会延伸到“速度估计方法本体”的代码学习、缺陷分析和改进，但当前能稳定跑起来的主线仍然是：

- `STO + PLL` 基线估计
- 速度滤波实验
- 仿真 + 实测 + 对比分析

## 仓库结构

```text
firmware/
  baseline_project/         基线工程，方便和当前工程做 diff
  current_project/          当前主工程，后续修改以这里为准

patches/
  current_patch/            轻量补丁快照，便于快速看核心改动

tools/
  mc_speed_logger/          串口自动采集 CSV
  mc_speed_viewer/          本地 HTML 对比页面

matlab/
  speed_filter_sim/         MATLAB 仿真脚本与结果

docs/
  reference_notes/          课设过程中整理出的精选资料与过程记录
  01_总览与目录.md
  02_队友与Agent快速上手.md
  03_代码入口与职责边界.md
  04_速度估计任务学习与改进指南.md
  05_Git协作与PR流程.md
  06_常见风险与不要乱动的地方.md

scripts/
  sync_patch_from_current_project.ps1
  sync_patch_to_current_project.ps1
```

## 源码权威位置

这一条必须统一：

- 权威源码目录：`firmware/current_project`
- `patches/current_patch` 只是便于快速 review 的轻量快照，不是权威工程

也就是说：

- 真正要改代码，请优先改 `firmware/current_project`
- 改完后如果触及补丁快照覆盖的文件，再执行同步脚本更新 `patches/current_patch`

## 立即开始

第一次上手建议按这个顺序：

1. 先读 `docs/01_总览与目录.md`
2. 再读 `docs/02_队友与Agent快速上手.md`
3. 如果你负责“速度估计方法本体”的研究，重点读 `docs/04_速度估计任务学习与改进指南.md`
4. 开始改代码之前，必须先看 `docs/05_Git协作与PR流程.md` 和 `docs/06_常见风险与不要乱动的地方.md`

## 当前工程的关键事实

- 电机当前主工程：`firmware/current_project`
- 基线工程：`firmware/baseline_project`
- 当前工程不是 Hall/编码器测速
- 当前配置使用 `State Observer + PLL`
- 证据在 `firmware/current_project/first_try.wb_def`
  - `HALL_SENSORS_AVAILABLE = FALSE`
  - `ENCODER_AVAILABLE = FALSE`
  - `STATE_OBSERVER_PLL = TRUE`
  - `SPEED_SENSOR_SELECTION = STATE_OBSERVER_PLL`
- 当前滤波实验的主改动集中在 `Src/mc_tasks.c` 和 `Inc/mc_tasks.h`
- 速度估计方法本体的关键库入口在：
  - `MCSDK_v5.Y.4-Full/MotorControl/MCSDK/MCLib/Any/Src/sto_pll_speed_pos_fdbk.c`
  - `MCSDK_v5.Y.4-Full/MotorControl/MCSDK/MCLib/Any/Inc/sto_pll_speed_pos_fdbk.h`
  - `MCSDK_v5.Y.4-Full/MotorControl/MCSDK/MCLib/Any/Src/speed_pos_fdbk.c`
  - `MCSDK_v5.Y.4-Full/MotorControl/MCSDK/MCLib/Any/Inc/speed_pos_fdbk.h`

## 协作原则

- 不要直接在 `main` 上堆修改
- 所有实质性改动走分支和 PR
- 没看清生成工程与库代码边界前，不要随便点 CubeMX 重新生成
- 队友的“速度估计改进”工作，优先做理解、定位、局部小改、可回退验证，不要一上来大改整条控制链

## 说明

这个仓库已经尽量按“无需重新搭复杂环境、拿到后就能理解和改代码”的思路整理好。  
如果后续要真正推送到 GitHub 远端，还需要在本机完成一次仓库初始化和远端绑定。
