from PyQt5.QtCore import QAbstractListModel, Qt
import krita


class LastDocumentsListModel(QAbstractListModel):

    def __init__(self, parent=None):
        super(LastDocumentsListModel, self).__init__(parent)

        self.rootItem = ('Path',)
        self.kritaInstance = krita.Krita.instance()
        self.recentDocuments = []

        self._loadRecentDocuments()

    def data(self, index, role):
        if not index.isValid():
            return None

        if index.row() >= len(self.recentDocuments):
            return None

        if role == Qt.DecorationRole:
            return self.recentDocuments[index.row()]
        else:
            return None

    def rowCount(self, parent):
        return len(self.recentDocuments)

    def headerData(self, section, orientation, role):
        if orientation == Qt.Horizontal and role == Qt.DisplayRole:
            return self.rootItem[section]

        return None

    def _loadRecentDocuments(self):
        recentDocumentsPaths = self.kritaInstance.recentDocuments()

        #for path in recentDocumentsPaths:
        #    document = self.kritaInstance.openDocument(path)
        #    if document:
        #        self.recentDocuments.append(document.thumbnail(70, 60))
        #        document.close()
