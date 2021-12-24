from krita import *
from .photobash import *

Krita.instance().addDockWidgetFactory(DockWidgetFactory("PhotobashDocker", DockWidgetFactoryBase.DockRight, PhotobashDocker))