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
from PyQt5.QtWidgets import QDialogButtonBox, QLabel, QVBoxLayout, QHBoxLayout, QLineEdit
from PyQt5.QtGui import QIntValidator
from PyQt5.QtCore import Qt
import krita

from .settings_dialog import SettingsDialog


class UIMixerSliderDocker(object):
    def __init__(self):
        self.krita_instance = krita.Krita.instance()
        self.main_dialog = SettingsDialog(self, self.krita_instance.activeWindow().qwindow())

        self.button_box = QDialogButtonBox(self.main_dialog)
        self.vbox = QVBoxLayout(self.main_dialog)
        self.hbox = QHBoxLayout(self.main_dialog)
        self.line_edit = None

        self.button_box.accepted.connect(self.main_dialog.accept)
        self.button_box.rejected.connect(self.main_dialog.reject)

        self.button_box.setOrientation(Qt.Horizontal)
        self.button_box.setStandardButtons(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)

    def initialize(self, docker):
        self.docker = docker

        self.vbox.addLayout(self.hbox)
        self.hbox.addWidget(QLabel(i18n('Number of slider lines: ')))
        self.line_edit = QLineEdit(str(len(docker.sliders)))
        self.line_edit.setValidator(QIntValidator(1, 8))
        self.hbox.addWidget(self.line_edit)

        self.vbox.addWidget(self.button_box)

        self.main_dialog.show()
        self.main_dialog.activateWindow()
        self.main_dialog.exec_()
