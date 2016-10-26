from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from krita import *

def hello():
    QMessageBox.information(QWidget(), "Test", "Hello World")

class HelloViewExtension(ViewExtension):

  def __init__(self, parent):
      super().__init__(parent)

  def setup(self):
      print("Hello Setup")
      action = Krita.instance().createAction("Hello")
      action.triggered.connect(hello)

      from mikro import create_pyqt_object, Error as MiKroError
      app = create_pyqt_object(Krita.instance())

      print("Notifier:", app.Notifier.Active)
      app.Notifier.Active = True
      print("Notifier:", app.Notifier.Active)

      print(app.Notifier)
      print("Batchmmode", app.Batchmode)
      app.Batchmode = True
      print("Batchmmode", app.Batchmode)

      print(dir(app))
      print(type(app.closeApplication()))
      print(app.closeApplication())



Krita.instance().addViewExtension(HelloViewExtension(Krita.instance()))

class HelloDocker(DockWidget):
  def __init__(self):
      super().__init__()
      label = QLabel("Hello", self)
      self.setWidget(label)
      self.label = label

  def canvasChanged(self, canvas):
      self.label.setText("canvas changed");

Krita.instance().addDockWidgetFactory(DockWidgetFactory("hello", DockWidgetFactoryBase.DockRight, HelloDocker))
