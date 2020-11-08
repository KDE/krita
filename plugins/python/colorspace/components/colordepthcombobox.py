# SPDX-License-Identifier: CC0-1.0

from PyQt5.QtWidgets import QComboBox


class ColorDepthComboBox(QComboBox):

    def __init__(self, uiColorSpace, parent=None):
        super(ColorDepthComboBox, self).__init__(parent)

        self.uiColorSpace = uiColorSpace

        self.currentTextChanged.connect(self.changedTextColorDepthComboBox)

    def changedTextColorDepthComboBox(self, colorDepth):
        self.uiColorSpace.loadColorProfiles()
