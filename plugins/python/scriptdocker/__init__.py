import krita
from .scriptdocker import ScriptDocker

Application.addDockWidgetFactory(
    krita.DockWidgetFactory("scriptdocker",
                            krita.DockWidgetFactoryBase.DockRight,
                            ScriptDocker)
)
