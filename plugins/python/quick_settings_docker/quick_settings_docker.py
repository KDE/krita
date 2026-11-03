# By Wolthera(originally)

# SPDX-License-Identifier: CC0-1.0

'''A Python based docker for quickly choosing the brushsize like
similar dockers in other drawing programs.

@package quick_settings_docker

'''

# Importing the relevant dependencies:
try:
    from PyQt6.QtCore import pyqtSlot, Qt, QPointF
    from PyQt6.QtGui import (QStandardItem, QStandardItemModel, QPainter, QPalette,
                            QPixmap, QImage, QBrush, QPen, QIcon)
    from PyQt6.QtWidgets import (
        QWidget, QTabWidget, QListView, QVBoxLayout,
        QInputDialog, QMenu, QMessageBox
    )
except:
    from PyQt5.QtCore import pyqtSlot, Qt, QPointF
    from PyQt5.QtGui import (QStandardItem, QStandardItemModel, QPainter, QPalette,
                            QPixmap, QImage, QBrush, QPen, QIcon)
    from PyQt5.QtWidgets import (
        QWidget, QTabWidget, QListView, QVBoxLayout,
        QInputDialog, QMenu, QMessageBox
    )
from krita import DockWidget
from builtins import i18n, Application


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
        self.brushSizeTableView.setViewMode(QListView.ViewMode.IconMode)
        self.brushSizeTableView.setMovement(QListView.Movement.Static)
        self.brushSizeTableView.setResizeMode(QListView.ResizeMode.Adjust)
        self.brushSizeTableView.setUniformItemSizes(True)
        self.brushSizeTableView.setSelectionMode(QListView.SelectionMode.SingleSelection)

        self.brushOpacityTableView = QListView()
        self.brushOpacityTableView.setViewMode(QListView.ViewMode.IconMode)
        self.brushOpacityTableView.setMovement(QListView.Movement.Static)
        self.brushOpacityTableView.setResizeMode(QListView.ResizeMode.Adjust)
        self.brushOpacityTableView.setUniformItemSizes(True)
        self.brushOpacityTableView.setSelectionMode(QListView.SelectionMode.SingleSelection)

        self.brushFlowTableView = QListView()
        self.brushFlowTableView.setViewMode(QListView.ViewMode.IconMode)
        self.brushFlowTableView.setMovement(QListView.Movement.Static)
        self.brushFlowTableView.setResizeMode(QListView.ResizeMode.Adjust)
        self.brushFlowTableView.setUniformItemSizes(True)
        self.brushFlowTableView.setSelectionMode(QListView.SelectionMode.SingleSelection)

        tabWidget.addTab(self.brushSizeTableView, i18n("Size"))
        tabWidget.addTab(self.brushOpacityTableView, i18n("Opacity"))
        tabWidget.addTab(self.brushFlowTableView, i18n("Flow"))

        # Tab index variables
        self.sizeTabIndex = 0
        self.opacityTabIndex = 1
        self.flowTabIndex = 2
        # keep track of the selected tab
        self.currentTabIndex = self.sizeTabIndex
        tabWidget.currentChanged.connect(self.onTabChanged)

        layout.addWidget(tabWidget)
        self.setWidget(widget)  # Add the widget to the docker.

        # amount of columns in each row for all the tables.

        # We want a grid with possible options to select.  To do this,
        # we'll make a ListView widget and use a standarditemmodel for
        # the entries. The entries are filled out based on the sizes
        # and opacity lists.

        # Sizes,opacity and flow lists. The former is half-way copied from
        # ptsai, the latter is based on personal experience of useful
        # opacities.
        sizes = self.getPresetsFor("size")
        opacity = self.getPresetsFor("opacity")
        flow = self.getPresetsFor("flow")
        # initialize the lists to the previously saved values else use the hardcoded lists
        self.sizesList = sizes if sizes else [
            1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16,
            20, 25, 30, 35, 40, 50, 60, 70, 80, 100, 120, 160, 200, 250, 300,
            350, 400, 450, 500]
        self.opacityList = opacity if opacity else [
            1, 5, 10, 15, 20, 30, 40, 50, 60, 70, 80, 90, 100]
        self.flowList = flow if flow else [
            20, 25, 33, 50, 66, 75, 80, 90, 100]
        self.brushSizeModel = QStandardItemModel()
        self.brushOpacityModel = QStandardItemModel()
        self.brushFlowModel = QStandardItemModel()
        self.fillSizesModel()
        self.fillOpacityModel()
        self.fillFlowModel()

        # Now we're done filling out our tables, we connect the views
        # to the functions that'll change the settings.
        self.brushSizeTableView.clicked.connect(self.setBrushSize)
        self.brushOpacityTableView.clicked.connect(self.setBrushOpacity)
        self.brushFlowTableView.clicked.connect(self.setBrushFlow)

        # Enable right-click context menus
        try:
            contextMenuPolicy = Qt.ContextMenuPolicy.CustomContextMenu
        except AttributeError:
            contextMenuPolicy = Qt.CustomContextMenu

        for tableView in (self.brushSizeTableView, self.brushOpacityTableView,
            self.brushFlowTableView):

            tableView.setContextMenuPolicy(contextMenuPolicy)
            tableView.customContextMenuRequested.connect(
                lambda pos, view=tableView:
                    self.handlePresetContextMenu(view, pos)
            )

    def handlePresetContextMenu(self, tableView, pos):
        index = tableView.indexAt(pos)
        # Determine which tab/list was right-clicked
        if tableView == self.brushSizeTableView:
            self.currentTabIndex = self.sizeTabIndex

        elif tableView == self.brushOpacityTableView:
            self.currentTabIndex = self.opacityTabIndex

        else:
            self.currentTabIndex = self.flowTabIndex

        # Create context menu
        menu = QMenu(self)
        addAction = menu.addAction(i18n("Add Preset"))
        deleteAction = None
        # Only allow deletion when clicking an existing preset
        if index.isValid():
            deleteAction = menu.addAction(i18n("Delete Preset"))

        # Display menu at cursor position
        globalPos = tableView.viewport().mapToGlobal(pos)
        try:
            action = menu.exec(globalPos)
        except AttributeError:
            action = menu.exec_(globalPos)

        # Handle selection
        if action == addAction:
            self.addPreset()
        elif deleteAction is not None and action == deleteAction:
            self.removePreset(index.row())

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
            img = QImage(64, 64, QImage.Format.Format_RGBA8888)
            circlePainter = QPainter()
            img.fill(Qt.GlobalColor.transparent)
            circlePainter.begin(img)
            brush = QBrush(Qt.BrushStyle.SolidPattern)
            brush.setColor(
                self.brushSizeTableView.palette().color(QPalette.ColorRole.Text))
            circlePainter.setBrush(brush)
            circlePainter.setPen(QPen(QBrush(Qt.GlobalColor.transparent), 0))
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
        for s in range(len(self.opacityList)):
            # we're gonna iterate over our list, and make a new item
            # for each entry.
            item = QStandardItem()
            item.setCheckable(False)
            item.setEditable(False)
            item.setDragEnabled(False)
            item.setText(str(self.opacityList[s])+" %")
            brushImage = QPixmap(64, 64)
            img = QImage(64, 64, QImage.Format.Format_RGBA8888)
            circlePainter = QPainter()
            img.fill(Qt.GlobalColor.transparent)
            circlePainter.begin(img)
            brush = QBrush(Qt.BrushStyle.SolidPattern)
            brush.setColor(
                self.brushSizeTableView.palette().color(QPalette.ColorRole.Text))
            circlePainter.setBrush(brush)
            circlePainter.setPen(QPen(QBrush(Qt.GlobalColor.transparent), 0))
            circlePainter.setOpacity(float(self.opacityList[s]) / 100.0)
            circlePainter.drawEllipse(QPointF(32, 32), 32, 32)
            circlePainter.end()
            brushImage = QPixmap.fromImage(img)
            item.setIcon(QIcon(brushImage))
            # now we are done with the icon for opacity, so we set it to the item.
            self.brushOpacityModel.appendRow(item)
        self.brushOpacityTableView.setModel(self.brushOpacityModel)

    def fillFlowModel(self):
        self.brushFlowModel.clear()
        for s in range(len(self.flowList)):
            # we're gonna iterate over our list, and make a new item
            # for each entry.
            item = QStandardItem()
            item.setCheckable(False)
            item.setEditable(False)
            item.setDragEnabled(False)
            item.setText(str(self.flowList[s]) + " %")
            # Create icon canvas to create a different icon for flow using overlapping circles
            brushImage = QPixmap(64, 64)
            brushImage.fill(Qt.GlobalColor.transparent)
            painter = QPainter(brushImage)
            pen = QPen(self.brushFlowTableView.palette().color(QPalette.ColorRole.Text))
            pen.setWidth(18)
            pen.setCapStyle(Qt.PenCapStyle.RoundCap)
            painter.setPen(pen)
            # Non-linear mapping for visibility
            flow_factor = self.flowList[s] / 100.0
            # Strength of paint flow
            strength = flow_factor ** 0.7
            # Number of dabs
            steps = 7
            for i in range(steps):
                # Adjust the opacity of each dab based on its number. This makes it so that the icon
                # becomes a line which increases in opacity, which should give a good example of flow
                painter.setOpacity(strength * ((i + 1) / steps))
                # we draw the dabs close to each other to simulate a line
                painter.drawPoint(12 + i * 6, 32)
            # now we are done with the icon, so we set it to the item.
            item.setIcon(QIcon(brushImage))
            self.brushFlowModel.appendRow(item)
            painter.end()
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
        brushFlow = float(self.flowList[i]) / 100.0
        window = Application.activeWindow()
        if window and window.views():
            window.views()[0].setPaintingFlow(brushFlow)

    # get preset from the saved preference setting
    def getPresetsFor(self, tab=""):
        # read the list that is saved as "val1;val2;val3;..."
        rawList = Application.readSetting("quicksettings", str(tab), "")
        if not rawList:
            return None
        try:
            # if a list is found then it converts floats to ints and then splits the values
            return sorted(set([int(float(x.strip())) for x in rawList.split(";") if x.strip()]))
        except Exception as e:
            print(f"Error parsing saved presets for '{tab}':", e)
            return None

    # Updates the current tab index to determine which list to modify based on active tab
    def onTabChanged(self, index):
        self.currentTabIndex = index

    def addPreset(self):
        if self.currentTabIndex == self.sizeTabIndex:
            maxVal = 1000
            label = i18n("Enter brush size (px):")
            targetList = self.sizesList
            saveFile = "size"

        elif self.currentTabIndex == self.opacityTabIndex:
            maxVal = 100
            label = i18n("Enter opacity (%):")
            targetList = self.opacityList
            saveFile = "opacity"

        elif self.currentTabIndex == self.flowTabIndex:
            maxVal = 100
            label = i18n("Enter flow (%):")
            targetList = self.flowList
            saveFile = "flow"

        else:
            return

        # Ask for the new value
        value, ok = QInputDialog.getInt(
            self,
            i18n("Add Preset"), label,
            1,
            1,
            maxVal,
            1
        )

        if not ok:
            return

        value = int(value)

        # Add value if it doesn't already exist
        if value not in targetList:
            targetList.append(value)
            targetList.sort()

        # Save updated presets
        content = ";".join(str(x) for x in sorted(set(targetList)))

        # Refresh the correct list
        if self.currentTabIndex == self.sizeTabIndex:
            self.fillSizesModel()

        elif self.currentTabIndex == self.opacityTabIndex:
            self.fillOpacityModel()

        elif self.currentTabIndex == self.flowTabIndex:
            self.fillFlowModel()

        Application.writeSetting("quicksettings", saveFile, content)

    def removePreset(self, index):
        if self.currentTabIndex == self.sizeTabIndex:
            targetList = self.sizesList
            saveFile = "size"

        elif self.currentTabIndex == self.opacityTabIndex:
            targetList = self.opacityList
            saveFile = "opacity"

        elif self.currentTabIndex == self.flowTabIndex:
            targetList = self.flowList
            saveFile = "flow"

        else:
            return

        if index < 0 or index >= len(targetList):
            return

        value = targetList[index]

        try:
            yesButton = QMessageBox.StandardButton.Yes
            noButton = QMessageBox.StandardButton.No
        except AttributeError:
            yesButton = QMessageBox.Yes
            noButton = QMessageBox.No

        reply = QMessageBox.question(
            self,
            i18n("Delete Preset"),
            i18n("Are you sure you want to delete this preset?"),
            yesButton | noButton,
            noButton
        )
        if reply != yesButton:
            return

        targetList.remove(value)

        # Save updated presets
        content = ";".join(str(x) for x in sorted(set(targetList)))

        # Refresh the correct list
        if self.currentTabIndex == self.sizeTabIndex:
            self.fillSizesModel()

        elif self.currentTabIndex == self.opacityTabIndex:
            self.fillOpacityModel()

        elif self.currentTabIndex == self.flowTabIndex:
            self.fillFlowModel()

        Application.writeSetting("quicksettings", saveFile, content)

