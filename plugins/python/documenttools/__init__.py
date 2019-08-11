import krita
from .documenttools import DocumentToolsExtension

Scripter.addExtension(DocumentToolsExtension(krita.Krita.instance()))
