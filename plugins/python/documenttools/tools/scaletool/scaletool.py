/# This script is licensed CC 0 1.0, so that you can learn from it.

# ------ CC 0 1.0 ---------------

# The person who associated a work with this deed has dedicated the
# work to the public domain by waiving all of his or her rights to the
# work worldwide under copyright law, including all related and
# neighboring rights, to the extent allowed by law.

# You can copy, modify, distribute and perform the work, even for
# commercial purposes, all without asking permission.

# https://creativecommons.org/publicdomain/zero/1.0/legalcode

from PyQt5.QtWidgets import (QWidget, QSpinBox,
                             QVBoxLayout, QFormLayout, QComboBox)


class ScaleTool(QWidget):

    def __init__(self, mainDialog, parent=None):
        super(ScaleTool, self).__init__(parent)

        self.setObjectName(i18n("Scale"))

        self.layout = QFormLayout()
        self.resolutionLayout = QVBoxLayout()
        self.widthSpinBox = QSpinBox()
        self.heightSpinBox = QSpinBox()
        self.xResSpinBox = QSpinBox()
        self.yResSpinBox = QSpinBox()
        self.strategyComboBox = QComboBox()

        self.strategyComboBox.setSizeAdjustPolicy(QComboBox.AdjustToContents)
        self.setLayout(self.layout)
        self.initialize()

    def initialize(self):
        self.widthSpinBox.setRange(1, 10000)
        self.heightSpinBox.setRange(1, 10000)
        self.xResSpinBox.setRange(1, 10000)
        self.yResSpinBox.setRange(1, 10000)

        strategies = ['Hermite', 'Bicubic', 'Box',
                      'Bilinear', 'Bell', 'BSpline',
                      'Kanczos3', 'Mitchell']
        self.strategyComboBox.addItems(strategies)

        self.resolutionLayout.addWidget(self.xResSpinBox)
        self.resolutionLayout.addWidget(self.yResSpinBox)

        self.layout.addRow(i18n("Width:"), self.widthSpinBox)
        self.layout.addRow(i18n("Height:"), self.heightSpinBox)
        self.layout.addRow(i18n("Resolution:"), self.resolutionLayout)
        self.layout.addRow(i18n("Filter:"), self.strategyComboBox)

    def adjust(self, documents):
        for document in documents:
            document.scaleImage(self.widthSpinBox.value(),
                                self.heightSpinBox.value(),
                                self.xResSpinBox.value(),
                                self.yResSpinBox.value(),
                                self.strategyComboBox.currentText())
