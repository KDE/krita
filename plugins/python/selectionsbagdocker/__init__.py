from krita import Krita, DockWidgetFactory
from .selectionsbagdocker import SelectionsBagDocker

Krita.instance().addDockWidgetFactory(
    DockWidgetFactory("SelectionsBagDocker",
                      DockWidgetFactory.DockRight,
                      SelectionsBagDocker))
