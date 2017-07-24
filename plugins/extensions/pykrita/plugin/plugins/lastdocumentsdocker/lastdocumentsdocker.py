from PyQt5.QtWidgets import QLabel
import krita


class LastDocumentsDocker(krita.DockWidget):

    def __init__(self):
       super(LastDocumentsDocker, self).__init__()

       label = QLabel("Hello", self)
       self.setWidget(label)
       self.label = label
       self.setWindowTitle("Hello Docker")

    def canvasChanged(self, canvas):
        self.label.setText("Hellodocker: canvas changed");


Application.addDockWidgetFactory(krita.DockWidgetFactory("lastdocumentsdocker", krita.DockWidgetFactoryBase.DockRight, LastDocumentsDocker))
