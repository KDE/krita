from scripter.document_scripter import document


class DocumentController(object):

    def __init__(self):
        self._activeDocument = None

    @property
    def activeDocument(self):
        return self._activeDocument

    def openDocument(self, filePath):
        if filePath:
            newDocument = document.Document(filePath)
            newDocument.open()
            self._activeDocument = newDocument
            return newDocument
