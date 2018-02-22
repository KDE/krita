from PyQt5.QtWidgets import QComboBox
import krita


class FilterComboBox(QComboBox):

    def __init__(self, uiFilterManager, parent=None):
        super(FilterComboBox, self).__init__(parent)

        self.uiFilterManager = uiFilterManager

        self.addItems(self.uiFilterManager.filters)
