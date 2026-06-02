# Git 协作与 PR 流程

## 1. 分支策略

推荐至少保持下面几类分支：

- `main`
  - 只放当前可用、已验证、可继续跟进的版本

- `feature/<topic>`
  - 新功能或新研究线

- `fix/<topic>`
  - 修 bug 或修稳定性问题

- `docs/<topic>`
  - 只改文档

## 2. 代码提交范围

一次 PR 最好只做一类事情：

- 只补文档
- 只补仿真
- 只补滤波代码
- 只做速度估计代码分析或小改

不要把这些混在一起，不然 review 成本高，也不好回退。

## 3. 提 PR 前必须回答的四个问题

1. 改了哪些文件
2. 这些文件为什么该改
3. 怎么验证没把主工程弄坏
4. 如果有风险，风险在哪里

## 4. PR 描述模板

建议用下面格式：

```text
## 目的
简述这次改动的目标

## 改动内容
- 文件 A：做了什么
- 文件 B：做了什么

## 验证
- 看了哪些代码路径
- 做了哪些仿真或实验
- 有没有实际编译/运行

## 风险
- 可能影响什么
- 哪些还没验证
```

## 5. 合并标准

能合并到 `main` 的改动，至少满足：

- 主工程结构没坏
- 改动目的清楚
- 文档跟上了
- 如果涉及控制链，至少说明验证情况

## 6. 主工程维护侧怎么跟进队友 PR

建议这样做：

1. 先看 PR 是否只改了声明范围内的文件
2. 再看是否碰了 `ioc/wb_def/Drivers`
3. 再看改动是否能解释清楚
4. 如果只改文档和分析，优先合并
5. 如果改了控制逻辑，优先先合到单独分支复测
6. 复测通过后再进 `main`

## 7. 同步补丁快照

如果合并的 PR 改动了下列文件：

- `firmware/current_project/Src/mc_tasks.c`
- `firmware/current_project/Inc/mc_tasks.h`
- `firmware/current_project/Src/main.c`

则合并后建议同步更新：

- `patches/current_patch/Src/mc_tasks.c`
- `patches/current_patch/Inc/mc_tasks.h`
- `patches/current_patch/Src/main.c`

可以用：

- `scripts/sync_patch_from_current_project.ps1`

## 8. 不建议的做法

- 直接在 `main` 上改完就提交
- 把临时 build 产物一起提上来
- 没说明验证情况就提大改
- 改主工程但不更新文档
