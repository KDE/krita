from PyQt5.QtWidgets import QPushButton
from PyQt5.QtGui import QPixmap


class DropButton(QPushButton):

    def __init__(self, parent):
        super(DropButton, self).__init__(parent)

        self.preset = None
        self.setFixedSize(64, 64)
        self.setIconSize(QSize(64, 64))


    def selectPreset(self):
        self.preset = self.presetChooser.currentPreset().name()
        self.setIcon(QIcon(QPixmap.fromImage(self.presetChooser.currentPreset().image())))
