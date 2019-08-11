"""
Copyright (c) 2017 Eliakin Costa <eliakim170@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
"""
from PyQt5.QtWidgets import QAction
from PyQt5.QtGui import QIcon
from scripter import resources_rc
import krita


class StepAction(QAction):

    def __init__(self, scripter, toolbar, parent=None):
        super(StepAction, self).__init__(parent)
        self.scripter = scripter
        self.toolbar = toolbar

        self.triggered.connect(self.step)

        self.setText(i18n("Step Over"))
        # path to the icon
        self.setIcon(QIcon(':/icons/step.svg'))

    def step(self):
        status = self.scripter.debugcontroller.isActive
        if status:
            self.scripter.debugcontroller.step()
        else:
            self.toolbar.disableToolbar(True)
