'''
This script is licensed CC 0 1.0, so that you can learn from it.

------ CC 0 1.0 ---------------

The person who associated a work with this deed has dedicated the work to the public domain by waiving all of his or her rights to the work worldwide under copyright law, including all related and neighboring rights, to the extent allowed by law.

You can copy, modify, distribute and perform the work, even for commercial purposes, all without asking permission.

https://creativecommons.org/publicdomain/zero/1.0/legalcode
'''
import krita
from . import uicolorspace


class ColorSpaceExtension(krita.Extension):

    def __init__(self, parent):
        super(ColorSpaceExtension, self).__init__(parent)

    def setup(self):
        pass

    def createActions(self, window):
        action = window.createAction("color_space", i18n("Color Space"))
        action.setToolTip(i18n("Plugin to change color space of selected documents."))
        action.triggered.connect(self.initialize)

    def initialize(self):
        self.uicolorspace = uicolorspace.UIColorSpace()
        self.uicolorspace.initialize()


Scripter.addExtension(ColorSpaceExtension(krita.Krita.instance()))
