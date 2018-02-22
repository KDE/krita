"""
Part of the comics project management tools (CPMT).

This is a docker that shows your comic pages.
"""

from PyQt5.QtGui import QImage, QPainter
from PyQt5.QtWidgets import QDialog, QWidget, QVBoxLayout, QSizePolicy, QDialogButtonBox
from PyQt5.QtCore import QSize, Qt
from krita import *


class page_viewer(QWidget):

    def __init__(self, parent=None, flags=None):
        super(page_viewer, self).__init__(parent)
        self.image = QImage()
        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

    def set_image(self, image=QImage()):
        self.image = image
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        image = self.image.scaled(self.size(), Qt.KeepAspectRatio, Qt.SmoothTransformation)
        painter.drawImage(0, 0, image)

    def sizeHint(self):
        return QSize(256, 256)


class comics_project_page_viewer(QDialog):
    currentPageNumber = 0

    def __init__(self):
        super().__init__()
        self.setModal(False)
        self.setWindowTitle(i18n("Comics page viewer."))
        self.setMinimumSize(200, 200)
        self.listOfImages = [QImage()]
        self.setLayout(QVBoxLayout())

        self.viewer = page_viewer()
        self.layout().addWidget(self.viewer)
        buttonBox = QDialogButtonBox(QDialogButtonBox.Close)
        buttonBox.rejected.connect(self.close)
        self.layout().addWidget(buttonBox)

    def update_image(self, image=QImage()):
        self.viewer.set_image(image)
