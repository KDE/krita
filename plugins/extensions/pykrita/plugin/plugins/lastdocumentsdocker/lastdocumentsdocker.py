from PyQt5.QtWidgets import QWidget, QVBoxLayout, QListView
import krita
from lastdocumentsdocker import lastdocumentslistmodel


class LastDocumentsDocker(krita.DockWidget):

    def __init__(self):
        super(LastDocumentsDocker, self).__init__()

        self.baseWidget = QWidget()
        self.layout = QVBoxLayout()
        self.listView = QListView()

        self.listView.setFlow(QListView.LeftToRight)

        self.layout.addWidget(self.listView)

        self.baseWidget.setLayout(self.layout)
        self.setWidget(self.baseWidget)

        self.listView.setModel(lastdocumentslistmodel.LastDocumentsListModel())
        self.setWindowTitle("Last Documents Docker")

    def canvasChanged(self, canvas):
        pass


Application.addDockWidgetFactory(krita.DockWidgetFactory("lastdocumentsdocker", krita.DockWidgetFactoryBase.DockRight, LastDocumentsDocker))
