import krita
from .exportlayers import ExportLayersExtension


Scripter.addExtension(ExportLayersExtension(krita.Krita.instance()))
