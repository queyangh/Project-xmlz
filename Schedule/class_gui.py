import sys
import pandas as pd
import matplotlib.pyplot as plt
from PyQt5.QtWidgets import (QApplication, QWidget, QVBoxLayout, QLabel, QLineEdit,
                             QPushButton, QComboBox, QTextEdit, QMessageBox, QFileDialog)
from PyQt5.QtGui import QPixmap
from PyQt5.QtCore import QByteArray
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
import io

class CourseQueryApp(QWidget):
    def __init__(self):
        super().__init__()
        self.excel_file_path = ""
        self.result_image = None  # 存储图片的内存
        self.init_ui()

    def init_ui(self):
        self.setWindowTitle("课程查询")
        self.setGeometry(100, 100, 800, 600)

        layout = QVBoxLayout()

        self.btn_select_file = QPushButton("选择 Excel 文件")
        self.btn_select_file.clicked.connect(self.select_excel_file)
        layout.addWidget(self.btn_select_file)

        self.label_status = QLabel("尚未加载文件")
        layout.addWidget(self.label_status)

        self.entry_teacher = QLineEdit()
        self.entry_teacher.setPlaceholderText("教师姓名")
        layout.addWidget(self.entry_teacher)

        self.campus_combo = QComboBox()
        self.campus_combo.addItems(["东渡", "同安"])
        layout.addWidget(QLabel("选择校区:"))
        layout.addWidget(self.campus_combo)

        self.grade_combo = QComboBox()
        self.grade_combo.addItems(["高一", "高二", "高三"])
        layout.addWidget(QLabel("选择年级:"))
        layout.addWidget(self.grade_combo)

        self.subject1_combo = QComboBox()
        self.subject2_combo = QComboBox()
        subjects = ["", "语文", "数学", "英语", "物理", "化学", "生物", "政治", "历史", "地理",
                    "体育", "音鉴", "通用", "信息", "心理", "劳动", "美术"]
        self.subject1_combo.addItems(subjects)
        self.subject2_combo.addItems(subjects)

        layout.addWidget(QLabel("所教科目1:"))
        layout.addWidget(self.subject1_combo)
        layout.addWidget(QLabel("所教科目2:"))
        layout.addWidget(self.subject2_combo)

        self.btn_search = QPushButton("查询")
        self.btn_search.clicked.connect(self.search_classes)
        layout.addWidget(self.btn_search)

        self.text_output = QTextEdit()
        self.text_output.setReadOnly(True)
        layout.addWidget(self.text_output)

        self.result_image_label = QLabel()
        layout.addWidget(self.result_image_label)

        self.btn_download = QPushButton("下载表格图片")
        self.btn_download.clicked.connect(self.download_image)
        layout.addWidget(self.btn_download)

        self.setLayout(layout)

    def select_excel_file(self):
        self.excel_file_path, _ = QFileDialog.getOpenFileName(self, "选择 Excel 文件", "", "Excel Files (*.xls *.xlsx)")
        if self.excel_file_path:
            self.label_status.setText("加载成功！")

    def get_sheet_names(self, campus, grade):
        excel_file = pd.ExcelFile(self.excel_file_path)
        return [sheet for sheet in excel_file.sheet_names if campus in sheet and grade in sheet]

    def search_classes(self):
        try:
            teacher_name = self.entry_teacher.text().strip()
            subject1 = self.subject1_combo.currentText()
            subject2 = self.subject2_combo.currentText()

            if not self.excel_file_path:
                QMessageBox.warning(self, "错误", "请先选择 Excel 文件！")
                return

            campus = self.campus_combo.currentText()
            grade = self.grade_combo.currentText()
            sheet_names = self.get_sheet_names(campus, grade)

            if not sheet_names:
                QMessageBox.warning(self, "错误", "未找到对应的工作表！")
                return

            df = pd.read_excel(self.excel_file_path, sheet_name=sheet_names[0])
            range_df = df.iloc[14:35, 12:69]
            search_range = df.iloc[0:11, 0:106].fillna(method='ffill', axis=1)

            list1, list2, list3 = [], [], []

            if not teacher_name:
                QMessageBox.warning(self, "输入错误", "请填写教师姓名！")
                return

            matching_rows = range_df[range_df.isin([teacher_name]).any(axis=1)]
            first_column_data = matching_rows.iloc[:, 0].dropna()
            trimmed_data = [str(item)[:-1] for item in first_column_data]

            for value in trimmed_data:
                value_str = str(value)
                matching_cells = search_range[search_range.astype(str).isin([value_str])]

                if not matching_cells.empty:
                    for (row, col) in matching_cells.stack().index:
                        col_number = int(col.split(': ')[-1])
                        column_data = search_range.iloc[:, col_number]

                        for index, cell_value in enumerate(column_data):
                            cell_value_str = str(cell_value).strip()

                            if subject1 and subject2:
                                if (cell_value_str == subject1) or (cell_value_str == subject2):
                                    list1.append(search_range.iloc[index, 0])
                                    list2.append(search_range.iloc[0, col_number])
                                    list3.append(search_range.iloc[1, col_number])
                                    break
                            elif subject1:
                                if cell_value_str == subject1:
                                    list1.append(search_range.iloc[index, 0])
                                    list2.append(search_range.iloc[0, col_number])
                                    list3.append(search_range.iloc[1, col_number])
                                    break
                            elif subject2:
                                if cell_value_str == subject2:
                                    list1.append(search_range.iloc[index, 0])
                                    list2.append(search_range.iloc[0, col_number])
                                    list3.append(search_range.iloc[1, col_number])
                                    break

            list2_cleaned = [item.replace(' ', '') for item in list2]

            self.text_output.clear()
            self.create_result_table(list1, list2_cleaned, list3)
        except Exception as e:
            QMessageBox.critical(self, "错误", f"发生错误: {str(e)}")

    def create_result_table(self, list1, list2, list3):
        table_data = {day: [""] * 7 for day in ["星期一", "星期二", "星期三", "星期四", "星期五"]}

        for i in range(len(list1)):
            if list1[i] and list2[i] and list3[i] is not None:
                row_index = int(list1[i][1]) - 1  # "第X节" -> X-1
                col_index = ["星期一", "星期二", "星期三", "星期四", "星期五"].index(list2[i])
                table_data[list2[i]][row_index] = f"{list3[i]}班"

        self.plot_table(table_data)

    def plot_table(self, table_data):
        fig, ax = plt.subplots(figsize=(8, 6))  # 调整整体图形的大小
        ax.axis('tight')
        ax.axis('off')

        # 添加表头
        teacher_name = self.entry_teacher.text().strip()
        title = f"2024-2025学年第一学期 {teacher_name} 老师课表"
        ax.text(0.5, 1.1, title, ha='center', va='center', fontsize=16, fontweight='bold', transform=ax.transAxes)

        cell_text = []
        for i in range(7):  # 7节
            row = []
            for day in ["星期一", "星期二", "星期三", "星期四", "星期五"]:
                row.append(table_data[day][i])
            cell_text.append(row)

        the_table = ax.table(cellText=cell_text,
                             colLabels=["星期一", "星期二", "星期三", "星期四", "星期五"],
                             rowLabels=[f"第{i + 1}节" for i in range(7)],
                             cellLoc='center', loc='center')

        # 设置行高
        for i in range(len(cell_text)):
            for j in range(len(cell_text[i])):
                the_table[(i + 1, j)].set_height(0.1)  # 可以根据需要调整高度

        # 设置中文字体
        plt.rcParams['font.sans-serif'] = ['SimHei']
        plt.rcParams['axes.unicode_minus'] = False

        # 使用 BytesIO 存储图像数据
        buf = io.BytesIO()
        plt.savefig(buf, format='jpg', bbox_inches='tight', pad_inches=0.1)  # 调整边距
        plt.close(fig)
        buf.seek(0)
        self.result_image = QPixmap()
        self.result_image.loadFromData(buf.getvalue())

        # 显示图片
        self.result_image_label.setPixmap(self.result_image)

    def download_image(self):
        if self.result_image:
            options = QFileDialog.Options()
            file_name, _ = QFileDialog.getSaveFileName(self, "保存图片", "", "Image Files (*.jpg *.jpeg);;All Files (*)", options=options)
            if file_name:
                self.result_image.save(file_name, "JPG")
        else:
            QMessageBox.warning(self, "错误", "没有可下载的图片！")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = CourseQueryApp()
    window.show()
    sys.exit(app.exec_())
