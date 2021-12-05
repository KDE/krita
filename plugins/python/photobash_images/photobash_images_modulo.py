# Photobash Images is a Krita plugin to get CC0 images based on a search,
# straight from the Krita Interface. Useful for textures and concept art!
# Copyright (C) 2020  Pedro Reis.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from krita import *
from PyQt5 import QtWidgets, QtCore

DRAG_DELTA = 30
TRIANGLE_SIZE = 20

FAVOURITE_TRIANGLE = QPolygon([
    QPoint(0, 0),
    QPoint(0, TRIANGLE_SIZE),
    QPoint(TRIANGLE_SIZE, 0)
])

def customPaintEvent(instance, event):
    painter = QPainter(instance)
    painter.setRenderHint(QtGui.QPainter.Antialiasing, True)
    painter.setPen(QPen(Qt.black, 2, Qt.SolidLine))
    painter.setBrush(QBrush(Qt.white, Qt.SolidPattern))

    # Calculations
    total_width = event.rect().width()
    total_height = event.rect().height()
    image_width = instance.qimage.width()
    image_height = instance.qimage.height()

    try:
        var_w = total_width / image_width
        var_h = total_height / image_height
    except:
        var_w = 1
        var_h = 1

    size = 0

    if var_w <= var_h:
        size = var_w
    if var_w > var_h:
        size = var_h

    wt2 = total_width * 0.5
    ht2 = total_height * 0.5

    instance.scaled_width = image_width * size
    instance.scaled_height = image_height * size

    offset_x = wt2 - (instance.scaled_width * 0.5)
    offset_y = ht2 - (instance.scaled_height * 0.5)

    # Save State for Painter
    painter.save()
    painter.translate(offset_x, offset_y)
    painter.scale(size, size)
    painter.drawImage(0,0,instance.qimage)
    # paint something if it is a favourite
    if hasattr(instance, 'isFavourite'):
        if instance.isFavourite: 
            # reset scale to draw favourite triangle
            painter.scale(1/size, 1/size)
            painter.drawPolygon(FAVOURITE_TRIANGLE)

    # Restore Space
    painter.restore()

def customSetImage(instance, image):
    instance.qimage = QImage() if image is None else image
    instance.pixmap = QPixmap(50, 50).fromImage(instance.qimage)

    instance.update()

def customMouseMoveEvent(self, event):
    if event.modifiers() != QtCore.Qt.ShiftModifier and event.modifiers() != QtCore.Qt.AltModifier:
        self.PREVIOUS_DRAG_X = None
        return 

    # alt modifier is reserved for scrolling through
    if self.PREVIOUS_DRAG_X and event.modifiers() == QtCore.Qt.AltModifier:
        if self.PREVIOUS_DRAG_X < event.x() - DRAG_DELTA:
            self.SIGNAL_WUP.emit(0)
            self.PREVIOUS_DRAG_X = event.x()
        elif self.PREVIOUS_DRAG_X > event.x() + DRAG_DELTA:
            self.SIGNAL_WDN.emit(0)
            self.PREVIOUS_DRAG_X = event.x()

        return 

    # MimeData
    mimedata = QMimeData()
    url = QUrl().fromLocalFile(self.path)
    mimedata.setUrls([url])

    # create appropriate res image that will placed
    doc = Krita.instance().activeDocument()

    # Saving a non-existent document causes crashes, so lets check for that first.
    if doc is None:
        return 

    scale = self.scale / 100

    # only scale to document if it exists
    if self.fitCanvasChecked and not doc is None:
        fullImage = QImage(self.path).scaled(doc.width() * scale, doc.height() * scale, Qt.KeepAspectRatio, Qt.SmoothTransformation)
    else:
        fullImage = QImage(self.path)
        # scale image, now knowing the bounds
        fullImage = fullImage.scaled(fullImage.width() * scale, fullImage.height() * scale, Qt.KeepAspectRatio, Qt.SmoothTransformation)

    fullPixmap = QPixmap(50, 50).fromImage(fullImage)
    mimedata.setImageData(fullPixmap)

    # Clipboard
    QApplication.clipboard().setImage(self.qimage)

    # drag, using information about the smaller version of the image
    drag = QDrag(self)
    drag.setMimeData(mimedata)
    drag.setPixmap(self.pixmap)
    drag.setHotSpot(QPoint(self.qimage.width() / 2, self.qimage.height() / 2))
    drag.exec_(Qt.CopyAction)

