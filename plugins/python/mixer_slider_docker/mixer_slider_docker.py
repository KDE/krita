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
from PyQt5.QtGui import QColor
from PyQt5.QtWidgets import QWidget, QVBoxLayout, QHBoxLayout, QPushButton

from krita import Krita, DockWidget, ManagedColor, DockWidgetFactory, DockWidgetFactoryBase

from .slider_line import SliderLine
from .ui_mixer_slider_docker import UIMixerSliderDocker


class MixerSliderDocker(DockWidget):
    # Init the docker

    def __init__(self):
        super(MixerSliderDocker, self).__init__()

        main_program = Krita.instance()
        settings = main_program.readSetting("", "MixerSliderColors",
                                            "RGBA,U8,sRGB-elle-V2-srgbtrc.icc,1,0.8,0.4,1|" +
                                            "RGBA,U8,sRGB-elle-V2-srgbtrc.icc,0,0,0,1")  # alpha=1 == non-transparent

        self.default_left_color = self.qcolor_to_managedcolor(QColor.fromRgbF(0.4, 0.8, 1, 1))
        self.default_right_color = self.qcolor_to_managedcolor(QColor.fromRgbF(0, 0, 0, 1))

        # make base-widget and layout
        self.widget = QWidget()
        self.sliders = []
        self.top_layout = QVBoxLayout()
        self.main_layout = QHBoxLayout()
        self.top_layout.addLayout(self.main_layout)
        self.top_layout.addStretch(0)
        # The text on the button for settings
        self.settings_button = QPushButton(i18n('S'))
        self.settings_button.setToolTip(i18n('Change settings'))
        self.settings_button.setMaximumSize(30, 30)
        self.main_layout.addWidget(self.settings_button)
        self.layout = QVBoxLayout()
        self.layout.setSpacing(0)
        self.main_layout.addLayout(self.layout)
        for line in settings.split(";"):
            colors = line.split('|')
            left_color = self.parse_color(colors[0].split(','))
            right_color = self.parse_color(colors[1].split(','))
            widget = SliderLine(left_color, right_color, self)
            self.sliders.append(widget)
            self.layout.addWidget(widget)

        self.widget.setLayout(self.top_layout)
        self.setWindowTitle(i18n("Mixer Slider Docker"))
        self.setWidget(self.widget)
        [x.show() for x in self.sliders]

        self.settings_button.clicked.connect(self.init_ui)

    def settings_changed(self):
        if self.ui.line_edit is not None:
            num_sliders = int(self.ui.line_edit.text())
            if len(self.sliders) > num_sliders:
                for extra_line in self.sliders[num_sliders:]:
                    self.layout.removeWidget(extra_line)
                    extra_line.setParent(None)

                self.sliders = self.sliders[0:num_sliders]
            elif len(self.sliders) < num_sliders:
                for i in range(num_sliders - len(self.sliders)):
                    widget = SliderLine(self.default_left_color, self.default_right_color, self)
                    self.sliders.append(widget)
                    self.layout.addWidget(widget)
        self.write_settings()

    def get_color_space(self):
        if self.canvas() is not None:
            if self.canvas().view() is not None:
                canvas_color = self.canvas().view().foregroundColor()
                return ManagedColor(canvas_color.colorModel(), canvas_color.colorDepth(), canvas_color.colorProfile())
        return ManagedColor('RGBA', 'U8', 'sRGB-elle-V2-srgbtrc.icc')

    def init_ui(self):
        self.ui = UIMixerSliderDocker()
        self.ui.initialize(self)

    def write_settings(self):
        main_program = Krita.instance()
        setting = ';'.join(
            [self.color_to_settings(line.left) + '|' + self.color_to_settings(line.right)
             for line in self.sliders])

        main_program.writeSetting("", "MixerSliderColors", setting)

    def color_to_settings(self, managedcolor):
        return ','.join([managedcolor.colorModel(),
                         managedcolor.colorDepth(),
                         managedcolor.colorProfile()] +
                        [str(c) for c in managedcolor.components()])

    def parse_color(self, array):
        color = ManagedColor(array[0], array[1], array[2])
        color.setComponents([float(x) for x in array[3:]])
        return color

    def canvasChanged(self, canvas):
        pass

    def qcolor_to_managedcolor(self, qcolor):
        mc = ManagedColor.fromQColor(qcolor, self.canvas())
        return mc

    def managedcolor_to_qcolor(self, managedcolor):
        return managedcolor.colorForCanvas(self.canvas())

Application.addDockWidgetFactory(DockWidgetFactory("mixer_slider_docker", DockWidgetFactoryBase.DockRight, MixerSliderDocker))
