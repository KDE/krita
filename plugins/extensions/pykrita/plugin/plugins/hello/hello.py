import sys
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from krita import *

def hello():
    QMessageBox.information(QWidget(), "Test", "Hello Krita")
    print(dir(Krita.instance()))

class HelloViewExtension(ViewExtension):

  def __init__(self, parent):
      super().__init__(parent)

  def setup(self):
      qDebug("Hello Setup")
      action = Krita.instance().createAction("Hello")
      action.triggered.connect(hello)

Scripter.addViewExtension(HelloViewExtension(Krita.instance()))

class HelloDocker(DockWidget):
  def __init__(self):
      super().__init__()
      label = QLabel("Hello", self)
      self.setWidget(label)
      self.label = label

  def canvasChanged(self, canvas):
      self.label.setText("Hellodocker: canvas changed");

Application.addDockWidgetFactory(DockWidgetFactory("hello", DockWidgetFactoryBase.DockRight, HelloDocker))
