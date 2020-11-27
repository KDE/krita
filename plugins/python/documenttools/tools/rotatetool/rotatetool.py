# SPDX-License-Identifier: CC0-1.0

from PyQt5.QtWidgets import QWidget, QSpinBox, QFormLayout
import math


class RotateTool(QWidget):

    def __init__(self, mainDialog, parent=None):
        super(RotateTool, self).__init__(parent)

        self.setObjectName(i18n("Rotate"))

        self.layout = QFormLayout()

        self.degreesSpinBox = QSpinBox()

        self.setLayout(self.layout)
        self.initialize()

    def initialize(self):
        self.degreesSpinBox.setRange(-180, 180)
        self.degreesSpinBox.setToolTip(
            i18n("Negative degrees will rotate the image to the left"))

        self.layout.addRow(i18n("Degrees:"), self.degreesSpinBox)

    def adjust(self, documents):
        for document in documents:
            document.rotateImage(math.radians(self.degreesSpinBox.value()))
