"""
SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
SPDX-FileCopyrightText: 2018 Ragnar Brynúlfsson <me@ragnarb.com>

This file is part of the Comics Project Management Tools(CPMT).

SPDX-License-Identifier: GPL-3.0-or-later
"""

"""
This is a docker that shows your comic pages.
"""

import os
import sys
import json
import zipfile
from PyQt5.QtGui import QImage, QPainter
from PyQt5.QtWidgets import QDialog, QWidget, QHBoxLayout, QVBoxLayout, QPushButton, QSizePolicy, QDialogButtonBox, QShortcut, QLabel
from PyQt5.QtCore import QSize, Qt

# To run standalon
from PyQt5.QtWidgets import QApplication


class page_viewer(QPushButton):

    def __init__(self, parent=None, flags=None):
        super(page_viewer, self).__init__(parent)
        self.alignment = 'left'
        self.image = QImage()
        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

    def align_left(self):
        self.alignment = 'left'

    def align_right(self):
        self.alignment = 'right'

    def align_center(self):
        self.alignment = 'center'

    def set_image(self, image=QImage()):
        self.image = image
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        previewSize = self.size()*self.devicePixelRatioF()
        image = ""

        if self.image.width() <= previewSize.width() or self.image.height() <= previewSize.height():
            # pixel art
            image = self.image.scaled(previewSize, Qt.KeepAspectRatio, Qt.FastTransformation)
        else:
            image = self.image.scaled(previewSize, Qt.KeepAspectRatio, Qt.SmoothTransformation)
        image.setDevicePixelRatio(self.devicePixelRatioF())
        if self.alignment == 'right':
            x_offset = int(self.width() - image.width()/self.devicePixelRatioF())
        elif self.alignment == 'center':
            x_offset = int((self.width() - image.width()/self.devicePixelRatioF()) / 2)
        else:
            x_offset = 0
        painter.drawImage(x_offset, 0, image)

    def sizeHint(self):
        return QSize(256, 256)


