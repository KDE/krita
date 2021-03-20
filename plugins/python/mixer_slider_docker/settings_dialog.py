'''
    SPDX-FileCopyrightText: 2019 Tusooa Zhu <tusooa@vista.aero>

    This file is part of Krita-docker-color-slider.

    SPDX-License-Identifier: GPL-3.0-or-later
'''
from PyQt5.QtWidgets import QDialog


class SettingsDialog(QDialog):
    def __init__(self, ui_mixer_slider, parent=None):
        super(SettingsDialog, self).__init__(parent)

        self.ui_mixer_slider = ui_mixer_slider

    def accept(self):
        self.ui_mixer_slider.docker.settings_changed()

        super(SettingsDialog, self).accept()

    def closeEvent(self, event):
        event.accept()
