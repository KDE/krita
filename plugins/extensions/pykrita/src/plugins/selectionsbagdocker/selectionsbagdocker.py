from PyQt4.QtGui import *
from PyQt4 import uic
from krita import *
import os

class SelectionsBagDocker(DockWidget):
  def __init__(self):
      super().__init__()
      widget = QWidget(self)
      uic.loadUi(os.path.dirname(os.path.realpath(__file__)) + '/selectionsbagdocker.ui', widget)
      self.setWidget(widget)
      self.setWindowTitle("Selections bag")