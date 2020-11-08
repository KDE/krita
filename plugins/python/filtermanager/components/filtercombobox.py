# SPDX-License-Identifier: CC0-1.0

from PyQt5.QtWidgets import QComboBox


class FilterComboBox(QComboBox):

    def __init__(self, uiFilterManager, parent=None):
        super(FilterComboBox, self).__init__(parent)

        self.uiFilterManager = uiFilterManager

        self.addItems(self.uiFilterManager.filters)
