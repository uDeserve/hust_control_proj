#!/usr/bin/env python3
"""
学生自己写的小工具：
1. 连接串口
2. 自动把实验数据保存成 CSV
3. 文件名里带上日期、实验编号、方法名和参数，后面找起来快
"""

from __future__ import annotations

import argparse
import csv
import datetime as dt
import json
import re
import sys
from pathlib import Path

try:
    import serial
except ImportError as exc:  # pragma: no cover
    raise SystemExit("缺少 pyserial，请先安装：pip install pyserial") from exc


HEADER = [
    "time_ms",
    "target_speed_rpm",
    "raw_speed_rpm",
    "filtered_speed_rpm",
    "final_speed_rpm",
    "session_id",
    "config_index",
    "method_name",
    "param_tag",
    "phase",
    "stop_reason",
    "sample_index",
]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="电机滤波实验自动采集器")
    parser.add_argument("--port", required=True, help="串口号，例如 COM11")
    parser.add_argument("--baud", type=int, default=1843200, help="波特率，默认 1843200")
    parser.add_argument(
        "--output-dir",
        default=str(Path.home() / "Documents" / "mc_speed_logs"),
        help="CSV 输出目录",
    )
    parser.add_argument(
        "--series-name",
        default="speed_filter_suite",
        help="实验系列名，会写进索引文件",
    )
    return parser.parse_args()


def sanitize_name(text: str) -> str:
    cleaned = re.sub(r"[^0-9A-Za-z_-]+", "_", text.strip())
    return cleaned.strip("_") or "unknown"


def build_output_path(base_dir: Path, row: dict[str, str]) -> Path:
    now = dt.datetime.now()
    stamp = now.strftime("%Y%m%d_%H%M%S")
    session_id = row.get("session_id", "0")
    config_index = row.get("config_index", "0")
    method_name = sanitize_name(row.get("method_name", "unknown"))
    param_tag = sanitize_name(row.get("param_tag", "unknown"))
    filename = f"{stamp}_s{session_id}_cfg{config_index}_{method_name}_{param_tag}.csv"
    return base_dir / filename


def ensure_index_file(base_dir: Path) -> Path:
    return base_dir / "session_index.jsonl"


def append_index(index_path: Path, meta: dict[str, str]) -> None:
    with index_path.open("a", encoding="utf-8") as fp:
        fp.write(json.dumps(meta, ensure_ascii=False) + "\n")


def open_new_session(base_dir: Path, first_row: dict[str, str], series_name: str):
    csv_path = build_output_path(base_dir, first_row)
    csv_file = csv_path.open("w", newline="", encoding="utf-8-sig")
    writer = csv.DictWriter(csv_file, fieldnames=HEADER)
    writer.writeheader()

    meta = {
        "created_at": dt.datetime.now().isoformat(timespec="seconds"),
        "series_name": series_name,
        "csv_file": str(csv_path),
        "session_id": first_row.get("session_id", ""),
        "config_index": first_row.get("config_index", ""),
        "method_name": first_row.get("method_name", ""),
        "param_tag": first_row.get("param_tag", ""),
        "target_speed_rpm": first_row.get("target_speed_rpm", ""),
    }
    append_index(ensure_index_file(base_dir), meta)
    return csv_path, csv_file, writer


def is_header_line(line: str) -> bool:
    return line.strip() == ",".join(HEADER)


def is_session_end(line: str) -> bool:
    return line.startswith("#session_end")


def parse_csv_row(line: str) -> dict[str, str] | None:
    parts = [item.strip() for item in line.split(",")]
    if len(parts) != len(HEADER):
        return None
    return dict(zip(HEADER, parts))


def run_logger(args: argparse.Namespace) -> int:
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    current_csv_path: Path | None = None
    current_csv_file = None
    current_writer = None
    current_session_id: str | None = None

    print(f"正在监听串口 {args.port}，输出目录：{output_dir}")
    print("按 Ctrl+C 结束。")

    try:
        ser = serial.Serial(args.port, args.baud, timeout=0.3)
    except serial.SerialException as exc:
        print(f"打开串口失败：{exc}")
        print("请先确认板子已连接，并改用正确的 COM 口，比如 COM11。")
        return 2

    with ser:
        while True:
            try:
                raw = ser.readline()
            except KeyboardInterrupt:
                break

            if not raw:
                continue

            try:
                line = raw.decode("utf-8", errors="ignore").strip()
            except KeyboardInterrupt:
                break

            if not line:
                continue

            print(line)

            if is_header_line(line):
                continue

            if is_session_end(line):
                if current_csv_file is not None:
                    current_csv_file.close()
                    print(f"会话结束，已保存：{current_csv_path}")
                current_csv_path = None
                current_csv_file = None
                current_writer = None
                current_session_id = None
                continue

            if line.startswith("#"):
                continue

            row = parse_csv_row(line)
            if row is None:
                continue

            if current_writer is None or current_session_id != row["session_id"]:
                if current_csv_file is not None:
                    current_csv_file.close()
                current_csv_path, current_csv_file, current_writer = open_new_session(
                    output_dir, row, args.series_name
                )
                current_session_id = row["session_id"]
                print(f"新建会话文件：{current_csv_path}")

            current_writer.writerow(row)
            current_csv_file.flush()

    if current_csv_file is not None:
        current_csv_file.close()

    return 0


def main() -> int:
    args = parse_args()
    return run_logger(args)


if __name__ == "__main__":
    sys.exit(main())
