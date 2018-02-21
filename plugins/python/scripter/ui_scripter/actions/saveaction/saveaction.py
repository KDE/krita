from PyQt5.QtWidgets import QAction, QFileDialog
from PyQt5.QtGui import QKeySequence
from PyQt5.QtCore import Qt


class SaveAction(QAction):

    def __init__(self, scripter, parent=None):
        super(SaveAction, self).__init__(parent)
        self.scripter = scripter
        self.editor = self.scripter.uicontroller.editor

        self.triggered.connect(self.save)

        self.setText('Save')
        self.setObjectName('save')
        self.setShortcut(QKeySequence(Qt.CTRL + Qt.Key_S))

    @property
    def parent(self):
        return 'File'

    def save(self):
        text = self.editor.toPlainText()
        fileName = ''

        if not self.scripter.documentcontroller.activeDocument:
            fileName = QFileDialog.getSaveFileName(self.scripter.uicontroller.mainWidget,
                                                   'Save Python File', '',
                                                   'Python File (*.py)')[0]
            if not fileName:
                return

            # don't validate file name - trust user to specify the extension they want
            # getSaveFileName will add ".py" if there is no extension. 
            # It will strip a trailing period and, in each case,  test for file collisions 

        document = self.scripter.documentcontroller.saveDocument(text, fileName)
        if document:
            self.scripter.uicontroller.setStatusBar(document.filePath)
        else:
            self.scripter.uicontroller.setStatusBar('untitled')
        self.editor._documentModified = False
        self.scripter.uicontroller.setStatusModified()
        return document
