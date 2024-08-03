# SPDX-FileCopyrightText: 2024 Timothée Giet <animtim@gmail.com>
# Authors:
#   Timothée Giet <animtim@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

from PyQt5.QtCore import QSize
from PyQt5.QtGui import QIcon, QKeySequence
from PyQt5.QtWidgets import QWidget, QVBoxLayout, QScrollArea, QToolButton, QShortcut
from krita import Krita, DockWidget, DockWidgetFactory, DockWidgetFactoryBase, ManagedColor
from .flow_layout import FlowLayout
from .buttons_settings_dialog import LISTOFTOOLS, LISTOFSIZES, ButtonsSettingsDialog
import ast

DOCKER_NAME = 'Workflow buttons'
DOCKER_ID = 'pykrita_workflow_buttons'
INSTANCE = Krita.instance()

class WorkflowButtons(DockWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle(DOCKER_NAME)
        self.sizeIndex = 2
        self.globalButtonSize = QSize(LISTOFSIZES[self.sizeIndex], LISTOFSIZES[self.sizeIndex])

        # all widget and layout setup of the docker
        mainWidget = QWidget(self)
        self.setWidget(mainWidget)
        mainLayout = QVBoxLayout(mainWidget)

        buttonsScrollArea = QScrollArea(mainWidget)
        buttonsScrollArea.setMinimumSize(self.globalButtonSize)
        self.buttonsWidget = QWidget(buttonsScrollArea)
        buttonsScrollArea.setWidgetResizable(True)
        buttonsScrollArea.setWidget(self.buttonsWidget)

        mainLayout.addWidget(buttonsScrollArea)
        self.buttonsLayout = FlowLayout(self.buttonsWidget)

        # Edit button setup
        self.editButton = QToolButton(self.buttonsWidget)
        self.editButton.setIconSize(self.globalButtonSize)
        self.editButton.setIcon(INSTANCE.icon('properties'))
        self.editButton.setToolTip(i18n("Open workflow buttons settings dialog"))
        self.editButton.clicked.connect(self.openSettingsDialog)
        self.buttonsLayout.addWidget(self.editButton)

        self.buttonsContentList = []
        self.readSettings()
        self.populateButtons()

    def canvasChanged(self, canvas):
        pass

    def readSettings(self):
        savedSize = Application.readSetting("workflowbuttons", "buttonsSize", "")
        if savedSize:
            self.sizeIndex = int(savedSize)
            self.refreshButtonsSize()
        savedList = Application.readSetting("workflowbuttons", "buttons", "")
        if savedList:
            self.buttonsContentList = ast.literal_eval(savedList)

    def writeSettings(self):
        Application.writeSetting("workflowbuttons", "buttonsSize", str(self.sizeIndex))
        Application.writeSetting("workflowbuttons", "buttons", str(self.buttonsContentList))

    def clearButtons(self):
        for widget in self.buttonsWidget.children():
            if isinstance(widget, CustomButton):
                widget.deleteLater()

    def populateButtons(self):
        buttonID = -1
        for buttonToCreate in self.buttonsContentList:
            buttonID += 1
            buttonIcon = QIcon(buttonToCreate["icon"])
            button = CustomButton(self.buttonsWidget, buttonID, buttonToCreate)
            button.setIconSize(self.globalButtonSize)
            button.setIcon(buttonIcon)
            self.buttonsLayout.addWidget(button)

    def refreshButtons(self):
        self.clearButtons()
        self.populateButtons()

    def openSettingsDialog(self):
        newDialog = ButtonsSettingsDialog(self, self.buttonsContentList, self.sizeIndex)
        newDialog.exec_()
        if newDialog.result() == 1:
            self.buttonsContentList = newDialog.buttonsContentList
            self.sizeIndex = newDialog.sizeIndex
            self.refreshButtonsSize()
            self.writeSettings()
            self.refreshButtons()

    def refreshButtonsSize(self):
        self.globalButtonSize = QSize(LISTOFSIZES[self.sizeIndex], LISTOFSIZES[self.sizeIndex])
        self.editButton.setIconSize(self.globalButtonSize)

class CustomButton(QToolButton):
    # Class to define the custom buttons inside the docker
    def __init__(self, parent=None, buttonID=-1, values={}):
        super().__init__(parent)
        self.buttonID = buttonID
        self.values = values
        self.clicked.connect(self.triggeredFunction)
        if self.values["tooltip"]:
            self.setToolTip(values["tooltip"])

    def triggeredFunction(self):
        currentView = INSTANCE.activeWindow().activeView()
        # select given tool
        if self.values["toolIndex"] != "":
            toolIndex = int(self.values["toolIndex"])
            if(toolIndex > 0 and toolIndex < len(LISTOFTOOLS) - 1):
                INSTANCE.action(LISTOFTOOLS[int(self.values["toolIndex"])]["toolName"]).trigger()

        # get list of all presets
        if self.values["presetName"] != "":
            allBrushPresets = INSTANCE.resources('preset')
            # find given preset by name
            brushPreset = allBrushPresets[self.values["presetName"]]
            # select the brush preset
            currentView.setCurrentBrushPreset(brushPreset)

        # set given FGColor
        if self.values["FGColorValues"]["model"] != "":
            colorToSelect = ManagedColor(self.values["FGColorValues"]["model"],
                                         self.values["FGColorValues"]["depth"],
                                         self.values["FGColorValues"]["profile"])
            colorComponents = self.values["FGColorValues"]["components"]
            colorToSelect.setComponents(colorComponents)
            currentView.setForeGroundColor(colorToSelect)

        # set given BGColor
        if self.values["BGColorValues"]["model"] != "":
            colorToSelect = ManagedColor(self.values["BGColorValues"]["model"],
                                         self.values["BGColorValues"]["depth"],
                                         self.values["BGColorValues"]["profile"])
            colorComponents = self.values["BGColorValues"]["components"]
            colorToSelect.setComponents(colorComponents)
            currentView.setBackGroundColor(colorToSelect)

        # run extra script
        if self.values["script"] != "":
            scriptPath = self.values["script"]
            exec(open(scriptPath).read())


dock_widget_factory = DockWidgetFactory(DOCKER_ID,
                                        DockWidgetFactoryBase.DockRight,
                                        WorkflowButtons)

INSTANCE.addDockWidgetFactory(dock_widget_factory)
