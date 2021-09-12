# SPDX-License-Identifier: CC0-1.0

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
        self.layout.addRow(i18nc("Resize intepolation method list label", "Filter:"), self.strategyComboBox)

    def adjust(self, documents):
        for document in documents:
            document.scaleImage(self.widthSpinBox.value(),
                                self.heightSpinBox.value(),
                                self.xResSpinBox.value(),
                                self.yResSpinBox.value(),
                                self.strategyComboBox.currentText())
