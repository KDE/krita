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

    def save(self):
        with open(self._filePath, 'w') as pythonFile:
            for line in self._document:
                print(line, file=pythonFile)

    def compare(self, new_doc):
        if len(self._document) != len(new_doc):
            return False

        for line in range(len(new_doc)):
            if new_doc[line] != self._document[line]:
                return False

        return True

    @property
    def data(self):
        return self._document

    @data.setter
    def data(self, data):
        self._document = data

    @property
    def filePath(self):
        return self._filePath
