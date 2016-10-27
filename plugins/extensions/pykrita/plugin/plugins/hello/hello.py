import sys
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from krita import *

def hello():
    from mikro import create_pyqt_object, Error as MiKroError
    app = create_pyqt_object(Krita.instance())

    print("Actions", app.Actions["next_frame"])
    #print("ActiveDocument", app.ActiveDocument)
    #print("Batchmode", app.Batchmode)
    #print("Exporters", app.Exporters)
    #print("Filters", app.Filters)
    #print("Generators", app.Generators)
    #print("Importers", app.Importers)
    #print("Notifier:", app.Notifier, "Active:", app.Notifier.Active)
    #print("Preferences:", app.Preferences)
    #print("Views:", app.Views)
    #print("Windows", app.Windows)
    #print("Resources", app.Resources)

    #document = app.createDocument()
    #document = app.openDocument()
    #window = app.openWindow()
    #action = app.createAction("test")

    QMessageBox.information(QWidget(), "Test", "Hello World")

class HelloViewExtension(ViewExtension):

  def __init__(self, parent):
      super().__init__(parent)


  def setup(self):
      print("Hello Setup")
      action = Krita.instance().createAction("Hello")
      action.triggered.connect(hello)

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
