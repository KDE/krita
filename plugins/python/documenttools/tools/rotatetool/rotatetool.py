'''
This script is licensed CC 0 1.0, so that you can learn from it.

------ CC 0 1.0 ---------------

The person who associated a work with this deed has dedicated the work to the public domain by waiving all of his or her rights to the work worldwide under copyright law, including all related and neighboring rights, to the extent allowed by law.

You can copy, modify, distribute and perform the work, even for commercial purposes, all without asking permission.

https://creativecommons.org/publicdomain/zero/1.0/legalcode
'''
from PyQt5.QtWidgets import (QWidget, QSpinBox, QHBoxLayout,
                             QVBoxLayout, QFormLayout)
import math
import krita


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
        self.degreesSpinBox.setToolTip(i18n("Negative degrees will rotate the image to the left"))

        self.layout.addRow(i18n("Degrees:"), self.degreesSpinBox)

    def adjust(self, documents):
        for document in documents:
            document.rotateImage(math.radians(self.degreesSpinBox.value()))
