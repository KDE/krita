#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

"""
Exception hook
If some unexpected error occurs it can be shown in a nice looking dialog.
Especially useful is the traceback view.

Things to extend: Clicking on the filename should open an editor.
Things to consider: Mail exceptions, copy to clipboard or send to bug tracker.
"""
import sys
import cgitb
import atexit

from PyQt5.QtCore import pyqtSlot, Qt
from PyQt5.QtWidgets import QApplication, QDialog

from excepthook_ui import Ui_ExceptHookDialog


def on_error(exc_type, exc_obj, exc_tb):
    """
    This is the callback function for sys.excepthook
    """
    dlg = ExceptHookDialog(exc_type, exc_obj, exc_tb)
    dlg.show()
    dlg.exec_()


def show_current_error(title=None):
    """
    Call this function to show the current error.
    It can be used inside an except-block.
    """
    dlg = ExceptHookDialog(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2], title)
    dlg.show()
    dlg.exec_()


def install():
    "activates the error handler"
    sys.excepthook = on_error


def uninstall():
    "removes the error handler"
    sys.excepthook = sys.__excepthook__

atexit.register(uninstall)


class ExceptHookDialog(QDialog):

    def __init__(self, exc_type, exc_obj, exc_tb, title=None):
        QDialog.__init__(self)
        self.ui = Ui_ExceptHookDialog()
        self.ui.setupUi(self)
        if title:
            self.setWindowTitle(self.windowTitle() + ": " + title)
        msg = "%s: %s" % (exc_type.__name__, exc_obj)
        self.ui.exceptionLabel.setText(msg)
        html = cgitb.text((exc_type, exc_obj, exc_tb))
        self.ui.tracebackBrowser.setText(html)
        self.resize(650, 350)  # give enough space to see the backtrace better

    @pyqtSlot()
    def on_closeButton_clicked(self):
        self.close()


if __name__ == "__main__":
    # Some tests:
    app = QApplication(sys.argv)
    install()
    print("Triggering error 1")
    try:
        fail = 1 / 0
    except:
        show_current_error("Using inside except")
    print("Triggering error 2")
    fail2 = 1 / 0
    print("This will never be reached because excepthook")
    print("complains about fail2")
