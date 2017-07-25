from PyQt5.QtCore import QAbstractListModel, Qt
import krita


class LastDocumentsListModel(QAbstractListModel):

    def __init__(self, parent=None):
        super(LastDocumentsListModel, self).__init__(parent)

        self.rootItem = ('Path',)
        self.recentDocuments = []

        self._loadRecentDocuments()

    def data(self, index, role):
        if not index.isValid():
            return None

        if index.row() >= len(self.recentDocuments):
            return None

        if role == Qt.DisplayRole:
            return self.recentDocuments[index.row()].fileName()
        else:
            return None

    def rowCount(self, parent):
        return len(self.recentDocuments)

    def headerData(self, section, orientation, role):
        if orientation == Qt.Horizontal and role == Qt.DisplayRole:
            return self.rootItem[section]

        return None

    def _loadRecentDocuments(self):
        pass
