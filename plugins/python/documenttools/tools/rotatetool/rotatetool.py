from PyQt5.QtWidgets import (QWidget, QSpinBox, QHBoxLayout,
                             QVBoxLayout, QFormLayout)
import math


class RotateTool(QWidget):

    def __init__(self, mainDialog, parent=None):
        super(RotateTool, self).__init__(parent)

        self.setObjectName("Rotate")

        self.layout = QFormLayout()

        self.degreesSpinBox = QSpinBox()

        self.setLayout(self.layout)
        self.initialize()

    def initialize(self):
        self.degreesSpinBox.setRange(-180, 180)
        self.degreesSpinBox.setToolTip("Negative degrees will rotate the image to the left")

        self.layout.addRow('Degrees', self.degreesSpinBox)

    def adjust(self, documents):
        for document in documents:
            document.rotateImage(math.radians(self.degreesSpinBox.value()))
