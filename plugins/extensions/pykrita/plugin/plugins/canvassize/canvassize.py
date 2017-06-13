import krita
from canvassize import uicanvassize


class CanvasSizeExtension(krita.Extension):

    def __init__(self, parent):
        super(CanvasSizeExtension, self).__init__(parent)

    def setup(self):
        action = krita.Krita.instance().createAction("Canvas Size")
        action.setToolTip("Plugin to change canvas size to selected documents")
        action.triggered.connect(self.initialize)

    def initialize(self):
        self.uicanvassize = uicanvassize.UICanvasSize()
        self.uicanvassize.initialize()


Scripter.addExtension(CanvasSizeExtension(krita.Krita.instance()))
