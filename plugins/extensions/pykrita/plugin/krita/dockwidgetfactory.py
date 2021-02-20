#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyKrita.krita import *


class DockWidgetFactory(DockWidgetFactoryBase):

    def __init__(self, _id, _dockPosition, _klass):
        super(DockWidgetFactory, self).__init__(_id, _dockPosition)
        self.klass = _klass

    def createDockWidget(self):
        return self.klass()
