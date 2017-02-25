from PyQt5.QtWidgets import QDialog
from krita import *
from scripter import uicontroller, documentcontroller, debugcontroller


class ScripterViewExtension(ViewExtension):

    def __init__(self, parent):
        super().__init__(parent)

    def setup(self):
        action = Krita.instance().createAction("Scripter")
        action.triggered.connect(self.initialize)

    def initialize(self):
        self.uicontroller = uicontroller.UIController(QDialog())
        self.documentcontroller = documentcontroller.DocumentController()
        self.debugcontroller = debugcontroller.DebugController(self)
        self.uicontroller.initialize(self)


Krita.instance().addViewExtension(ScripterViewExtension(Krita.instance()))
