# This script is licensed CC 0 1.0, so that you can learn from it.

# ------ CC 0 1.0 ---------------

# The person who associated a work with this deed has dedicated the
# work to the public domain by waiving all of his or her rights to the
# work worldwide under copyright law, including all related and
# neighboring rights, to the extent allowed by law.

# You can copy, modify, distribute and perform the work, even for
# commercial purposes, all without asking permission.

# https://creativecommons.org/publicdomain/zero/1.0/legalcode

from PyQt5.QtWidgets import QPushButton
from PyQt5.QtGui import QPixmap, QIcon
from PyQt5.QtCore import QSize


class DropButton(QPushButton):

    def __init__(self, parent):
        super(DropButton, self).__init__(parent)

        self.presetChooser = None

        self.preset = None
        self.setFixedSize(64, 64)
        self.setIconSize(QSize(64, 64))

    def selectPreset(self):
        self.preset = self.presetChooser.currentPreset().name()
        current_preset = self.presetChooser.currentPreset()
        self.setIcon(QIcon(QPixmap.fromImage(current_preset.image())))
