# SPDX-License-Identifier: CC0-1.0

import krita
from . import uifiltermanager


class FilterManagerExtension(krita.Extension):

    def __init__(self, parent):
        super(FilterManagerExtension, self).__init__(parent)

    def setup(self):
        pass

    def createActions(self, window):
        action = window.createAction("filter_manager", i18n("Filter Manager"))
        action.setToolTip(i18n("Plugin to filters management."))
        action.triggered.connect(self.initialize)

    def initialize(self):
        self.uifiltermanager = uifiltermanager.UIFilterManager()
        self.uifiltermanager.initialize()
