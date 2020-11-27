# SPDX-License-Identifier: CC0-1.0

import krita
from . import uicolorspace


class ColorSpaceExtension(krita.Extension):

    def __init__(self, parent):
        super(ColorSpaceExtension, self).__init__(parent)

    def setup(self):
        pass

    def createActions(self, window):
        action = window.createAction("color_space", i18n("Color Space"))
        action.setToolTip(
            i18n("Plugin to change color space of selected documents."))
        action.triggered.connect(self.initialize)

    def initialize(self):
        self.uicolorspace = uicolorspace.UIColorSpace()
        self.uicolorspace.initialize()
