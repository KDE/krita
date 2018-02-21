from PyQt5.QtWidgets import QAction, QMessageBox
from PyQt5.QtGui import QKeySequence
from PyQt5.QtCore import Qt


class NewAction(QAction):

    def __init__(self, scripter, parent=None):
        super(NewAction, self).__init__(parent)
        self.scripter = scripter

        self.triggered.connect(self.new)

        self.setText('New')
        self.setObjectName('new')
        self.setShortcut(QKeySequence(Qt.CTRL + Qt.Key_N))

    @property
    def parent(self):
        return 'File'

    def new(self):
        msgBox = QMessageBox(self.scripter.uicontroller.mainWidget)

        msgBox.setText("The document has been modified.")
        msgBox.setInformativeText("Do you want to save your changes?")
        msgBox.setStandardButtons(QMessageBox.Save | QMessageBox.Discard | QMessageBox.Cancel)
        msgBox.setDefaultButton(QMessageBox.Save)

        ret = msgBox.exec()

        if ret == QMessageBox.Cancel:
            return
        if ret == QMessageBox.Save:
            self.scripter.uicontroller.invokeAction('save')

        self.scripter.documentcontroller.clearActiveDocument()
        self.scripter.uicontroller.setStatusBar()
        self.scripter.uicontroller.clearEditor()
