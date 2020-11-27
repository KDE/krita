# SPDX-License-Identifier: CC0-1.0

from PyQt5.QtWidgets import QComboBox


class ColorModelComboBox(QComboBox):

    def __init__(self, uiColorSpace, parent=None):
        super(ColorModelComboBox, self).__init__(parent)

        self.uiColorSpace = uiColorSpace

        self.currentTextChanged.connect(self.changedTextColorModelComboBox)

    def changedTextColorModelComboBox(self, colorModel):
        self.uiColorSpace.loadColorDepths()
