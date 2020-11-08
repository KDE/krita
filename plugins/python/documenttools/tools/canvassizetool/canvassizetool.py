# SPDX-License-Identifier: CC0-1.0

from PyQt5.QtWidgets import (QWidget, QSpinBox,
                             QVBoxLayout, QFormLayout)


class CanvasSizeTool(QWidget):

    def __init__(self, mainDialog, parent=None):
        super(CanvasSizeTool, self).__init__(parent)

        self.setObjectName(i18n("Canvas Size"))

        self.layout = QFormLayout()
        self.offsetLayout = QVBoxLayout()

        self.widthSpinBox = QSpinBox()
        self.heightSpinBox = QSpinBox()
        self.xOffsetSpinBox = QSpinBox()
        self.yOffsetSpinBox = QSpinBox()

        self.setLayout(self.layout)
        self.initialize()

    def initialize(self):
        self.widthSpinBox.setRange(1, 10000)
        self.heightSpinBox.setRange(1, 10000)
        self.xOffsetSpinBox.setRange(-10000, 10000)
        self.yOffsetSpinBox.setRange(-10000, 10000)

        self.offsetLayout.addWidget(self.xOffsetSpinBox)
        self.offsetLayout.addWidget(self.yOffsetSpinBox)

        self.layout.addRow(i18n("Width:"), self.widthSpinBox)
        self.layout.addRow(i18n("Height:"), self.heightSpinBox)
        self.layout.addRow(i18n("Offset:"), self.offsetLayout)

    def adjust(self, documents):
        for document in documents:
            document.resizeImage(self.xOffsetSpinBox.value(),
                                 self.yOffsetSpinBox.value(),
                                 self.widthSpinBox.value(),
                                 self.heightSpinBox.value())
