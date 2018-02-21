import krita
from colorspace import uicolorspace


class ColorSpaceExtension(krita.Extension):

    def __init__(self, parent):
        super(ColorSpaceExtension, self).__init__(parent)

    def setup(self):
        action = krita.Krita.instance().createAction("color_space", "Color Space")
        action.setToolTip("Plugin to change color space to selected documents")
        action.triggered.connect(self.initialize)

    def initialize(self):
        self.uicolorspace = uicolorspace.UIColorSpace()
        self.uicolorspace.initialize()


Scripter.addExtension(ColorSpaceExtension(krita.Krita.instance()))
