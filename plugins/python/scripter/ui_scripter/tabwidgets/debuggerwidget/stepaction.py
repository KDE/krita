"""
SPDX-FileCopyrightText: 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
from PyQt5.QtWidgets import QAction
from PyQt5.QtGui import QIcon
from scripter import resources_rc
from .... import utils
import krita


class StepAction(QAction):

    def __init__(self, scripter, toolbar, parent=None):
        super(StepAction, self).__init__(parent)
        self.scripter = scripter
        self.toolbar = toolbar

        self.triggered.connect(self.step)

        self.setText(i18n("Step Over"))
        # path to the icon
        self.setIcon(utils.getThemedIcon(':/icons/step.svg'))

    def step(self):
        status = self.scripter.debugcontroller.isActive
        if status:
            self.scripter.debugcontroller.step()
        else:
            self.toolbar.disableToolbar(True)
