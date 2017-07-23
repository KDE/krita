import sys
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from krita import *
from tenbrushes import dropbutton, uitenbrushes


class TenBrushesExtension(Extension):

    def __init__(self, parent):
        super(TenBrushesExtension, self).__init__(parent)

    def setup(self):
        action = Application.createAction("ten_brushes", "Ten Brushes")
        action.setToolTip("Assign ten brush presets to ten shortcuts.")
        action.triggered.connect(self.initialize)

    def initialize(self):
        self.uitenbrushes = uitenbrushes.UITenBrushes()
        self.uitenbrushes.initialize()


Scripter.addExtension(TenBrushesExtension(Application))
