from PyQt5.QtWidgets import QAction
from PyQt5.QtGui import QIcon


class StopAction(QAction):

    def __init__(self, scripter, toolbar, parent=None):
        super(StopAction, self).__init__(parent)
        self.scripter = scripter
        self.toolbar = toolbar

        self.triggered.connect(self.stop)

        self.setText('Stop')
        # path to the icon
        self.setIcon(QIcon('/home/eliakincosta/Pictures/stop.svg'))

    def stop(self):
        self.scripter.debugcontroller.stop()
        self.toolbar.disableToolbar(True)
