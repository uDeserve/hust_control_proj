# Contributing

## 1. 总原则

- 权威工程目录只有一个：`firmware/current_project`
- 任何会影响板上可运行性的改动，都必须能说明：
  - 改了什么
  - 为什么改
  - 如何验证
  - 风险在哪里
- `patches/current_patch` 只是快照，不是主工程

## 2. 允许优先修改的区域

- `firmware/current_project/Src`
- `firmware/current_project/Inc`
- `firmware/current_project/MCSDK_v5.Y.4-Full/MotorControl/MCSDK/MCLib/Any`
- `tools/mc_speed_logger`
- `tools/mc_speed_viewer`
- `matlab/speed_filter_sim`
- `docs`

## 3. 慎改区域

- `first_try.ioc`
- `first_try.wb_def`
- `Drivers/`
- `STM32CubeIDE/`
- `MDK-ARM/`

这些目录不是绝对不能改，但只要改动它们，就必须在 PR 描述里解释原因和影响。

## 4. 提交要求

- 每次 PR 尽量只做一件事
- 提交信息不要写成“update”或“fix”
- 推荐格式：

```text
feat: analyze STO+PLL speed path and add notes
fix: adjust adaptive filter parameter switching logic
docs: add teammate onboarding and PR workflow
```

## 5. 合并前自检

- 工程结构是否仍然完整
- 是否误提交了构建产物、日志或临时文件
- 是否同步更新了相关文档
- 如果改了 `mc_tasks.c/.h`，是否考虑更新 `patches/current_patch`
