#!/usr/bin/env python

import sys
from qt import *

from engine import Widget

a = QApplication(sys.argv)
mw = Widget()
mw.setCaption('Prototype of the formula engine')
mw.show()
a.connect(a, SIGNAL('lastWindowClosed()'), a, SLOT('quit()'))
a.exec_loop()
