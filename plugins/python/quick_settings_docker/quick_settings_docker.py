# By Wolthera(originally)

# SPDX-License-Identifier: CC0-1.0

'''A Python based docker for quickly choosing the brushsize like
similar dockers in other drawing programs.

@package quick_settings_docker

'''

# Importing the relevant dependencies:
from PyQt5.QtCore import pyqtSlot, Qt, QPointF
from PyQt5.QtGui import (QStandardItem, QStandardItemModel, QPainter, QPalette,
                         QPixmap, QImage, QBrush, QPen, QIcon)
from PyQt5.QtWidgets import QWidget, QTabWidget, QListView, QVBoxLayout
from krita import DockWidget


class QuickSettingsDocker(DockWidget):
    # Init the docker

    def __init__(self):
        super(QuickSettingsDocker, self).__init__()
        # make base-widget and layout
        widget = QWidget()
        layout = QVBoxLayout()
        widget.setLayout(layout)
        self.setWindowTitle(i18n("Quick Settings Docker"))
        tabWidget = QTabWidget()

        self.brushSizeTableView = QListView()
        self.brushSizeTableView.setViewMode(QListView.IconMode)
        self.brushSizeTableView.setMovement(QListView.Static)
        self.brushSizeTableView.setResizeMode(QListView.Adjust)
        self.brushSizeTableView.setUniformItemSizes(True)
        self.brushSizeTableView.setSelectionMode(QListView.SingleSelection)

        self.brushOpacityTableView = QListView()
        self.brushOpacityTableView.setViewMode(QListView.IconMode)
        self.brushOpacityTableView.setMovement(QListView.Static)
        self.brushOpacityTableView.setResizeMode(QListView.Adjust)
        self.brushOpacityTableView.setUniformItemSizes(True)
        self.brushOpacityTableView.setSelectionMode(QListView.SingleSelection)

        self.brushFlowTableView = QListView()
        self.brushFlowTableView.setViewMode(QListView.IconMode)
        self.brushFlowTableView.setMovement(QListView.Static)
        self.brushFlowTableView.setResizeMode(QListView.Adjust)
        self.brushFlowTableView.setUniformItemSizes(True)
        self.brushFlowTableView.setSelectionMode(QListView.SingleSelection)

        tabWidget.addTab(self.brushSizeTableView, i18n("Size"))
        tabWidget.addTab(self.brushOpacityTableView, i18n("Opacity"))
        tabWidget.addTab(self.brushFlowTableView, i18n("Flow"))
        layout.addWidget(tabWidget)
        self.setWidget(widget)  # Add the widget to the docker.

        # amount of columns in each row for all the tables.

        # We want a grid with possible options to select.  To do this,
        # we'll make a ListView widget and use a standarditemmodel for
        # the entries. The entries are filled out based on the sizes
        # and opacity lists.

        # Sizes and opacity lists. The former is half-way copied from
        # ptsai, the latter is based on personal experience of useful
        # opacities.
        self.sizesList = [
            0.7, 1.0, 1.5, 2, 2.5, 3, 3.5, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16,
            20, 25, 30, 35, 40, 50, 60, 70, 80, 100, 120, 160, 200, 250, 300,
            350, 400, 450, 500]
        self.opacityList = [
            0.1, 0.5, 1, 5, 10, 15, 20, 30, 40, 50, 60, 70, 80, 90, 100]
        self.brushSizeModel = QStandardItemModel()
        self.brushOpacityModel = QStandardItemModel()
        self.brushFlowModel = QStandardItemModel()
        self.fillSizesModel()
        self.fillOpacityModel()

        # Now we're done filling out our tables, we connect the views
        # to the functions that'll change the settings.
        self.brushSizeTableView.clicked.connect(self.setBrushSize)
        self.brushOpacityTableView.clicked.connect(self.setBrushOpacity)
        self.brushFlowTableView.clicked.connect(self.setBrushFlow)

    def fillSizesModel(self):
        # First we empty the old model. We might wanna use this
        # function in the future to fill it with the brushmask of the
        # selected brush, but there's currently no API for recognising
        # changes in the current brush nor is there a way to get its
        # brushmask.
        self.brushSizeModel.clear()
        for s in range(len(self.sizesList)):
            # we're gonna iterate over our list, and make a new item
            # for each entry.  We need to disable a bunch of stuff to
            # make sure people won't do funny things to our entries.
            item = QStandardItem()
            item.setCheckable(False)
            item.setEditable(False)
            item.setDragEnabled(False)
            item.setText(str(self.sizesList[s])+" px")
            # And from here on we'll make an icon.
            brushImage = QPixmap(64, 64)
            img = QImage(64, 64, QImage.Format_RGBA8888)
            circlePainter = QPainter()
            img.fill(Qt.transparent)
            circlePainter.begin(img)
            brush = QBrush(Qt.SolidPattern)
            brush.setColor(
                self.brushSizeTableView.palette().color(QPalette.Text))
            circlePainter.setBrush(brush)
            circlePainter.setPen(QPen(QBrush(Qt.transparent), 0))
            brushSize = max(min(self.sizesList[s], 64), 1)
            brushSize = brushSize * 0.5
            circlePainter.drawEllipse(QPointF(32, 32), brushSize, brushSize)
            circlePainter.end()
            brushImage = QPixmap.fromImage(img)
            # now we're done with drawing the icon, so we set it on the item.
            item.setIcon(QIcon(brushImage))
            self.brushSizeModel.appendRow(item)
        self.brushSizeTableView.setModel(self.brushSizeModel)

    def fillOpacityModel(self):
        self.brushOpacityModel.clear()
        self.brushFlowModel.clear()
        for s in range(len(self.opacityList)):
            # we're gonna iterate over our list, and make a new item
            # for each entry.
            item = QStandardItem()
            item.setCheckable(False)
            item.setEditable(False)
            item.setDragEnabled(False)
            item.setText(str(self.opacityList[s])+" %")
            brushImage = QPixmap(64, 64)
            img = QImage(64, 64, QImage.Format_RGBA8888)
            circlePainter = QPainter()
            img.fill(Qt.transparent)
            circlePainter.begin(img)
            brush = QBrush(Qt.SolidPattern)
            brush.setColor(
                self.brushSizeTableView.palette().color(QPalette.Text))
            circlePainter.setBrush(brush)
            circlePainter.setPen(QPen(QBrush(Qt.transparent), 0))
            circlePainter.setOpacity(float(self.opacityList[s]) / 100.0)
            circlePainter.drawEllipse(QPointF(32, 32), 32, 32)
            circlePainter.end()
            brushImage = QPixmap.fromImage(img)
            item.setIcon(QIcon(brushImage))
            # the flow and opacity models will use virtually the same
            # items, but Qt would like us to make sure we understand
            # these are not really the same items, so hence the clone.
            itemFlow = item.clone()
            self.brushOpacityModel.appendRow(item)
            self.brushFlowModel.appendRow(itemFlow)
        self.brushOpacityTableView.setModel(self.brushOpacityModel)
        self.brushFlowTableView.setModel(self.brushFlowModel)

    def canvasChanged(self, canvas):
        pass

    @pyqtSlot('QModelIndex')
    def setBrushSize(self, index):
        i = index.row()
        brushSize = self.sizesList[i]
        window = Application.activeWindow()
        if window and window.views():
            window.views()[0].setBrushSize(brushSize)

    @pyqtSlot('QModelIndex')
    def setBrushOpacity(self, index):
        i = index.row()
        brushOpacity = float(self.opacityList[i]) / 100.0
        window = Application.activeWindow()
        if window and window.views():
            window.views()[0].setPaintingOpacity(brushOpacity)

    @pyqtSlot('QModelIndex')
    def setBrushFlow(self, index):
        i = index.row()
        brushOpacity = float(self.opacityList[i]) / 100.0
        window = Application.activeWindow()
        if window and window.views():
            window.views()[0].setPaintingFlow(brushOpacity)
