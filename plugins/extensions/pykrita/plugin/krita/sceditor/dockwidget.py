# Ported from KoDockWidgetTitleBar.cpp which is part of KOffice
# SPDX-FileCopyrightText: 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
# SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
import os

from PyQt5.QtCore import QPoint, QSize, Qt, QRect, QTimer
from PyQt5.QtGui import (QIcon, QPainter)
from PyQt5.QtWidgets import (QAbstractButton, QApplication, QComboBox,
                             QDockWidget, QHBoxLayout, QLayout, QMainWindow,
                             QPushButton, QStyle, QStyleOptionDockWidget,
                             QStyleOptionToolButton, QStylePainter, QWidget)


import dockwidget_icons


def hasFeature(dockwidget, feature):
    return dockwidget.features() & feature == feature


class DockWidgetTitleBarButton(QAbstractButton):

    def __init__(self, titlebar):
        QAbstractButton.__init__(self, titlebar)
        self.setFocusPolicy(Qt.NoFocus)

    def sizeHint(self):
        self.ensurePolished()
        margin = self.style().pixelMetric(QStyle.PM_DockWidgetTitleBarButtonMargin, None, self)
        if self.icon().isNull():
            return QSize(margin, margin)
        iconSize = self.style().pixelMetric(QStyle.PM_SmallIconSize, None, self)
        pm = self.icon().pixmap(iconSize)
        return QSize(pm.width() + margin, pm.height() + margin)

    def enterEvent(self, event):
        if self.isEnabled():
            self.update()
        QAbstractButton.enterEvent(self, event)

    def leaveEvent(self, event):
        if self.isEnabled():
            self.update()
        QAbstractButton.leaveEvent(self, event)

    def paintEvent(self, event):
        p = QPainter(self)
        r = self.rect()
        opt = QStyleOptionToolButton()
        opt.init(self)
        opt.state |= QStyle.State_AutoRaise
        if self.isEnabled() and self.underMouse() and \
           not self.isChecked() and not self.isDown():
            opt.state |= QStyle.State_Raised
        if self.isChecked():
            opt.state |= QStyle.State_On
        if self.isDown():
            opt.state |= QStyle.State_Sunken
        self.style().drawPrimitive(
            QStyle.PE_PanelButtonTool, opt, p, self)
        opt.icon = self.icon()
        opt.subControls = QStyle.SubControls()
        opt.activeSubControls = QStyle.SubControls()
        opt.features = QStyleOptionToolButton.None
        opt.arrowType = Qt.NoArrow
        size = self.style().pixelMetric(QStyle.PM_SmallIconSize, None, self)
        opt.iconSize = QSize(size, size)
        self.style().drawComplexControl(QStyle.CC_ToolButton, opt, p, self)


