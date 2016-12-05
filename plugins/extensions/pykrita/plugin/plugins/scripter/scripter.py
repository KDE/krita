from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from krita import *
from scripter import uicontroller


class ScripterViewExtension(ViewExtension):

    def __init__(self, parent):
        super().__init__(parent)
        self.uicontroller = uicontroller.UIController(QDialog())

    def setup(self):
        print("Scripter setup")
        action = Krita.instance().createAction("Scripter")
        action.triggered.connect(self.initialize)

    def initialize(self):
        self.uicontroller.initialize(self)

Krita.instance().addViewExtension(ScripterViewExtension(Krita.instance()))
