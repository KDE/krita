'''
    Copyright (C) 2019 Tusooa Zhu <tusooa@vista.aero>

    This file is part of Krita-docker-color-slider.

    Krita-docker-color-slider is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Krita-docker-color-slider is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Krita-docker-color-slider.  If not, see <https://www.gnu.org/licenses/>.
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
