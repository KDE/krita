from filtermanager import filtermanagerdialog
from filtermanager.components import (filtercombobox, filtermanagertreemodel)
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (QFormLayout, QAbstractItemView, QDialogButtonBox,
                             QVBoxLayout, QFrame, QAbstractScrollArea, QWidget,
                             QTreeView)
import krita


class UIFilterManager(object):

    def __init__(self):
        self.mainDialog = filtermanagerdialog.FilterManagerDialog()
        self.mainLayout = QVBoxLayout(self.mainDialog)
        self.formLayout = QFormLayout()
        self.buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)

        self.kritaInstance = krita.Krita.instance()
        self._filters = sorted(self.kritaInstance.filters())
        self._documents = self.kritaInstance.documents()
        self.treeModel = filtermanagertreemodel.FilterManagerTreeModel(self)

        self.documentsTreeView = QTreeView()
        self.filterComboBox = filtercombobox.FilterComboBox(self)

        self.buttonBox.accepted.connect(self.confirmButton)
        self.buttonBox.rejected.connect(self.mainDialog.close)

        self.documentsTreeView.setSelectionMode(QAbstractItemView.MultiSelection)
        self.mainDialog.setWindowModality(Qt.NonModal)

    def initialize(self):
        self.documentsTreeView.setModel(self.treeModel)
        self.documentsTreeView.setWindowTitle("Document Tree Model")
        self.documentsTreeView.resizeColumnToContents(0)
        self.documentsTreeView.resizeColumnToContents(1)
        self.documentsTreeView.resizeColumnToContents(2)

        self.formLayout.addRow("Filters", self.filterComboBox)

        self.line = QFrame()
        self.line.setFrameShape(QFrame.HLine)
        self.line.setFrameShadow(QFrame.Sunken)

        self.mainLayout.addWidget(self.documentsTreeView)
        self.mainLayout.addLayout(self.formLayout)
        self.mainLayout.addWidget(self.line)
        self.mainLayout.addWidget(self.buttonBox)

        self.mainDialog.resize(500, 300)
        self.mainDialog.setWindowTitle("Filter Manager")
        self.mainDialog.setSizeGripEnabled(True)
        self.mainDialog.show()
        self.mainDialog.activateWindow()

    def confirmButton(self):
        selectionModel = self.documentsTreeView.selectionModel()
        for index in selectionModel.selectedRows():
            node = self.treeModel.data(index, Qt.UserRole + 1)
            documentIndex = self.treeModel.data(index, Qt.UserRole + 2)
            self.applyFilterOverNode(node, self.documents[documentIndex])

    def applyFilterOverNode(self, node, document):
        _filter = self.kritaInstance.filter(self.filterComboBox.currentText())
        _filter.apply(node, 0, 0, document.width(), document.height())

    @property
    def filters(self):
        return self._filters

    @property
    def documents(self):
        return self._documents
