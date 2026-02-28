import tkinter as tk
from tkinter import filedialog, messagebox
import hashlib

def select_file1():
    file_path = filedialog.askopenfilename(
        title="选择第一个BIN文件",
        filetypes=[("BIN文件", "*.bin"), ("所有文件", "*")]
    )
    if file_path:
        entry1.delete(0, tk.END)
        entry1.insert(0, file_path)

def select_file2():
    file_path = filedialog.askopenfilename(
        title="选择第二个BIN文件",
        filetypes=[("BIN文件", "*.bin"), ("所有文件", "*")]
    )
    if file_path:
        entry2.delete(0, tk.END)
        entry2.insert(0, file_path)

def calculate_hash(file_path):
    """计算文件的MD5哈希值"""
    try:
        hasher = hashlib.md5()
        with open(file_path, 'rb') as f:
            while True:
                data = f.read(65536)  # 64KB chunks
                if not data:
                    break
                hasher.update(data)
        return hasher.hexdigest()
    except Exception as e:
        messagebox.showerror("错误", f"读取文件时出错: {str(e)}")
        return None

def compare_files():
    file1 = entry1.get()
    file2 = entry2.get()
    
    if not file1 or not file2:
        messagebox.showwarning("警告", "请选择两个BIN文件")
        return
    
    try:
        with open(file1, 'rb') as f1, open(file2, 'rb') as f2:
            data1 = f1.read()
            data2 = f2.read()
        
        # 比较文件大小
        if len(data1) != len(data2):
            messagebox.showinfo("对比结果", "两个BIN文件不一致！")
            print(f"文件大小不一致: {len(data1)} bytes vs {len(data2)} bytes")
            return
        
        # 逐字节比较
        diffs = []
        for i, (b1, b2) in enumerate(zip(data1, data2)):
            if b1 != b2:
                diffs.append((i, b1, b2))
        
        if not diffs:
            messagebox.showinfo("对比结果", "两个BIN文件一致！")
            print("两个BIN文件一致！")
        else:
            messagebox.showinfo("对比结果", "两个BIN文件不一致！")
            print(f"发现 {len(diffs)} 处不一致")
            print("前100处不一致的内容对比:")
            print("位置    文件1    文件2")
            print("-" * 30)
            for i, (pos, b1, b2) in enumerate(diffs[:100]):
                print(f"0x{pos:08X}  0x{b1:02X}    0x{b2:02X}")
    except Exception as e:
        messagebox.showerror("错误", f"比较文件时出错: {str(e)}")
        print(f"错误: {str(e)}")

# 创建主窗口
root = tk.Tk()
root.title("BIN文件对比工具")
root.geometry("500x200")

# 创建文件选择部分
frame = tk.Frame(root, padx=20, pady=20)
frame.pack(fill=tk.BOTH, expand=True)

# 第一个文件选择
label1 = tk.Label(frame, text="第一个BIN文件:")
label1.grid(row=0, column=0, sticky=tk.W, pady=5)

entry1 = tk.Entry(frame, width=40)
entry1.grid(row=0, column=1, padx=5, pady=5)

button1 = tk.Button(frame, text="浏览", command=select_file1)
button1.grid(row=0, column=2, padx=5, pady=5)

# 第二个文件选择
label2 = tk.Label(frame, text="第二个BIN文件:")
label2.grid(row=1, column=0, sticky=tk.W, pady=5)

entry2 = tk.Entry(frame, width=40)
entry2.grid(row=1, column=1, padx=5, pady=5)

button2 = tk.Button(frame, text="浏览", command=select_file2)
button2.grid(row=1, column=2, padx=5, pady=5)

# 对比按钮
compare_button = tk.Button(frame, text="对比文件", command=compare_files, width=20, height=2)
compare_button.grid(row=2, column=0, columnspan=3, pady=20)

# 运行主循环
root.mainloop()