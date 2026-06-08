import shutil
from pathlib import Path


def convert_env_to_utf8(env_path: Path) -> None:
    if not env_path.exists():
        print(f"文件不存在: {env_path}")
        return

    backup_path = env_path.with_suffix(env_path.suffix + ".bak")

    # 先备份一份原始文件
    if not backup_path.exists():
        shutil.copy2(env_path, backup_path)
        print(f"已创建备份: {backup_path}")
    else:
        print(f"备份已存在: {backup_path}")

    raw = env_path.read_bytes()
    print(f"原始字节长度: {len(raw)}")

    # 尝试直接按 UTF-8 解码，看是否仍然报错
    try:
        text = raw.decode("utf-8")
        print("当前文件已经是 UTF-8 编码，无需转换。")
        return
    except UnicodeDecodeError as e:
        print(f"按 UTF-8 解码失败: {e}")

    # 尝试按 GBK 解码（常见中文编码）
    try:
        text_gbk = raw.decode("gbk")
        print("按 GBK 解码成功，准备转存为 UTF-8。")
    except UnicodeDecodeError as e:
        print(f"按 GBK 解码也失败，请手工检查文件: {e}")
        return

    # 以 UTF-8 重新写回文件
    env_path.write_text(text_gbk, encoding="utf-8", newline="\n")
    print(f"已将文件 {env_path} 以 UTF-8 重新保存。")


if __name__ == "__main__":
    base_dir = Path(__file__).resolve().parent
    env_file = base_dir / ".env.dev"
    convert_env_to_utf8(env_file)