class DockWidgetTitleBar(QWidget):
    # XXX: support QDockWidget.DockWidgetVerticalTitleBar feature

    def __init__(self, dockWidget):
        QWidget.__init__(self, dockWidget)
        self.openIcon = QIcon(":arrow-down.png")
        self.closeIcon = QIcon(":arrow-right.png")
        self.pinIcon = QIcon(":pin.png")
        q = dockWidget
        self.floatButton = DockWidgetTitleBarButton(self)
        self.floatButton.setIcon(q.style().standardIcon(
            QStyle.SP_TitleBarNormalButton, None, q))
        self.floatButton.clicked.connect(self.toggleFloating)
        self.floatButton.setVisible(True)
        self.closeButton = DockWidgetTitleBarButton(self)
        self.closeButton.setIcon(q.style().standardIcon(
            QStyle.SP_TitleBarCloseButton, None, q))
        self.closeButton.clicked.connect(dockWidget.close)
        self.closeButton.setVisible(True)
        self.collapseButton = DockWidgetTitleBarButton(self)
        self.collapseButton.setIcon(self.openIcon)
        self.collapseButton.clicked.connect(self.toggleCollapsed)
        self.collapseButton.setVisible(True)
        self.pinButton = DockWidgetTitleBarButton(self)
        self.pinButton.setIcon(self.pinIcon)
        self.pinButton.setCheckable(True)
        self.pinButton.setChecked(True)
        self.pinButton.clicked.connect(self.togglePinned)
        self.pinButton.setVisible(True)
        dockWidget.featuresChanged.connect(self.featuresChanged)
        self.featuresChanged(0)

    def minimumSizeHint(self):
        return self.sizeHint()

    def sizeHint(self):
        q = self.parentWidget()
        mw = q.style().pixelMetric(QStyle.PM_DockWidgetTitleMargin, None, q)
        fw = q.style().pixelMetric(QStyle.PM_DockWidgetFrameWidth, None, q)
        closeSize = QSize(0, 0)
        if self.closeButton:
            closeSize = self.closeButton.sizeHint()
        floatSize = QSize(0, 0)
        if self.floatButton:
            floatSize = self.floatButton.sizeHint()
        hideSize = QSize(0, 0)
        if self.collapseButton:
            hideSize = self.collapseButton.sizeHint()
        pinSize = QSize(0, 0)
        if self.pinButton:
            pinSize = self.pinButton.sizeHint()
        buttonHeight = max(max(closeSize.height(), floatSize.height()),
                           hideSize.height(), pinSize.height()) + 2
        buttonWidth = closeSize.width() + floatSize.width() + hideSize.width() + pinSize.width()
        titleFontMetrics = q.fontMetrics()
        fontHeight = titleFontMetrics.lineSpacing() + 2 * mw
        height = max(buttonHeight, fontHeight)
        width = buttonWidth + height + 4 * mw + 2 * fw
        if hasFeature(q, QDockWidget.DockWidgetVerticalTitleBar):
            width, height = height, width
        return QSize(width, height)

    def paintEvent(self, event):
        p = QStylePainter(self)
        q = self.parentWidget()
        if hasFeature(q, QDockWidget.DockWidgetVerticalTitleBar):
            fw = 1 or q.isFloating() and q.style().pixelMetric(
                QStyle.PM_DockWidgetFrameWidth, None, q) or 0
            mw = q.style().pixelMetric(QStyle.PM_DockWidgetTitleMargin, None, q)
            titleOpt = QStyleOptionDockWidget()
            titleOpt.initFrom(q)
            titleOpt.verticalTitleBar = True
            titleOpt.rect = QRect(
                QPoint(fw, fw + mw +
                       self.collapseButton.size().height() + self.pinButton.size().height()),
                QSize(
                    self.geometry().width() - (fw * 2),
                    self.geometry().height() - (fw * 2) -
                    mw - self.collapseButton.size().height() - self.pinButton.size().height()))
            titleOpt.title = q.windowTitle()
            titleOpt.closable = hasFeature(q, QDockWidget.DockWidgetClosable)
            titleOpt.floatable = hasFeature(q, QDockWidget.DockWidgetFloatable)
            p.drawControl(QStyle.CE_DockWidgetTitle, titleOpt)
        else:
            fw = q.isFloating() and q.style().pixelMetric(
                QStyle.PM_DockWidgetFrameWidth, None, q) or 0
            mw = q.style().pixelMetric(QStyle.PM_DockWidgetTitleMargin, None, q)
            titleOpt = QStyleOptionDockWidget()
            titleOpt.initFrom(q)
            titleOpt.rect = QRect(
                QPoint(fw + mw +
                       self.collapseButton.size().width() + self.pinButton.size().width(), fw),
                QSize(
                    self.geometry().width() - (fw * 2) -
                    mw - self.collapseButton.size().width() - self.pinButton.size().width(),
                    self.geometry().height() - (fw * 2)))
            titleOpt.title = q.windowTitle()
            titleOpt.closable = hasFeature(q, QDockWidget.DockWidgetClosable)
            titleOpt.floatable = hasFeature(q, QDockWidget.DockWidgetFloatable)
            p.drawControl(QStyle.CE_DockWidgetTitle, titleOpt)

    def resizeEvent(self, event):
        q = self.parentWidget()
        if hasFeature(q, QDockWidget.DockWidgetVerticalTitleBar):
            fh = q.isFloating() and q.style().pixelMetric(
                QStyle.PM_DockWidgetFrameWidth, None, q) or 0
            opt = QStyleOptionDockWidget()
            opt.initFrom(q)
            opt.verticalTitleBar = True
            opt.rect = QRect(
                QPoint(fh, 40),  # self.geometry().height() - (fh * 3)),
                QSize(
                    self.geometry().width() - (fh * 2),
                    fh * 2))
            opt.title = q.windowTitle()
            opt.closable = hasFeature(q, QDockWidget.DockWidgetClosable)
            opt.floatable = hasFeature(q, QDockWidget.DockWidgetFloatable)
            floatRect = q.style().subElementRect(
                QStyle.SE_DockWidgetFloatButton, opt, q)
            if not floatRect.isNull():
                self.floatButton.setGeometry(floatRect)
            closeRect = q.style().subElementRect(
                QStyle.SE_DockWidgetCloseButton, opt, q)
            if not closeRect.isNull():
                self.closeButton.setGeometry(closeRect)
            top = fh
            if not floatRect.isNull():
                top = floatRect.x()
            elif not closeRect.isNull():
                top = closeRect.x()
            size = self.collapseButton.size()
            if not closeRect.isNull():
                size = self.closeButton.size()
            elif not floatRect.isNull():
                size = self.floatButton.size()
            collapseRect = QRect(QPoint(top, fh), size)
            self.collapseButton.setGeometry(collapseRect)
            pinRect = QRect(QPoint(top, fh + collapseRect.height() + 1), size)
            self.pinButton.setGeometry(pinRect)
        else:
            fw = q.isFloating() and q.style().pixelMetric(
                QStyle.PM_DockWidgetFrameWidth, None, q) or 0
            opt = QStyleOptionDockWidget()
            opt.initFrom(q)
            opt.rect = QRect(
                QPoint(fw, fw),
                QSize(
                    self.geometry().width() - (fw * 2),
                    self.geometry().height() - (fw * 2)))
            opt.title = q.windowTitle()
            opt.closable = hasFeature(q, QDockWidget.DockWidgetClosable)
            opt.floatable = hasFeature(q, QDockWidget.DockWidgetFloatable)
            floatRect = q.style().subElementRect(
                QStyle.SE_DockWidgetFloatButton, opt, q)
            if not floatRect.isNull():
                self.floatButton.setGeometry(floatRect)
            closeRect = q.style().subElementRect(
                QStyle.SE_DockWidgetCloseButton, opt, q)
            if not closeRect.isNull():
                self.closeButton.setGeometry(closeRect)
            top = fw
            if not floatRect.isNull():
                top = floatRect.y()
            elif not closeRect.isNull():
                top = closeRect.y()
            size = self.collapseButton.size()
            if not closeRect.isNull():
                size = self.closeButton.size()
            elif not floatRect.isNull():
                size = self.floatButton.size()
            collapseRect = QRect(QPoint(fw, top), size)
            self.collapseButton.setGeometry(collapseRect)
            pinRect = QRect(QPoint(fw + collapseRect.width() + 1, top), size)
            self.pinButton.setGeometry(pinRect)

    def setCollapsed(self, collapsed):
        q = self.parentWidget()
        if q and q.widget() and q.widget().isHidden() != collapsed:
            self.toggleCollapsed()

    def toggleFloating(self):
        q = self.parentWidget()
        q.setFloating(not q.isFloating())

    def toggleCollapsed(self):
        q = self.parentWidget()
        if not q:
            return
        q.toggleCollapsed()
        self.setCollapsedIcon(q.isCollapsed())

    def setCollapsedIcon(self, flag):
        self.collapseButton.setIcon(flag and self.openIcon or self.closeIcon)

    def togglePinned(self, checked):
        self.parent().setPinned(checked)

    def featuresChanged(self, features):
        q = self.parentWidget()
        self.closeButton.setVisible(hasFeature(q, QDockWidget.DockWidgetClosable))
        self.floatButton.setVisible(hasFeature(q, QDockWidget.DockWidgetFloatable))
        # self.resizeEvent(None)


