from PyQt5.QtWidgets import QAction, QMessageBox
from PyQt5.QtGui import QKeySequence
from PyQt5.QtCore import Qt


class CloseAction(QAction):

    def __init__(self, scripter, parent=None):
        super(CloseAction, self).__init__(parent)
        self.scripter = scripter

        self.triggered.connect(self.close)

        self.setText('Close')
        self.setObjectName('close')
        self.setShortcut(QKeySequence(Qt.CTRL + Qt.Key_Q))

    @property
    def parent(self):
        return 'File'

    def close(self):
        msgBox = QMessageBox(self.scripter.uicontroller.mainWidget)

        msgBox.setInformativeText("Do you want to save the current document?")
        msgBox.setStandardButtons(QMessageBox.Save | QMessageBox.Discard | QMessageBox.Cancel)
        msgBox.setDefaultButton(QMessageBox.Save)

        ret = msgBox.exec()

        if ret == QMessageBox.Cancel:
            return
        if ret == QMessageBox.Save:
            if not self.scripter.uicontroller.invokeAction('save'):
                return

        self.scripter.uicontroller.closeScripter()
