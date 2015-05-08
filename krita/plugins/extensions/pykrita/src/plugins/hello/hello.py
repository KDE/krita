from PyQt4.QtGui import *
from PyKrita4.krita import *

def hello():
    QMessageBox.information(QWidget(), "Test", "Hello World")

class HelloViewExtension(ViewExtension):
  def __init__(self, parent):
      super().__init__(parent)
  def setup(self, viewManager):
      action = viewManager.createAction("Hello")
      action.triggered.connect(hello)

Krita.instance().addViewExtension(HelloViewExtension(Krita.instance()))
