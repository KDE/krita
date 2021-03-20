'''
    SPDX-FileCopyrightText: 2019 Tusooa Zhu <tusooa@vista.aero>

    This file is part of Krita-docker-color-slider.

    SPDX-License-Identifier: GPL-3.0-or-later
'''
from PyQt5.QtWidgets import QDialogButtonBox, QLabel, QVBoxLayout, QHBoxLayout, QSpinBox
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
        self.line_edit = QSpinBox()
        self.line_edit.setValue(len(docker.sliders))
        self.hbox.addWidget(self.line_edit)

        self.vbox.addWidget(self.button_box)

        self.main_dialog.show()
        self.main_dialog.activateWindow()
        self.main_dialog.exec_()
