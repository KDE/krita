from krita import *
from .selectionsbagdocker import *

Krita.instance().addDockWidgetFactory(DockWidgetFactory("SelectionsBagDocker", DockWidgetFactory.DockRight, SelectionsBagDocker))
