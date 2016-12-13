from PyQt5.QtWidgets import QAction, QFileDialog, QMessageBox


class OpenAction(QAction):

    def __init__(self, scripter, parent=None):
        super(OpenAction, self).__init__(parent)
        self.scripter = scripter

        self.triggered.connect(self.open)

        self.setText('Open')
        self.setObjectName('open')

    @property
    def parent(self):
        return 'File'

    def open(self):
        dialog = QFileDialog(self.scripter.uicontroller.mainWidget)
        dialog.setNameFilter('Python files (*.py)')

        if dialog.exec():
            try:
                selectedFile = dialog.selectedFiles()[0]
                fileExtension = selectedFile.rsplit('.', maxsplit=1)[1]

                if fileExtension=='py':
                    document = self.scripter.documentcontroller.openDocument(selectedFile)
                    self.scripter.uicontroller.setDocumentEditor(document)
                    self.scripter.uicontroller.setStatusBar(document.filePath)

            except:
                QMessageBox.information(self.scripter.uicontroller.mainWidget,
                                        'Invalid File',
                                        'Open files with .py extension')
