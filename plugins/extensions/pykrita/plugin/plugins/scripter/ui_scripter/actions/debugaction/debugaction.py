from PyQt5.QtWidgets import QAction
from PyQt5.QtGui import QIcon, QPixmap


class DebugAction(QAction):

    def __init__(self, scripter, parent=None):
        super(DebugAction, self).__init__(parent)
        self.scripter = scripter

        self.triggered.connect(self.debug)

        self.setText('Debug')
        # path to the icon
        self.setIcon(QIcon('/home/eliakincosta/Pictures/debug.svg'))

    @property
    def parent(self):
        return 'toolBar'

    def debug(self):
        if self.scripter.uicontroller.invokeAction('save'):
            self.scripter.uicontroller.setActiveWidget('Debugger')
            self.scripter.debugcontroller.start(self.scripter.documentcontroller.activeDocument)
            widget = self.scripter.uicontroller.findStackWidget('Debugger')
            widget.startDebugger()
