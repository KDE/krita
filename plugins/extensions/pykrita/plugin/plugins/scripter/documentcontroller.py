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

    def saveDocument(self, data, filePath):
        if not self._activeDocument:
            self._activeDocument = document.Document(filePath)

        text = str(data)

        if not self._activeDocument.compare(text):
            self._activeDocument.data = text
            self._activeDocument.save()

        return self._activeDocument

    def clearActiveDocument(self):
        self._activeDocument = None
