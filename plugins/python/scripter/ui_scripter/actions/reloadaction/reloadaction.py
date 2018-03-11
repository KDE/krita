from PyQt5.QtWidgets import QAction, QFileDialog, QMessageBox
from PyQt5.QtGui import QKeySequence
from PyQt5.QtCore import Qt


class ReloadAction(QAction):

    def __init__(self, scripter, parent=None):
        super(ReloadAction, self).__init__(parent)
        self.scripter = scripter
        self.editor = self.scripter.uicontroller.editor

        self.triggered.connect(self.reloadFile)

        self.setText('Reload File')
        self.setObjectName('reloadfile')
        self.setShortcut(QKeySequence(Qt.ALT + Qt.Key_R))
        self.setIcon(Krita.instance().icon("view-refresh"))        

    @property
    def parents(self):
        return ('File', 'toolBar')

    def reloadFile(self):
        # get the currently open document's path
        curr_doc_fpath = ''
        document = self.scripter.documentcontroller._activeDocument
        if document is None:
            QMessageBox.critical(self.scripter.uicontroller.mainWidget,
                                 'No existing document',
                                 'Please specify a document by opening it before reloading')
            return
        else:
            curr_doc_fpath = document.filePath

        # clear the editor
        self.scripter.documentcontroller.clearActiveDocument()
        self.scripter.uicontroller.setStatusBar()
        self.scripter.uicontroller.clearEditor()

        # reload the document
        document = self.scripter.documentcontroller.openDocument(curr_doc_fpath)
        self.scripter.uicontroller.setDocumentEditor(document)
        self.scripter.uicontroller.setStatusBar(document.filePath)
        print("reload is run")

        return document
