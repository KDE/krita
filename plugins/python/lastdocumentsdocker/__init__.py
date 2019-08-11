import krita
from .lastdocumentsdocker import LastDocumentsDocker


Application.addDockWidgetFactory(
    krita.DockWidgetFactory("lastdocumentsdocker",
                            krita.DockWidgetFactoryBase.DockRight,
                            LastDocumentsDocker))
