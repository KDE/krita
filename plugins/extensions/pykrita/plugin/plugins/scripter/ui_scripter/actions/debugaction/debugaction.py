from PyQt5.QtWidgets import QAction
from PyQt5.QtGui import QIcon, QPixmap, QKeySequence
from scripter import resources_rc
from PyQt5.QtCore import Qt


class DebugAction(QAction):

    def __init__(self, scripter, parent=None):
        super(DebugAction, self).__init__(parent)
        self.scripter = scripter

        self.triggered.connect(self.debug)

        self.setText('Debug')
        self.setToolTip('Debug Ctrl+D')
        self.setIcon(QIcon(':/icons/debug.svg'))
        self.setShortcut(QKeySequence(Qt.CTRL + Qt.Key_D))

    @property
    def parent(self):
        return 'toolBar'

    def debug(self):
        if self.scripter.uicontroller.invokeAction('save'):
            self.scripter.uicontroller.setActiveWidget('Debugger')
            self.scripter.debugcontroller.start(self.scripter.documentcontroller.activeDocument)
            widget = self.scripter.uicontroller.findStackWidget('Debugger')
            widget.startDebugger()
