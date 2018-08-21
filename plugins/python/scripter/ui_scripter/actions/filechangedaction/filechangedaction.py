from PyQt5.QtWidgets import QAction, QMessageBox
from PyQt5.QtGui import QKeySequence
from PyQt5.QtCore import Qt, QFileInfo
from scripter.document_scripter import document
import krita


class FileChangedAction(QAction):

    def __init__(self, scripter, parent=None):
        super(FileChangedAction, self).__init__(parent)

        self.scripter = scripter
        self.editor = self.scripter.uicontroller.editor

        self.scripter.documentcontroller.fileWatcher.fileChanged.connect(self.fileChanged)

        self.setObjectName('filechanged')

    @property
    def parents(self):
        return ()

    def fileChanged(self, path):
        fileSystemDocument = document.Document(path)
        fileSystemDocument.open()

        if not self.scripter.documentcontroller.activeDocument.compare(fileSystemDocument.data):
            msgBox = QMessageBox(self.scripter.uicontroller.mainWidget)

            msgBox.setText(i18n("The file {0} was modified by another program.".format(path)))

            reloadButton = msgBox.addButton(i18n("Reload"), QMessageBox.ActionRole)
            overwriteButton = msgBox.addButton(i18n("Overwrite"), QMessageBox.ActionRole)
            ignoreChangesButton = msgBox.addButton(i18n("Ignore"), QMessageBox.ActionRole)

            ret = msgBox.exec_()

            if (msgBox.clickedButton() == reloadButton):
                self.scripter.uicontroller.setDocumentEditor(fileSystemDocument)
            elif (msgBox.clickedButton() == overwriteButton):
                self.scripter.documentcontroller.saveDocument(self.editor.document().toPlainText(), path)
            elif (msgBox.clickedButton() == ignoreChangesButton):
                self.scripter.documentcontroller.fileWatcher.removePath(path)
                self.scripter.documentcontroller.fileModifiedOtherProgram = True

            """ some programs delete the original file and create a new one with
                the current content, for that reason it's necessary adding the
                path again to the file watcher """
            if QFileInfo(path).exists() and not msgBox.clickedButton() == ignoreChangesButton:
                self.scripter.documentcontroller.fileWatcher.addPath(path)
