# This script is licensed CC 0 1.0, so that you can learn from it.

# ------ CC 0 1.0 ---------------

# The person who associated a work with this deed has dedicated the
# work to the public domain by waiving all of his or her rights to the
# work worldwide under copyright law, including all related and
# neighboring rights, to the extent allowed by law.

# You can copy, modify, distribute and perform the work, even for
# commercial purposes, all without asking permission.

# https://creativecommons.org/publicdomain/zero/1.0/legalcode

from PyQt5.QtWidgets import QWidget

from PyQt5 import uic
from krita import DockWidget
import os


class SelectionsBagDocker(DockWidget):

    def __init__(self):
        super().__init__()
        widget = QWidget(self)
        ui_filename = os.path.join(
            os.path.dirname(os.path.realpath(__file__)),
            'selectionsbagdocker.ui')
        uic.loadUi(ui_filename, widget)
        self.setWidget(widget)
        self.setWindowTitle(i18n("Selections Bag"))

    def canvasChanged(self, canvas):
        print("Canvas", canvas)
