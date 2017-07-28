from PyQt5.QtWidgets import QWidget, QVBoxLayout, QListView
import krita


class ScriptDocker(krita.DockWidget):

    def __init__(self):
       super(ScriptDocker, self).__init__()

       self.baseWidget = QWidget()
       self.layout = QVBoxLayout()

       self.baseWidget.setLayout(self.layout)
       self.setWidget(self.baseWidget)

       self.setWindowTitle("Script Docker")

    def canvasChanged(self, canvas):
        pass


Application.addDockWidgetFactory(krita.DockWidgetFactory("scriptdocker", krita.DockWidgetFactoryBase.DockRight, ScriptDocker))