class DockMainWidgetWrapper(QWidget):

    def __init__(self, dockwidget):
        QWidget.__init__(self, dockwidget)
        self.widget = None
        self.hlayout = QHBoxLayout(self)
        self.hlayout.setSpacing(0)
        self.hlayout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(self.hlayout)

    def setWidget(self, widget):
        self.widget = widget
        self.widget_height = widget.height
        self.layout().addWidget(widget)

    def isCollapsed(self):
        return self.widget.isVisible()

    def setCollapsed(self, flag):
        if not flag:
            self.old_size = self.size()
            self.layout().removeWidget(self.widget)
            self.widget.hide()
            if hasFeature(self.parent(), QDockWidget.DockWidgetVerticalTitleBar):
                self.parent().setMaximumWidth(self.parent().width() - self.width())
            else:
                self.parent().setMaximumHeight(self.parent().height() - self.height())
        else:
            self.setFixedSize(self.old_size)
            self.parent().setMinimumSize(QSize(1, 1))
            self.parent().setMaximumSize(QSize(32768, 32768))
            self.widget.show()
            self.layout().addWidget(self.widget)
            self.setMinimumSize(QSize(1, 1))
            self.setMaximumSize(QSize(32768, 32768))


class DockWidget(QDockWidget):

    def __init__(self, *args):
        QDockWidget.__init__(self, *args)
        self.titleBar = DockWidgetTitleBar(self)
        self.setTitleBarWidget(self.titleBar)
        self.mainWidget = None
        self.entered = False
        self.pinned = True
        self.shot = False

    def enterEvent(self, event):
        self.entered = True
        if not self.shot and not self.isPinned() and not self.isFloating():
            self.shot = True
            QTimer.singleShot(500, self.autoshow)
        return QDockWidget.enterEvent(self, event)

    def leaveEvent(self, event):
        self.entered = False
        if not self.shot and not self.isPinned() and not self.isFloating():
            self.shot = True
            QTimer.singleShot(1000, self.autohide)
        return QDockWidget.leaveEvent(self, event)

    def autohide(self):
        self.shot = False
        if not self.entered:
            self.setCollapsed(False)

    def autoshow(self):
        self.shot = False
        if self.entered:
            self.setCollapsed(True)

    def isPinned(self):
        return self.pinned

    def setPinned(self, flag):
        self.pinned = flag

    def setWidget(self, widget):
        self.mainWidget = DockMainWidgetWrapper(self)
        self.mainWidget.setWidget(widget)
        QDockWidget.setWidget(self, self.mainWidget)

    def setCollapsed(self, flag):
        self.mainWidget.setCollapsed(flag)
        self.titleBarWidget().setCollapsedIcon(flag)

    def isCollapsed(self):
        return self.mainWidget.isCollapsed()

    def toggleCollapsed(self):
        self.setCollapsed(not self.isCollapsed())


if __name__ == "__main__":
    import sys
    from PyQt5.QtGui import QTextEdit
    app = QApplication(sys.argv)
    app.setStyle("qtcurve")
    win = QMainWindow()
    dock1 = DockWidget("1st dockwidget", win)
    dock1.setFeatures(dock1.features() | QDockWidget.DockWidgetVerticalTitleBar)
    combo = QComboBox(dock1)
    dock1.setWidget(combo)
    win.addDockWidget(Qt.LeftDockWidgetArea, dock1)
    dock2 = DockWidget("2nd dockwidget")
    dock2.setFeatures(dock1.features() | QDockWidget.DockWidgetVerticalTitleBar)
    button = QPushButton("Hello, world!", dock2)
    dock2.setWidget(button)
    win.addDockWidget(Qt.RightDockWidgetArea, dock2)
    edit = QTextEdit(win)
    win.setCentralWidget(edit)
    win.resize(640, 480)
    win.show()
    app.exec_()
