import krita
from .filtermanager import FilterManagerExtension


Scripter.addExtension(FilterManagerExtension(krita.Krita.instance()))
