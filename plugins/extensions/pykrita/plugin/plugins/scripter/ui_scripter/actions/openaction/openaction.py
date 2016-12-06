from PyQt5.QtWidgets import QAction


class OpenAction(QAction):

    def __init__(self, scripter, parent=None):
        super(OpenAction, self).__init__(parent)
        self.scripter = scripter

        self.triggered.connect(self.open)

        self.setText('Open')

    @property
    def parent(self):
        return 'File'

    def open(self):
        print('opening file')
