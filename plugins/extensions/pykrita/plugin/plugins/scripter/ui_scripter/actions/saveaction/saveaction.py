from PyQt5.QtWidgets import QAction, QFileDialog, QMessageBox
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
        fileExtension = ''

        if not self.scripter.documentcontroller.activeDocument:
            try:
                fileName = QFileDialog.getSaveFileName(self.scripter.uicontroller.mainWidget,
                                                       'Save Python File', '',
                                                       'Python File (*.py)')[0]
                if not fileName:
                    return

                fileExtension = fileName.rsplit('.', maxsplit=1)[1]
            except:
                if not fileExtension == 'py':
                    QMessageBox.information(self.scripter.uicontroller.mainWidget,
                                            'Invalid File',
                                            'Save files with .py extension')
                return

        document = self.scripter.documentcontroller.saveDocument(text, fileName)
        if document:
            self.scripter.uicontroller.setStatusBar(document.filePath)
        else:
            self.scripter.uicontroller.setStatusBar('untitled')

        return document
