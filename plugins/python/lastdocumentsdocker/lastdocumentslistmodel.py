# SPDX-License-Identifier: CC0-1.0

from PyQt5.QtCore import QAbstractListModel, Qt, QSize
from PyQt5.QtGui import QImage
import krita
import zipfile
from pathlib import Path


class LastDocumentsListModel(QAbstractListModel):

    def __init__(self, devicePixelRatioF, parent=None):
        super(LastDocumentsListModel, self).__init__(parent)

        self.rootItem = ('Path',)
        self.kritaInstance = krita.Krita.instance()
        self.recentDocuments = []
        self.devicePixelRatioF = devicePixelRatioF

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

    def loadRecentDocuments(self):
        self.recentDocuments = []
        recentDocumentsPaths = self.kritaInstance.recentDocuments()

        for path in recentDocumentsPaths:
            if path:
                thumbnail = None
                extension = Path(path).suffix
                page = None
                if extension == '.kra':
                    page = zipfile.ZipFile(path, "r")
                    thumbnail = QImage.fromData(page.read("mergedimage.png"))
                    if thumbnail.isNull():
                        thumbnail = QImage.fromData(page.read("preview.png"))
                else:
                    thumbnail = QImage(path)

                if thumbnail.isNull():
                    continue

                thumbSize = QSize(200*self.devicePixelRatioF, 150*self.devicePixelRatioF)
                if thumbnail.width() <= thumbSize.width() or thumbnail.height() <= thumbSize.height():
                	thumbnail = thumbnail.scaled(thumbSize, Qt.KeepAspectRatio, Qt.FastTransformation)
                else:
                	thumbnail = thumbnail.scaled(thumbSize, Qt.KeepAspectRatio, Qt.SmoothTransformation)
                thumbnail.setDevicePixelRatio(self.devicePixelRatioF)
                self.recentDocuments.append(thumbnail)
        self.modelReset.emit()
