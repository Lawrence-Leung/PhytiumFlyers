from PySide6.QtWidgets import QLabel,QDialog, QVBoxLayout,QComboBox, QPushButton
from PySide6.QtCore import Qt
class CameraSelectionDialog(QDialog):
    def __init__(self, cams, parent=None):
        super().__init__(parent)
        self.setWindowTitle('Select Camera')
        layout = QVBoxLayout(self)
        
        self.label = QLabel('Select camera:', self)
        layout.addWidget(self.label)

        self.comboBox = QComboBox(self)
        layout.addWidget(self.comboBox)

        for cam in cams:
            self.comboBox.addItem(str(cam))  # 将设备编号作为下拉列表项添加
        self.okButton = QPushButton('OK', self)
        self.okButton.clicked.connect(self.accept)
        layout.addWidget(self.okButton)

        self.setLayout(layout)
        self.setWindowFlag(Qt.WindowContextHelpButtonHint, False)

    def selected_camera(self):
        return self.comboBox.currentText()
    