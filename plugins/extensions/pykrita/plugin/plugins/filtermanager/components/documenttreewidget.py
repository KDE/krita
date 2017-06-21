from PyQt5.QtWidgets import QTreeWidget, QTreeWidgetItem
from PyQt5.QtCore import Qt, QVariant
from PyQt5.QtGui import QPixmap


class DocumentTreeWidget(QTreeWidget):

    def __init__(self, uiFilterManager, parent=None):
        super(DocumentTreeWidget, self).__init__(parent)

        self.uiFilterManager = uiFilterManager

        self.setHeaderLabels(["Path", "Type"])

        self.loadDocuments()

    def loadDocuments(self):
        """ It loads just the top level of the tree for performance reasons. """

        for document in self.uiFilterManager.documents:
            self.item = QTreeWidgetItem()

            self.item.setText(0, document.fileName())
            self.item.setData(0, Qt.DecorationRole, QPixmap.fromImage(document.thumbnail(30, 30)))
            self.item.setText(1, "Document")
            self.item.setData(1, Qt.UserRole + 1, document.rootNode())

            self.addTopLevelItem(self.item)
