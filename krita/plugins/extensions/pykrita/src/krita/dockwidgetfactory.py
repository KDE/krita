from PyQt4.QtGui import *
from PyKrita4.krita import *

class DockWidgetFactory(DockWidgetFactoryBase):
  def __init__(self, _id, _dockPosition, _klass):
      super().__init__(_id, _dockPosition)
      self.klass = _klass
  def createDockWidget(self):
      return self.klass()
