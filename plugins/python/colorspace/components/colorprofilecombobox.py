from PyQt5.QtWidgets import QComboBox


class ColorProfileComboBox(QComboBox):

    def __init__(self, uiColorSpace, parent=None):
        super(ColorProfileComboBox, self).__init__(parent)

        self.uiColorSpace = uiColorSpace
        self.setSizeAdjustPolicy(self.AdjustToContents)
