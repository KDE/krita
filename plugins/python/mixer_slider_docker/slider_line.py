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
from PyQt5.QtWidgets import QHBoxLayout, QWidget
from PyQt5.QtGui import QPixmap, QPainter
from PyQt5.Qt import pyqtSlot, pyqtSignal

from .color_slider import ColorSlider


class SliderBtn(QWidget):
    clicked = pyqtSignal()

    def __init__(self, parent=None):
        super(SliderBtn, self).__init__(parent)

    def set_color(self, qcolor):
        self.color = qcolor
        self.update()

    def update_color(self):
        color_sq = QPixmap(self.width(), self.height())
        color_sq.fill(self.color)
        image = color_sq.toImage()

        painter = QPainter(self)
        painter.drawImage(0, 0, image)

    def paintEvent(self, event):
        self.update_color()

    def mouseReleaseEvent(self, event):
        self.clicked.emit()


class SliderLine(QWidget):
    def __init__(self, left_color, right_color, docker, parent=None):
        super(SliderLine, self).__init__(parent)
        self.left_button = SliderBtn()
        self.right_button = SliderBtn()
        self.docker = docker
        self.color_slider = ColorSlider(docker)
        self.layout = QHBoxLayout()
        self.layout.setContentsMargins(2, 2, 2, 2)
        self.setLayout(self.layout)
        self.layout.addWidget(self.left_button)
        self.layout.addWidget(self.color_slider)
        self.layout.addWidget(self.right_button)
        self.left_button.clicked.connect(self.slot_update_left_color)
        self.right_button.clicked.connect(self.slot_update_right_color)
        self.set_color('left', left_color)
        self.set_color('right', right_color)
        self.left_button.setMinimumSize(30, 30)
        self.left_button.setMaximumSize(30, 30)
        self.right_button.setMinimumSize(30, 30)
        self.right_button.setMaximumSize(30, 30)
        self.color_slider.setMaximumHeight(30)

    def set_color(self, pos, color):
        button_to_set = None
        if pos == 'left':
            self.left = color
            button_to_set = self.left_button
        else:
            self.right = color
            button_to_set = self.right_button

        self.color_slider.set_color(pos, color)

        button_to_set.set_color(self.docker.managedcolor_to_qcolor(color))

    @pyqtSlot()
    def slot_update_left_color(self):
        if self.docker.canvas() is not None:
            if self.docker.canvas().view() is not None:
                self.set_color('left', self.docker.canvas().view().foregroundColor())
        self.color_slider.value_x = 0  # set the cursor to the left-most
        self.color_slider.update()
        self.docker.write_settings()

    @pyqtSlot()
    def slot_update_right_color(self):
        if self.docker.canvas() is not None:
            if self.docker.canvas().view() is not None:
                self.set_color('right', self.docker.canvas().view().foregroundColor())
        self.color_slider.value_x = self.color_slider.width() - 1
        self.color_slider.update()
        self.docker.write_settings()
