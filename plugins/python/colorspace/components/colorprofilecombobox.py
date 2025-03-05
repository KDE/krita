# SPDX-License-Identifier: CC0-1.0

try:
    from PyQt6.QtWidgets import QComboBox
except:
    from PyQt5.QtWidgets import QComboBox


class ColorProfileComboBox(QComboBox):

    def __init__(self, uiColorSpace, parent=None):
        super(ColorProfileComboBox, self).__init__(parent)

        self.uiColorSpace = uiColorSpace
        self.setSizeAdjustPolicy(QComboBox.SizeAdjustPolicy.AdjustToContents)
