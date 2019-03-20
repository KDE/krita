# This script is licensed CC 0 1.0, so that you can learn from it.

# ------ CC 0 1.0 ---------------

# The person who associated a work with this deed has dedicated the
# work to the public domain by waiving all of his or her rights to the
# work worldwide under copyright law, including all related and
# neighboring rights, to the extent allowed by law.

# You can copy, modify, distribute and perform the work, even for
# commercial purposes, all without asking permission.

# https://creativecommons.org/publicdomain/zero/1.0/legalcode

from PyQt5.QtWidgets import QDialog


class TenScriptsDialog(QDialog):

    def __init__(self, uitenscripts, parent=None):
        super(TenScriptsDialog, self).__init__(parent)

        self.uitenscripts = uitenscripts

    def accept(self):
        self.uitenscripts.tenscripts.writeSettings()

        super(TenScriptsDialog, self).accept()

    def closeEvent(self, event):
        event.accept()
