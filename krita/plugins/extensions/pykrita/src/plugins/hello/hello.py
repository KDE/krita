from PyQt4.QtGui import *
from PyKrita4.krita import *

import krita

def hello():
    QMessageBox.information(QWidget(), "Test", "Hello World")

kr = Krita()
ac = kr.createAction("Hello")
ac.triggered.connect(hello)
