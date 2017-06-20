from PyQt5.QtWidgets import QTreeWidgetItem


class DocumentTreeWidgetItem(QTreeWidgetItem):

    def __init__(self, parent=None):
        super(DocumentTreeWidgetItem, self).__init__(parent)
