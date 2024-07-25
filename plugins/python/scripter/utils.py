"""
SPDX-FileCopyrightText: 2022 Ivan Santa Maria <ghevan@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
"""
from PyQt5.QtGui import QIcon, QPixmap, QColor, QPainter

needDarkIcon = False

def setNeedDarkIcon(color):
    global needDarkIcon
    needDarkIcon = True if color.value() > 100 else False
    return

def getThemedIcon(filepath) -> QIcon:
    global needDarkIcon
    pixmap = QPixmap(filepath)
    painter = QPainter(pixmap)
    painter.setCompositionMode(QPainter.CompositionMode_SourceIn)

    if needDarkIcon:
        painter.fillRect(pixmap.rect(),QColor(32,32,32))
    else:
        painter.fillRect(pixmap.rect(),QColor(255,255,255))

    painter.end()
    return QIcon(pixmap)
