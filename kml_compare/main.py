import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext
import xml.etree.ElementTree as ET
import os

class KMLComparator:
    def __init__(self, root):
        self.root = root
        self.root.title("KML文件对比工具")
        self.root.geometry("800x600")
        
        # 设置窗口图标（如果有）
        # self.root.iconbitmap("icon.ico")
        
        # 创建主框架
        self.main_frame = tk.Frame(self.root, padx=10, pady=10)
        self.main_frame.pack(fill=tk.BOTH, expand=True)
        
        # 创建文件选择区域
        self.file_frame = tk.Frame(self.main_frame)
        self.file_frame.pack(fill=tk.X, pady=(0, 20))
        
        # 第一个文件选择
        self.file1_label = tk.Label(self.file_frame, text="文件1:")
        self.file1_label.grid(row=0, column=0, sticky=tk.W, pady=5)
        
        self.file1_path = tk.StringVar()
        self.file1_entry = tk.Entry(self.file_frame, textvariable=self.file1_path, width=60)
        self.file1_entry.grid(row=0, column=1, sticky=tk.W, pady=5)
        
        self.file1_button = tk.Button(self.file_frame, text="浏览", command=self.browse_file1)
        self.file1_button.grid(row=0, column=2, padx=5, pady=5)
        
        self.view1_button = tk.Button(self.file_frame, text="查看", command=self.view_file1)
        self.view1_button.grid(row=0, column=3, padx=5, pady=5)
        
        # 第二个文件选择
        self.file2_label = tk.Label(self.file_frame, text="文件2:")
        self.file2_label.grid(row=1, column=0, sticky=tk.W, pady=5)
        
        self.file2_path = tk.StringVar()
        self.file2_entry = tk.Entry(self.file_frame, textvariable=self.file2_path, width=60)
        self.file2_entry.grid(row=1, column=1, sticky=tk.W, pady=5)
        
        self.file2_button = tk.Button(self.file_frame, text="浏览", command=self.browse_file2)
        self.file2_button.grid(row=1, column=2, padx=5, pady=5)
        
        self.view2_button = tk.Button(self.file_frame, text="查看", command=self.view_file2)
        self.view2_button.grid(row=1, column=3, padx=5, pady=5)
        
        # 对比模式选择
        self.compare_mode = tk.StringVar(value="xml")
        self.mode_frame = tk.Frame(self.main_frame)
        self.mode_frame.pack(pady=10)
        
        tk.Label(self.mode_frame, text="对比模式:").pack(side=tk.LEFT, padx=5)
        tk.Radiobutton(self.mode_frame, text="XML结构对比", variable=self.compare_mode, value="xml").pack(side=tk.LEFT, padx=10)
        tk.Radiobutton(self.mode_frame, text="纯文本对比", variable=self.compare_mode, value="text").pack(side=tk.LEFT, padx=10)
        tk.Radiobutton(self.mode_frame, text="详细对比", variable=self.compare_mode, value="detailed").pack(side=tk.LEFT, padx=10)
        
        # 输出目标选择
        self.output_target = tk.StringVar(value="ui")
        self.output_frame = tk.Frame(self.main_frame)
        self.output_frame.pack(pady=10)
        
        tk.Label(self.output_frame, text="输出目标:").pack(side=tk.LEFT, padx=5)
        tk.Radiobutton(self.output_frame, text="UI界面", variable=self.output_target, value="ui").pack(side=tk.LEFT, padx=10)
        tk.Radiobutton(self.output_frame, text="终端", variable=self.output_target, value="terminal").pack(side=tk.LEFT, padx=10)
        
        # 对比按钮
        self.compare_button = tk.Button(self.main_frame, text="开始对比", command=self.compare_files, font=("Arial", 12, "bold"))
        self.compare_button.pack(pady=10)
        
        # 结果显示区域
        self.result_frame = tk.Frame(self.main_frame)
        self.result_frame.pack(fill=tk.BOTH, expand=True)
        
        self.result_label = tk.Label(self.result_frame, text="对比结果:")
        self.result_label.pack(anchor=tk.W, pady=(0, 5))
        
        self.result_text = scrolledtext.ScrolledText(self.result_frame, width=100, height=20, font=("Courier New", 10))
        self.result_text.pack(fill=tk.BOTH, expand=True)
        
    def browse_file1(self):
        file_path = filedialog.askopenfilename(
            title="选择第一个KML文件",
            filetypes=[("KML文件", "*.kml"), ("所有文件", "*.*")],
            initialdir="C:/"
        )
        if file_path:
            self.file1_path.set(file_path)
    
    def browse_file2(self):
        file_path = filedialog.askopenfilename(
            title="选择第二个KML文件",
            filetypes=[("KML文件", "*.kml"), ("所有文件", "*.*")],
            initialdir="C:/"
        )
        if file_path:
            self.file2_path.set(file_path)
    
    def view_file1(self):
        """查看第一个文件的内容"""
        file_path = self.file1_path.get()
        if not file_path:
            messagebox.showwarning("警告", "请先选择文件1")
            return
        
        if not os.path.exists(file_path):
            messagebox.showerror("错误", f"文件不存在: {file_path}")
            return
        
        self.view_file_content(file_path, "文件1内容")
    
    def view_file2(self):
        """查看第二个文件的内容"""
        file_path = self.file2_path.get()
        if not file_path:
            messagebox.showwarning("警告", "请先选择文件2")
            return
        
        if not os.path.exists(file_path):
            messagebox.showerror("错误", f"文件不存在: {file_path}")
            return
        
        self.view_file_content(file_path, "文件2内容")
    
    def view_file_content(self, file_path, title):
        """查看文件内容"""
        try:
            # 尝试使用不同编码读取文件
            encodings = ['utf-8-sig', 'gbk', 'utf-16', 'utf-8']
            content = None
            
            for encoding in encodings:
                try:
                    with open(file_path, 'r', encoding=encoding) as f:
                        content = f.read()
                    break
                except UnicodeDecodeError:
                    continue
            
            # 如果所有编码都失败，使用latin-1
            if content is None:
                with open(file_path, 'r', encoding='latin-1') as f:
                    content = f.read()
            
            # 创建新窗口
            view_window = tk.Toplevel(self.root)
            view_window.title(title)
            view_window.geometry("800x600")
            
            # 创建滚动文本框
            text_widget = scrolledtext.ScrolledText(view_window, width=100, height=30, font=("Courier New", 10))
            text_widget.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
            
            # 插入文件内容
            text_widget.insert(tk.END, content)
            
            # 添加行号
            text_widget.config(state=tk.NORMAL)
            
        except Exception as e:
            messagebox.showerror("错误", f"读取文件时出错: {str(e)}")
    
    def escape_special_chars(self, text):
        """转义特殊字符，使隐藏字符可见"""
        # 转义常见的隐藏字符
        text = text.replace('\n', '\\n')
        text = text.replace('\r', '\\r')
        text = text.replace('\t', '\\t')
        text = text.replace(' ', '·')  # 用可见的点表示空格
        text = text.replace('\xc2\xa0', '\\xa0')  # 不间断空格
        return text
    
    def parse_kml(self, file_path):
        """解析KML文件并返回结构化数据"""
        try:
            tree = ET.parse(file_path)
            root = tree.getroot()
            # 定义KML命名空间
            namespaces = {
                'kml': 'http://www.opengis.net/kml/2.2'
            }
            
            # 提取KML文件的主要内容
            kml_data = {}
            
            # 提取Document
            documents = root.findall('.//kml:Document', namespaces)
            if documents:
                kml_data['Document'] = self.extract_children(documents[0], namespaces)
            
            # 提取Folder
            folders = root.findall('.//kml:Folder', namespaces)
            if folders:
                kml_data['Folders'] = []
                for folder in folders:
                    kml_data['Folders'].append(self.extract_children(folder, namespaces))
            
            # 提取Placemark
            placemarks = root.findall('.//kml:Placemark', namespaces)
            if placemarks:
                kml_data['Placemarks'] = []
                for placemark in placemarks:
                    kml_data['Placemarks'].append(self.extract_children(placemark, namespaces))
            
            return kml_data
        except ET.ParseError as e:
            messagebox.showerror("XML解析错误", f"解析文件 {os.path.basename(file_path)} 时出错:\n{str(e)}\n\n这通常是因为KML文件格式不正确，例如标签不匹配或格式错误。")
            return None
        except Exception as e:
            messagebox.showerror("错误", f"解析文件 {os.path.basename(file_path)} 时出错: {str(e)}")
            return None
    
    def extract_children(self, element, namespaces):
        """递归提取元素的子元素"""
        data = {}
        
        # 提取元素标签（去除命名空间）
        tag = element.tag.split('}')[-1] if '}' in element.tag else element.tag
        data['tag'] = tag
        
        # 提取元素文本
        if element.text and element.text.strip():
            data['text'] = element.text.strip()
        
        # 提取元素属性
        if element.attrib:
            data['attributes'] = element.attrib
        
        # 提取子元素
        children = element.findall('*')
        if children:
            data['children'] = []
            for child in children:
                data['children'].append(self.extract_children(child, namespaces))
        
        return data
    
    def compare_files(self):
        """对比两个KML文件"""
        file1 = self.file1_path.get()
        file2 = self.file2_path.get()
        
        if not file1 or not file2:
            messagebox.showwarning("警告", "请选择两个KML文件")
            return
        
        if not os.path.exists(file1):
            messagebox.showerror("错误", f"文件1不存在: {file1}")
            return
        
        if not os.path.exists(file2):
            messagebox.showerror("错误", f"文件2不存在: {file2}")
            return
        
        # 获取对比模式
        mode = self.compare_mode.get()
        
        self.result_text.delete(1.0, tk.END)
        
        # 优化UI更新：先清空结果，然后批量更新
        self.result_text.delete(1.0, tk.END)
        
        # 获取输出目标
        output_target = self.output_target.get()
        
        if mode == "xml":
            # XML结构对比
            if output_target == "ui":
                self.result_text.insert(tk.END, f"正在解析文件1: {os.path.basename(file1)}\n")
                self.root.update_idletasks()
            else:
                print(f"正在解析文件1: {os.path.basename(file1)}")
            
            data1 = self.parse_kml(file1)
            if not data1:
                return
            
            if output_target == "ui":
                self.result_text.insert(tk.END, f"正在解析文件2: {os.path.basename(file2)}\n")
                self.root.update_idletasks()
            else:
                print(f"正在解析文件2: {os.path.basename(file2)}")
            
            data2 = self.parse_kml(file2)
            if not data2:
                return
            
            if output_target == "ui":
                self.result_text.insert(tk.END, "\n开始对比文件...\n")
                self.result_text.insert(tk.END, "-" * 80 + "\n")
                self.root.update_idletasks()
            else:
                print("\n开始对比文件...")
                print("-" * 80)
            
            differences = []
            self.compare_data(data1, data2, "根节点", differences)
            
            # 生成结果
            result_lines = []
            if differences:
                total_diff = len(differences)
                result_lines.append(f"发现 {total_diff} 处差异:\n")
                
                # 只显示前100处差异
                display_diff = differences[:100]
                for diff in display_diff:
                    result_lines.append(f"{diff}\n")
                
                if total_diff > 100:
                    result_lines.append(f"\n... 省略了 {total_diff - 100} 处差异 ...\n")
                
                result_lines.append("\n结论: 两个KML文件不一致\n")
            else:
                result_lines.append("结论: 两个KML文件一致\n")
            
            # 输出结果
            result_text = ''.join(result_lines)
            if output_target == "ui":
                self.result_text.insert(tk.END, result_text)
            else:
                print(result_text)
        else:
            # 文本对比（纯文本或详细）
            if output_target == "ui":
                self.result_text.insert(tk.END, f"正在读取文件1: {os.path.basename(file1)}\n")
                self.root.update_idletasks()
            else:
                print(f"正在读取文件1: {os.path.basename(file1)}")
            
            try:
                # 尝试使用不同编码读取文件1
                encodings = ['utf-8-sig', 'gbk', 'utf-16', 'utf-8']
                content1 = None
                
                for encoding in encodings:
                    try:
                        with open(file1, 'r', encoding=encoding) as f:
                            content1 = f.readlines()
                        break
                    except UnicodeDecodeError:
                        continue
                
                # 如果所有编码都失败，使用latin-1
                if content1 is None:
                    with open(file1, 'r', encoding='latin-1') as f:
                        content1 = f.readlines()
                
                if output_target == "ui":
                    self.result_text.insert(tk.END, f"正在读取文件2: {os.path.basename(file2)}\n")
                    self.root.update_idletasks()
                else:
                    print(f"正在读取文件2: {os.path.basename(file2)}")
                
                # 尝试使用不同编码读取文件2
                encodings = ['utf-8-sig', 'gbk', 'utf-16', 'utf-8']
                content2 = None
                
                for encoding in encodings:
                    try:
                        with open(file2, 'r', encoding=encoding) as f:
                            content2 = f.readlines()
                        break
                    except UnicodeDecodeError:
                        continue
                
                # 如果所有编码都失败，使用latin-1
                if content2 is None:
                    with open(file2, 'r', encoding='latin-1') as f:
                        content2 = f.readlines()
                
                if output_target == "ui":
                    self.result_text.insert(tk.END, "\n开始对比文件...\n")
                    self.result_text.insert(tk.END, "-" * 80 + "\n")
                    self.root.update_idletasks()
                else:
                    print("\n开始对比文件...")
                    print("-" * 80)
                
                differences = []
                max_lines = max(len(content1), len(content2))
                
                for i in range(max_lines):
                    if i >= len(content1):
                        # 文件1结束，文件2还有内容
                        differences.append(f"文件1缺少行 {i+1}: {content2[i].rstrip()}")
                    elif i >= len(content2):
                        # 文件2结束，文件1还有内容
                        differences.append(f"文件2缺少行 {i+1}: {content1[i].rstrip()}")
                    else:
                        # 处理各种格式差异
                        line1 = content1[i]
                        line2 = content2[i]
                        
                        # 标准化处理：
                        # 1. 统一行尾格式为LF
                        line1_norm = line1.replace('\r\n', '\n').replace('\r', '\n')
                        line2_norm = line2.replace('\r\n', '\n').replace('\r', '\n')
                        
                        # 2. 去除行尾空格
                        line1_norm = line1_norm.rstrip()
                        line2_norm = line2_norm.rstrip()
                        
                        # 3. 统一空格类型（将不间断空格等转换为普通空格）
                        line1_norm = line1_norm.replace('\xc2\xa0', ' ')
                        line2_norm = line2_norm.replace('\xc2\xa0', ' ')
                        
                        # 4. 去除多余空格
                        line1_norm = ' '.join(line1_norm.split())
                        line2_norm = ' '.join(line2_norm.split())
                        
                        # 比较标准化后的内容
                        if line1_norm != line2_norm:
                            # 两行内容不同
                            if mode == "detailed":
                                # 详细对比，显示隐藏字符
                                differences.append(f"行 {i+1} 不同:")
                                differences.append(f"  文件1原始: {self.escape_special_chars(line1)}")
                                differences.append(f"  文件2原始: {self.escape_special_chars(line2)}")
                                differences.append(f"  文件1标准化: {line1_norm}")
                                differences.append(f"  文件2标准化: {line2_norm}")
                            else:
                                # 纯文本对比
                                differences.append(f"行 {i+1} 不同:")
                                differences.append(f"  文件1: {line1.rstrip()}")
                                differences.append(f"  文件2: {line2.rstrip()}")
                
                # 生成结果
                result_lines = []
                if differences:
                    total_diff = len(differences)
                    result_lines.append(f"发现 {total_diff} 处差异:\n")
                    
                    # 只显示前100处差异
                    display_diff = differences[:100]
                    for diff in display_diff:
                        result_lines.append(f"{diff}\n")
                    
                    if total_diff > 100:
                        result_lines.append(f"\n... 省略了 {total_diff - 100} 处差异 ...\n")
                    
                    result_lines.append("\n结论: 两个文件不一致\n")
                else:
                    result_lines.append("结论: 两个文件一致\n")
                
                # 输出结果
                result_text = ''.join(result_lines)
                if output_target == "ui":
                    self.result_text.insert(tk.END, result_text)
                else:
                    print(result_text)
                
            except Exception as e:
                if output_target == "ui":
                    messagebox.showerror("错误", f"读取文件时出错: {str(e)}")
                else:
                    print(f"错误: 读取文件时出错: {str(e)}")
    
    def compare_data(self, data1, data2, path, differences):
        """递归对比数据"""
        # 检查数据类型
        if type(data1) != type(data2):
            differences.append(f"类型不同: {path} - 文件1是{type(data1).__name__}, 文件2是{type(data2).__name__}")
            return
        
        # 对比字典
        if isinstance(data1, dict):
            # 检查键是否相同
            keys1 = set(data1.keys())
            keys2 = set(data2.keys())
            
            # 检查文件1有但文件2没有的键
            for key in keys1 - keys2:
                differences.append(f"文件1有但文件2没有: {path}.{key}")
            
            # 检查文件2有但文件1没有的键
            for key in keys2 - keys1:
                differences.append(f"文件2有但文件1没有: {path}.{key}")
            
            # 对比共同键的值
            for key in keys1 & keys2:
                new_path = f"{path}.{key}"
                self.compare_data(data1[key], data2[key], new_path, differences)
        
        # 对比列表
        elif isinstance(data1, list):
            # 检查长度
            if len(data1) != len(data2):
                differences.append(f"长度不同: {path} - 文件1有{len(data1)}个元素, 文件2有{len(data2)}个元素")
            
            # 对比对应元素
            min_len = min(len(data1), len(data2))
            for i in range(min_len):
                new_path = f"{path}[{i}]"
                self.compare_data(data1[i], data2[i], new_path, differences)
        
        # 对比其他类型
        else:
            if data1 != data2:
                differences.append(f"值不同: {path} - 文件1: {data1}, 文件2: {data2}")

if __name__ == "__main__":
    root = tk.Tk()
    app = KMLComparator(root)
    root.mainloop()