class Photobash_Display(QWidget):
    SIGNAL_HOVER = QtCore.pyqtSignal(str)
    SIGNAL_CLOSE = QtCore.pyqtSignal(int)
    fitCanvasChecked = False
    scale = 100

    def __init__(self, parent):
        super(Photobash_Display, self).__init__(parent)
        customSetImage(self, None)

    def sizeHint(self):
        return QtCore.QSize(5000,5000)

    def enterEvent(self, event):
        self.SIGNAL_HOVER.emit("D")

    def leaveEvent(self, event):
        self.SIGNAL_HOVER.emit("None")

    def mousePressEvent(self, event):
        if (event.modifiers() == QtCore.Qt.NoModifier and event.buttons() == QtCore.Qt.LeftButton):
            self.SIGNAL_CLOSE.emit(0)

    def mouseMoveEvent(self, event):
        customMouseMoveEvent(self, event)

    def setFitCanvas(self, newFit):
        self.fitCanvasChecked = newFit

    def setImageScale(self, newScale):
        self.scale = newScale

    def setImage(self, path, image):
        self.path = path
        customSetImage(self, image)

    def paintEvent(self, event):
        customPaintEvent(self, event)

class Photobash_Button(QWidget):
    SIGNAL_HOVER = QtCore.pyqtSignal(str)
    SIGNAL_LMB = QtCore.pyqtSignal(int)
    SIGNAL_WUP = QtCore.pyqtSignal(int)
    SIGNAL_WDN = QtCore.pyqtSignal(int)
    SIGNAL_PREVIEW = QtCore.pyqtSignal(str)
    SIGNAL_FAVOURITE = QtCore.pyqtSignal(str)
    SIGNAL_UN_FAVOURITE = QtCore.pyqtSignal(str)
    SIGNAL_OPEN_NEW = QtCore.pyqtSignal(str)
    SIGNAL_REFERENCE = QtCore.pyqtSignal(str)
    SIGNAL_DRAG = QtCore.pyqtSignal(int)
    PREVIOUS_DRAG_X = None
    fitCanvasChecked = False
    scale = 100
    isFavourite = False

    def __init__(self, parent):
        super(Photobash_Button, self).__init__(parent)
        # Variables
        self.number = -1
        # QImage
        customSetImage(self, None)

        self.scaled_width = 1
        self.scaled_height = 1

    def setFavourite(self, newFavourite):
        self.isFavourite = newFavourite

    def setImageScale(self, newScale):
        self.scale = newScale

    def setFitCanvas(self, newFit):
        self.fitCanvasChecked = newFit

    def setNumber(self, number):
        self.number = number

    def sizeHint(self):
        return QtCore.QSize(2000,2000)

    def enterEvent(self, event):
        self.SIGNAL_HOVER.emit(str(self.number))

    def leaveEvent(self, event):
        self.SIGNAL_HOVER.emit("None")

    def mousePressEvent(self, event):
        if event.modifiers() == QtCore.Qt.NoModifier and event.buttons() == QtCore.Qt.LeftButton:
            self.SIGNAL_LMB.emit(self.number)
        if event.modifiers() == QtCore.Qt.AltModifier:
            self.PREVIOUS_DRAG_X = event.x()

    def mouseDoubleClickEvent(self, event):
        # Prevent double click to open the same image twice
        pass

    def mouseMoveEvent(self, event):
        customMouseMoveEvent(self, event)

    def wheelEvent(self,event):
        delta = event.angleDelta()
        if delta.y() > 20:
            self.SIGNAL_WUP.emit(0)
        elif delta.y() < -20:
            self.SIGNAL_WDN.emit(0)

    # menu opened with right click
    def contextMenuEvent(self, event):
        cmenu = QMenu(self)

        cmenuDisplay = cmenu.addAction("Preview in Docker")
        favouriteString = "Unpin" if self.isFavourite else "Pin to Beginning"
        cmenuFavourite = cmenu.addAction(favouriteString)
        cmenuOpenNew = cmenu.addAction("Open as New Document")
        cmenuReference = cmenu.addAction("Place as Reference")

        background = qApp.palette().color(QPalette.Window).name().split("#")[1]
        cmenuStyleSheet = f"""QMenu {{ background-color: #AA{background}; border: 1px solid #{background}; }}"""
        cmenu.setStyleSheet(cmenuStyleSheet)

        action = cmenu.exec_(self.mapToGlobal(event.pos()))
        if action == cmenuDisplay:
            self.SIGNAL_PREVIEW.emit(self.path)
        if action == cmenuFavourite:
            if self.isFavourite:
                self.SIGNAL_UN_FAVOURITE.emit(self.path)
            else:
                self.SIGNAL_FAVOURITE.emit(self.path)
        if action == cmenuOpenNew:
            self.SIGNAL_OPEN_NEW.emit(self.path)
        if action == cmenuReference:
            self.SIGNAL_REFERENCE.emit(self.path)

    def setImage(self, path, image):
        self.path = path
        customSetImage(self, image)

    def paintEvent(self, event):
        customPaintEvent(self, event)
