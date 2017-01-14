from PyQt5.QtWidgets import QAction
from PyQt5.QtGui import QIcon


class StepAction(QAction):

    def __init__(self, scripter, toolbar, parent=None):
        super(StepAction, self).__init__(parent)
        self.scripter = scripter
        self.toolbar = toolbar

        self.triggered.connect(self.step)

        self.setText('Step Over')
        # path to the icon
        #self.setIcon(QIcon('/home/eliakincosta/Pictures/step.svg'))

    def step(self):
        status = self.scripter.debugcontroller.isActive
        if status:
            self.scripter.debugcontroller.step()
        else:
            self.toolbar.disableToolbar(True)
