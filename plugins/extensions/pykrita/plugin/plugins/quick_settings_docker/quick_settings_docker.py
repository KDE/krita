'''
Description: A Python based docker for quickly choosing the brushsize like similar dockers in other drawing programs.

By Wolthera

@package quick_settings_docker
'''

# Importing the relevant dependancies:
import sys
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from krita import *


class QuickSettingsDocker(DockWidget):
# Init the docker

    def __init__(self):
        super().__init__()
        # make base-widget and layout
        widget = QWidget()
        layout = QVBoxLayout()
        widget.setLayout(layout)
        self.setWindowTitle("Quick Settings Docker")
        tabWidget = QTabWidget()

        self.brushSizeTableView = QTableView()
        self.brushSizeTableView.verticalHeader().hide()
        self.brushSizeTableView.horizontalHeader().hide()
        self.brushSizeTableView.setSelectionMode(QTableView.SingleSelection)

        self.brushOpacityTableView = QTableView()
        self.brushOpacityTableView.verticalHeader().hide()
        self.brushOpacityTableView.horizontalHeader().hide()
        self.brushOpacityTableView.setSelectionMode(QTableView.SingleSelection)

        self.brushFlowTableView = QTableView()
        self.brushFlowTableView.verticalHeader().hide()
        self.brushFlowTableView.horizontalHeader().hide()
        self.brushFlowTableView.setSelectionMode(QTableView.SingleSelection)

        tabWidget.addTab(self.brushSizeTableView, "Size")
        tabWidget.addTab(self.brushOpacityTableView, "Opacity")
        tabWidget.addTab(self.brushFlowTableView, "Flow")
        layout.addWidget(tabWidget)
        self.setWidget(widget)  # Add the widget to the docker.

        # amount of columns in each row for all the tables.
        self.columns = 4

        # We want a grid with possible options to select.
        # To do this, we'll make a TableView widget and use a standarditemmodel for the entries.
        # The entries are filled out based on the sizes and opacity lists.

        # Sizes and opacity lists. The former is half-way copied from ptsai, the latter is based on personal experience of useful opacities.
        self.sizesList = [0.7, 1.0, 1.5, 2, 2.5, 3, 3.5, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 20, 25, 30, 35, 40, 50, 60, 70, 80, 100, 120, 160, 200, 250, 300, 350, 400, 450, 500]
        self.opacityList = [0.1, 0.5, 1, 5, 10, 15, 20, 30, 40, 50, 60, 70, 80, 90, 100]
        self.brushSizeModel = QStandardItemModel((len(self.sizesList) / self.columns) + 1, self.columns)
        self.brushOpacityModel = QStandardItemModel((len(self.opacityList) / self.columns) + 1, self.columns)
        self.brushFlowModel = QStandardItemModel((len(self.opacityList) / self.columns) + 1, self.columns)
        self.fillSizesModel()
        self.fillOpacityModel()

        # Now we're done filling out our tables, we connect the views to the functions that'll change the settings.
        self.brushSizeTableView.clicked.connect(self.setBrushSize)
        self.brushOpacityTableView.clicked.connect(self.setBrushOpacity)
        self.brushFlowTableView.clicked.connect(self.setBrushFlow)

    def fillSizesModel(self):
        # First we empty the old model. We might wanna use this function in the future to fill it with the brushmask of the selected brush, but there's currently no API for recognising changes in the current brush nor is there a way to get its brushmask.
        self.brushSizeModel.clear()
        for s in range(len(self.sizesList)):
            # we're gonna itterate over our list, and make a new item for each entry.
            # We need to disable a bunch of stuff to make sure people won't do funny things to our entries.
            item = QStandardItem()
            item.setCheckable(False)
            item.setEditable(False)
            item.setDragEnabled(False)
            item.setText(str(self.sizesList[s]))
            # And from here on we'll make an icon.
            brushImage = QPixmap(32, 32)
            img = brushImage.toImage()
            circlePainter = QPainter()
            img.fill(self.brushSizeTableView.palette().color(QPalette.Base))
            circlePainter.begin(img)
            brush = QBrush(Qt.SolidPattern)
            brush.setColor(self.brushSizeTableView.palette().color(QPalette.Text))
            circlePainter.setBrush(brush)
            circlePainter.pen().setWidth(0)
            brushSize = max(min((self.sizesList[s] / 500) * 100, 32), 1)
            brushSize = brushSize * 0.5
            circlePainter.drawEllipse(QPointF(16, 16), brushSize, brushSize)
            circlePainter.end()
            brushImage = QPixmap.fromImage(img)
            # now we're done with drawing the icon, so we set it on the item.
            item.setIcon(QIcon(brushImage))
            self.brushSizeModel.setItem(s / 4, s % 4, item)
        self.brushSizeTableView.setModel(self.brushSizeModel)
        self.brushSizeTableView.resizeColumnsToContents()

    def fillOpacityModel(self):
        self.brushOpacityModel.clear()
        self.brushFlowModel.clear()
        for s in range(len(self.opacityList)):
            # we're gonna itterate over our list, and make a new item for each entry.
            item = QStandardItem()
            item.setCheckable(False)
            item.setEditable(False)
            item.setDragEnabled(False)
            item.setText(str(self.opacityList[s]))
            brushImage = QPixmap(32, 32)
            img = brushImage.toImage()
            circlePainter = QPainter()
            img.fill(self.brushSizeTableView.palette().color(QPalette.Base))
            circlePainter.begin(img)
            brush = QBrush(Qt.SolidPattern)
            brush.setColor(self.brushSizeTableView.palette().color(QPalette.Text))
            circlePainter.setBrush(brush)
            circlePainter.pen().setWidth(0)
            circlePainter.setOpacity(self.opacityList[s] / 100)
            circlePainter.drawEllipse(QPointF(16, 16), 16, 16)
            circlePainter.end()
            brushImage = QPixmap.fromImage(img)
            item.setIcon(QIcon(brushImage))
            # the flow and opacity models will use virtually the same items, but Qt would like us to make sure we understand
            # these are not really the same items, so hence the clone.
            itemFlow = item.clone()
            self.brushOpacityModel.setItem(s / 4, s % 4, item)
            self.brushFlowModel.setItem(s / 4, s % 4, itemFlow)
        self.brushOpacityTableView.setModel(self.brushOpacityModel)
        self.brushFlowTableView.setModel(self.brushFlowModel)
        self.brushFlowTableView.resizeColumnsToContents()
        self.brushOpacityTableView.resizeColumnsToContents()

    def canvasChanged(self, canvas):
        pass

    @pyqtSlot('QModelIndex')
    def setBrushSize(self, index):
        i = index.column() + (index.row() * self.columns)
        brushSize = self.sizesList[i]
        if Application.activeWindow() and len(Application.activeWindow().views()) > 0:
            Application.activeWindow().views()[0].setBrushSize(brushSize)

    @pyqtSlot('QModelIndex')
    def setBrushOpacity(self, index):
        i = index.column() + (index.row() * self.columns)
        brushOpacity = self.opacityList[i] / 100
        if Application.activeWindow() and len(Application.activeWindow().views()) > 0:
            Application.activeWindow().views()[0].setPaintingOpacity(brushOpacity)

    @pyqtSlot('QModelIndex')
    def setBrushFlow(self, index):
        i = index.column() + (index.row() * self.columns)
        brushOpacity = self.opacityList[i] / 100
        if Application.activeWindow() and len(Application.activeWindow().views()) > 0:
            Application.activeWindow().views()[0].setPaintingFlow(brushOpacity)


# Add docker to the application :)
Application.addDockWidgetFactory(DockWidgetFactory("quick_settings_docker", DockWidgetFactoryBase.DockRight, QuickSettingsDocker))
