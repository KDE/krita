from PyQt5.QtCore import QFile, QIODevice, QTextStream


class Document(object):

    def __init__(self, filePath):
        self._document = []
        self._filePath = filePath

    def open(self, filePath=''):
        if filePath:
            self._filePath = filePath
        _file = QFile(self._filePath)

        if _file.open(QIODevice.ReadOnly | QIODevice.Text):
            out = QTextStream(_file)
            while not out.atEnd():
                line = out.readLine()
                self._document.append(line)

        _file.close()

    def data(self):
        return self._document
