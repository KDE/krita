from PyQt4.QtGui import *
from krita import *

def hello():
    QMessageBox.information(QWidget(), "Test", "Hello World")

class HelloViewExtension(ViewExtension):
  def __init__(self, parent):
      super().__init__(parent)
  def setup(self, viewManager):
      action = viewManager.createAction("Hello")
      action.triggered.connect(hello)

Krita.instance().addViewExtension(HelloViewExtension(Krita.instance()))

class HelloDocker(DockWidget):
  def __init__(self):
      super().__init__()
      label = QLabel("Hello", self)
      self.setWidget(label)

Krita.instance().addDockWidgetFactory(DockWidgetFactory("hello", DockWidgetFactory.DockRight, HelloDocker))
