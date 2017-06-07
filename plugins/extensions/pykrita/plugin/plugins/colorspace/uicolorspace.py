from colorspace import colorspacedialog
from PyQt5.QtCore import Qt


class UIColorSpace(object):

    def __init__(self):
        self.mainDialog = colorspacedialog.ColorSpaceDialog()
        self.mainDialog.setWindowModality(Qt.NonModal)

    def initialize(self):
        self.mainDialog.resize(400, 500)
        self.mainDialog.setWindowTitle("Color Space")
        self.mainDialog.setSizeGripEnabled(True)
        self.mainDialog.show()
        self.mainDialog.activateWindow()
