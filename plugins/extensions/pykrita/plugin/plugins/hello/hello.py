import sys
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from krita import *

def hello():
    QMessageBox.information(QWidget(), "Test", "Hello! This is Krita " + Application.version())

class HelloExtension(Extension):

  def __init__(self, parent):
      super().__init__(parent)

  def setup(self):
      qDebug("Hello Setup")
      action = Krita.instance().createAction("hello")
      action.triggered.connect(hello)

Scripter.addExtension(HelloExtension(Krita.instance()))

class HelloDocker(DockWidget):
  def __init__(self):
      super().__init__()
      label = QLabel("Hello", self)
      self.setWidget(label)
      self.label = label

  def canvasChanged(self, canvas):
      self.label.setText("Hellodocker: canvas changed");

Application.addDockWidgetFactory(DockWidgetFactory("hello", DockWidgetFactoryBase.DockRight, HelloDocker))
