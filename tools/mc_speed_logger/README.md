# 电机滤波实验自动采集器

这个小工具是配合板子上的“实验轮换模式”一起用的。

## 作用

- 自动监听串口
- 自动识别一次实验开始和结束
- 自动保存成 CSV
- 自动按统一规则命名
- 自动写一份 `session_index.jsonl`，后面查数据方便

## 文件命名规则

生成的 CSV 名字格式：

```text
YYYYMMDD_HHMMSS_s<session_id>_cfg<config_index>_<method_name>_<param_tag>.csv
```

例如：

```text
20260517_213522_s3_cfg2_LPF1_shift3.csv
```

这样你一眼就能看出：

- 采集时间
- 第几次实验
- 第几组配置
- 用的是什么方法
- 参数是什么

## 输出目录

默认保存到：

```text
%USERPROFILE%\Documents\mc_speed_logs
```

同时会在同目录生成：

```text
session_index.jsonl
```

这个索引文件会记录每个 CSV 的元信息，后面你做汇总对比会比较方便。

## 安装依赖

先装 Python 依赖：

```bash
pip install pyserial
```

## 使用方法

假设串口是 `COM5`：

```bash
python logger.py --port COM5
```

如果你想指定目录：

```bash
python logger.py --port COM5 --output-dir D:\mc_logs
```

## 建议实验流程

1. 先开这个采集器
2. 再给板子上电
3. 按一次 `USER` 键：启动第 1 组配置
4. 电机稳定后再按一次 `USER` 键：正常停止
5. 再按一次 `USER` 键：自动切到下一组配置并启动
6. 重复，直到多组数据都采完
7. 最后把生成的多个 CSV 一起拖到可视化页面里对比
