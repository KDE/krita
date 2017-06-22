from filtermanager import filtermanagerdialog
from filtermanager.components import (filtercombobox, documenttreewidget,
                                      filtermanagertreemodel)
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
        self.treeModel = filtermanagertreemodel.FilterManagerTreeModel("test")

        self.documentsTreeView = QTreeView()
        self.filterComboBox = filtercombobox.FilterComboBox(self)

        self.buttonBox.accepted.connect(self.confirmButton)
        self.buttonBox.rejected.connect(self.mainDialog.close)

        self.mainDialog.setWindowModality(Qt.NonModal)

    def initialize(self):
        self.documentsTreeView.setModel(self.treeModel)
        self.documentsTreeView.setWindowTitle("Document Tree Model")

        self.formLayout.addRow(self.documentsTreeView)
        self.formLayout.addRow("Filters", self.filterComboBox)

        self.line = QFrame()
        self.line.setFrameShape(QFrame.HLine)
        self.line.setFrameShadow(QFrame.Sunken)

        self.mainLayout.addLayout(self.formLayout)
        self.mainLayout.addWidget(self.line)
        self.mainLayout.addWidget(self.buttonBox)

        self.mainDialog.resize(500, 300)
        self.mainDialog.setWindowTitle("Filter Manager")
        self.mainDialog.setSizeGripEnabled(True)
        self.mainDialog.show()
        self.mainDialog.activateWindow()

    def confirmButton(self):
        pass

    @property
    def filters(self):
        return self._filters

    @property
    def documents(self):
        return self._documents
