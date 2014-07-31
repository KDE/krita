from PyQt4.QtGui import *
from PyKrita4.krita import *
from krita import *

def hello():
    QMessageBox.information(QWidget(), "Test", "Hello World")

QAction ac = Krita.createAction("Hello")
ac.triggered.conect(hello)