class comics_project_page_viewer(QDialog):
    pageIndex = 0
    spread = True

    def __init__(self):
        super().__init__()

        self.setModal(False)
        self.setWindowTitle('Untitled')
        self.setWindowFlags(
            Qt.WindowTitleHint |
            Qt.WindowMinimizeButtonHint |
            Qt.WindowMaximizeButtonHint |
            Qt.WindowCloseButtonHint
            )
        self.resize(1024, 768)
        self.setLayout(QVBoxLayout())

        self.left_viewer = page_viewer()
        self.left_viewer.align_right()
        self.left_viewer.clicked.connect(self.prev_page)
        self.right_viewer = page_viewer()
        self.right_viewer.align_left()
        self.right_viewer.clicked.connect(self.next_page)
        self.single_viewer = page_viewer()
        self.single_viewer.align_center()
        self.single_viewer.clicked.connect(self.next_page)
        self.single_viewer.hide()

        self.page_layout = QHBoxLayout()
        self.layout().addLayout(self.page_layout)
        self.page_layout.addWidget(self.left_viewer)
        self.page_layout.addWidget(self.right_viewer)
        self.page_layout.addWidget(self.single_viewer)

        # Keyboard shortcuts
        self.home_shortcut = QShortcut('Home', self)
        self.home_shortcut.activated.connect(self.first_page)
        self.left_shortcut = QShortcut('Left', self)
        self.left_shortcut.activated.connect(self.prev_page)
        self.right_shortcut = QShortcut('Right', self)
        self.right_shortcut.activated.connect(self.next_page)
        self.space_shortcut = QShortcut('Space', self)
        self.space_shortcut.activated.connect(self.next_page)
        self.end_shortcut = QShortcut('End', self)
        self.end_shortcut.activated.connect(self.last_page)

        self.first_btn = QPushButton('First')
        self.first_btn.clicked.connect(self.first_page)
        self.prev_btn = QPushButton('Previous')
        self.prev_btn.clicked.connect(self.prev_page)
        self.spread_btn = QPushButton('Single Page')
        self.spread_btn.clicked.connect(self.toggle_spread)
        self.next_btn = QPushButton('Next')
        self.next_btn.clicked.connect(self.next_page)
        self.last_btn = QPushButton('Last')
        self.last_btn.clicked.connect(self.last_page)

        self.buttons_layout = QHBoxLayout()
        self.layout().addLayout(self.buttons_layout)
        self.buttons_layout.addWidget(self.first_btn)
        self.buttons_layout.addWidget(self.prev_btn)
        self.buttons_layout.addWidget(self.spread_btn)
        self.buttons_layout.addWidget(self.next_btn)
        self.buttons_layout.addWidget(self.last_btn)

    def load_comic(self, path_to_config):
        self.path_to_config = path_to_config
        configFile = open(self.path_to_config, "r", newline="", encoding="utf-16")
        self.setupDictionary = json.load(configFile)
        self.projecturl = os.path.dirname(str(self.path_to_config))
        if 'readingDirection' in self.setupDictionary:
            if self.setupDictionary['readingDirection'] == "leftToRight":
                self.setLayoutDirection(Qt.LeftToRight)
                self.left_shortcut.disconnect()
                self.right_shortcut.disconnect()
                self.left_shortcut.activated.connect(self.prev_page)
                self.right_shortcut.activated.connect(self.next_page)
            else:
                self.left_shortcut.disconnect()
                self.right_shortcut.disconnect()
                self.setLayoutDirection(Qt.RightToLeft)
                self.left_shortcut.activated.connect(self.next_page)
                self.right_shortcut.activated.connect(self.prev_page)
        else:
            self.left_shortcut.disconnect()
            self.right_shortcut.disconnect()
            self.setLayoutDirection(Qt.LeftToRight)
            self.left_shortcut.activated.connect(self.prev_page)
            self.right_shortcut.activated.connect(self.next_page)
        configFile.close()

    def go_to_page_index(self, index):
        if index >= 0 and index < len(self.setupDictionary['pages']):
            self.pageIndex = index
        else:
            self.pageIndex = 0
        self.flip_page()

    def toggle_spread(self):
        if self.spread:
            self.spread = False
            self.spread_btn.setText('Double Spread')
            self.update_single_page(self.pageIndex)
            self.left_viewer.hide()
            self.right_viewer.hide()
            self.single_viewer.show()
            self.flip_page()
        else:
            self.spread = True
            self.spread_btn.setText('Single Page')
            self.left_viewer.show()
            self.right_viewer.show()
            self.single_viewer.hide()
            self.flip_page()

    def update_single_page(self, index):
        image = self.get_mergedimage(index)
        self.single_viewer.set_image(image)

    def update_left_page(self, index):
        image = self.get_mergedimage(index)
        self.left_viewer.set_image(image)

    def update_right_page(self, index):
        image = self.get_mergedimage(index)
        self.right_viewer.set_image(image)

    def first_page(self):
        self.pageIndex = 0
        self.flip_page()

    def prev_page(self):
        if self.pageIndex <= 0:
            self.pageIndex = len(self.setupDictionary['pages']) - 1
        else:
            if self.spread:
                self.pageIndex -= 2
            else:
                self.pageIndex -= 1
        self.flip_page()

    def next_page(self):
        if self.pageIndex >= len(self.setupDictionary['pages']) - 1:
            self.pageIndex = 0
        else:
            if self.spread:
                self.pageIndex += 2
            else:
                self.pageIndex += 1
        self.flip_page()

    def last_page(self):
        self.pageIndex = len(self.setupDictionary['pages']) - 1
        self.flip_page()

    def flip_page(self):
        if self.spread:
            if self.pageIndex % 2 == 0: # Even/Left
                left_page_number = self.pageIndex - 1
                right_page_number = self.pageIndex
            else: # Odd/Right
                left_page_number = self.pageIndex
                right_page_number = self.pageIndex + 1
            self.update_left_page(left_page_number)
            self.update_right_page(right_page_number)
            if left_page_number < 0:
                page_numbers = str(right_page_number + 1)
            elif right_page_number >= len(self.setupDictionary['pages']):
                page_numbers = str(left_page_number + 1)
            else:
                page_numbers = '{left} / {right}'.format(
                    left = str(left_page_number + 1),
                    right = str(right_page_number + 1))
            self.setWindowTitle('{name} - {page_numbers}'.format(
                name = self.setupDictionary['projectName'],
                page_numbers = page_numbers))
        else:
            if self.pageIndex >= len(self.setupDictionary['pages']):
                self.pageIndex = len(self.setupDictionary['pages']) - 1
            if self.pageIndex < 0:
                self.pageIndex = 0
            self.update_single_page(self.pageIndex)
            self.setWindowTitle('{name} - {page_numbers}'.format(
                name = self.setupDictionary['projectName'],
                page_numbers = str(self.pageIndex + 1)))


    def get_mergedimage(self, index):
        if index < len(self.setupDictionary['pages']) and index > -1:
            image_url = os.path.join(self.projecturl, self.setupDictionary['pages'][index])
            if os.path.exists(image_url):
                page = zipfile.ZipFile(image_url, "r")
                image = QImage.fromData(page.read("mergedimage.png"))
                page.close()
                return image
        image = QImage(QSize(10, 10), QImage.Format_ARGB32)
        image.fill(Qt.GlobalColor(19))
        return image

if __name__ == '__main__':
    ''' Run the page viewer outside Krita '''
    app = QApplication(sys.argv)
    if sys.argv:
        print(sys.argv)
        path_to_config = sys.argv[1]
        if os.path.exists(path_to_config):
            print('Run comics viewer')
            page_viewer_dialog = comics_project_page_viewer()
            page_viewer_dialog.load_comic(path_to_config)
            page_viewer_dialog.go_to_page_index(0)
            page_viewer_dialog.show()
        else:
            print('No comic found in {comic}'.format(comic=comic))
    else:
        print('Pass the path to a Krita comicConfig.json file to this script, to view the comic.')

    sys.exit(app.exec_())



