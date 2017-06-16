import krita
from filtermanager import uifiltermanager


class FilterManagerExtension(krita.Extension):

    def __init__(self, parent):
        super(FilterManagerExtension, self).__init__(parent)

    def setup(self):
        action = krita.Krita.instance().createAction("filter_manager", "Filter Manager")
        action.setToolTip("Plugin to filters management")
        action.triggered.connect(self.initialize)

    def initialize(self):
        self.uifiltermanager = uifiltermanager.UIFilterManager()
        self.uifiltermanager.initialize()


Scripter.addExtension(FilterManagerExtension(krita.Krita.instance()))
