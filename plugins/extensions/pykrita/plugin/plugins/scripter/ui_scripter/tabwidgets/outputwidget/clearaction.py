from PyQt5.QtWidgets import QAction
from PyQt5.QtGui import QIcon
from scripter import resources_rc


class ClearAction(QAction):

    def __init__(self, scripter, toolbar, parent=None):
        super(ClearAction, self).__init__(parent)
        self.scripter = scripter
        self.toolbar = toolbar

        self.triggered.connect(self.clear)

        self.setText('Clear')
        # path to the icon
        # self.setIcon(QIcon(':/icons/clear.svg'))

    def clear(self):
        self.toolbar.outputtextedit.clear()
