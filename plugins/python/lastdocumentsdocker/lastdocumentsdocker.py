'''
This script is licensed CC 0 1.0, so that you can learn from it.

------ CC 0 1.0 ---------------

The person who associated a work with this deed has dedicated the work to the public domain by waiving all of his or her rights to the work worldwide under copyright law, including all related and neighboring rights, to the extent allowed by law.

You can copy, modify, distribute and perform the work, even for commercial purposes, all without asking permission.

https://creativecommons.org/publicdomain/zero/1.0/legalcode
'''
from PyQt5.QtWidgets import QWidget, QVBoxLayout, QListView, QPushButton
import krita
from . import lastdocumentslistmodel


class LastDocumentsDocker(krita.DockWidget):

    def __init__(self):
        super(LastDocumentsDocker, self).__init__()

        self.baseWidget = QWidget()
        self.layout = QVBoxLayout()
        self.listView = QListView()
        self.loadButton = QPushButton(i18n("Refresh"))
        self.listModel = lastdocumentslistmodel.LastDocumentsListModel()

        self.listView.setModel(self.listModel)
        self.listView.setFlow(QListView.LeftToRight)

        self.layout.addWidget(self.listView)
        self.layout.addWidget(self.loadButton)

        self.baseWidget.setLayout(self.layout)
        self.setWidget(self.baseWidget)

        self.loadButton.clicked.connect(self.refreshRecentDocuments)
        self.setWindowTitle(i18n("Last Documents Docker"))

    def canvasChanged(self, canvas):
        pass

    def refreshRecentDocuments(self):
        self.listModel.loadRecentDocuments()


Application.addDockWidgetFactory(krita.DockWidgetFactory("lastdocumentsdocker", krita.DockWidgetFactoryBase.DockRight, LastDocumentsDocker))
