# SPDX-FileCopyrightText: 2024 Timothée Giet <animtim@gmail.com>
# Authors:
#   Timothée Giet <animtim@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

try:
    from PyQt6.QtCore import QSize
    from PyQt6.QtGui import QIcon, QPixmap, QAction
    from PyQt6.QtWidgets import QWidget, QVBoxLayout, QHBoxLayout, QScrollArea, QToolButton, QPushButton, QToolBar, QSizePolicy
except:
    from PyQt5.QtCore import QSize
    from PyQt5.QtGui import QIcon, QPixmap
    from PyQt5.QtWidgets import QWidget, QVBoxLayout, QHBoxLayout, QScrollArea, QToolButton, QPushButton, QToolBar, QAction, QSizePolicy
from krita import Krita, DockWidget, DockWidgetFactory, DockWidgetFactoryBase, ManagedColor
from builtins import i18n, i18nc, Application
from .flow_layout import FlowLayout
from .buttons_settings_dialog import LISTOFTOOLS, LISTOFSIZES, ButtonsSettingsDialog
import ast

DOCKER_ID = 'pykrita_workflow_buttons'
INSTANCE = Krita.instance()

class WorkflowButtons(DockWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle(i18nc("@title:window", "Workflow Buttons"))
        self.sizeIndex = 2
        self.globalButtonSize = QSize(LISTOFSIZES[self.sizeIndex], LISTOFSIZES[self.sizeIndex])
        self.settingsButtonPosition = 0

        # all widget and layout setup of the docker
        mainWidget = QWidget(self)
        self.setWidget(mainWidget)
        mainLayout = QVBoxLayout(mainWidget)
        mainLayout.setSpacing(2)
        mainLayout.setContentsMargins(0, 0, 0, 0)

        buttonsScrollArea = QScrollArea(mainWidget)
        buttonsScrollArea.setMinimumSize(self.globalButtonSize)
        self.buttonsWidget = QWidget(buttonsScrollArea)
        buttonsScrollArea.setWidgetResizable(True)
        buttonsScrollArea.setWidget(self.buttonsWidget)

        mainLayout.addWidget(buttonsScrollArea)

        self.buttonsLayout = FlowLayout(self.buttonsWidget)
        self.buttonsLayout.setSpacing(2)
        self.buttonsLayout.setContentsMargins(0, 0, 0, 0)

        self.bottomLayout = QHBoxLayout()
        self.bottomLayout.setSpacing(2)
        self.bottomLayout.setContentsMargins(0, 0, 0, 0)
        mainLayout.addLayout(self.bottomLayout)

        barSpacer = QWidget(mainWidget)
        barSpacer.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Ignored)
        self.bottomLayout.addWidget(barSpacer)

        self.bottomBarButton = QToolButton(mainWidget)
        self.bottomBarButton.setAutoRaise(True)
        self.bottomBarButton.setIconSize(QSize(22,22))

        self.settingsAction = QAction(self)
        self.settingsAction.setIcon(INSTANCE.icon('view-choose-22'))
        self.settingsAction.setToolTip(i18n("Open workflow buttons settings dialog"))
        self.settingsAction.triggered.connect(self.openSettingsDialog)

        self.bottomBarButton.setDefaultAction(self.settingsAction)
        self.bottomLayout.addWidget(self.bottomBarButton)

        # Inline edit button setup
        self.editButton = QToolButton(self.buttonsWidget)
        self.editButton.setIconSize(self.globalButtonSize)
        self.editButton.setDefaultAction(self.settingsAction)
        self.buttonsLayout.addWidget(self.editButton)

        self.buttonsContentList = []
        self.readSettings()
        self.refreshSettingsButtonPosition()
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
        savedSettingsButtonPosition = Application.readSetting("workflowbuttons", "settingsButtonPosition", "")
        if savedSettingsButtonPosition:
            self.settingsButtonPosition = int(savedSettingsButtonPosition)

    def writeSettings(self):
        Application.writeSetting("workflowbuttons", "buttonsSize", str(self.sizeIndex))
        Application.writeSetting("workflowbuttons", "buttons", str(self.buttonsContentList))
        Application.writeSetting("workflowbuttons", "settingsButtonPosition", str(self.settingsButtonPosition))

    def refreshSettingsButtonPosition(self):
        if self.settingsButtonPosition == 0:
            self.bottomBarButton.setVisible(True)
            self.editButton.setVisible(False)
        elif self.settingsButtonPosition == 1:
            self.bottomBarButton.setVisible(False)
            self.editButton.setVisible(True)

    def clearButtons(self):
        for widget in self.buttonsWidget.children():
            if isinstance(widget, CustomButton):
                widget.deleteLater()

    def populateButtons(self):
        buttonID = -1
        allBrushPresets = INSTANCE.resources('preset')
        for buttonToCreate in self.buttonsContentList:
            buttonID += 1
            if buttonToCreate["iconMode"] == 0:
                buttonIcon = QIcon(buttonToCreate["icon"])
            elif buttonToCreate["iconMode"] == 1 and buttonToCreate["toolIndex"] != 0:
                buttonIcon = INSTANCE.icon(LISTOFTOOLS[buttonToCreate["toolIndex"]]["toolIcon"])
            elif buttonToCreate["iconMode"] == 2 and buttonToCreate["presetName"] != "":
                brushPreset = allBrushPresets[buttonToCreate["presetName"]]
                buttonIcon = QIcon(QPixmap.fromImage(brushPreset.image()))
            else:
                buttonIcon = QIcon()
            button = CustomButton(self.buttonsWidget, buttonID, buttonToCreate)
            button.setIconSize(self.globalButtonSize)
            button.setIcon(buttonIcon)
            self.buttonsLayout.addWidget(button)

    def refreshButtons(self):
        self.clearButtons()
        self.populateButtons()

    def openSettingsDialog(self):
        newDialog = ButtonsSettingsDialog(self, self.buttonsContentList, self.sizeIndex, self.settingsButtonPosition)
        newDialog.exec()
        if newDialog.result() == 1:
            self.buttonsContentList = newDialog.buttonsContentList
            self.sizeIndex = newDialog.sizeIndex
            self.settingsButtonPosition = newDialog.settingsButtonPosition
            self.refreshSettingsButtonPosition()
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
            if(toolIndex > 0 and toolIndex < len(LISTOFTOOLS)):
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
                                        DockWidgetFactoryBase.DockPosition.DockRight,
                                        WorkflowButtons)

INSTANCE.addDockWidgetFactory(dock_widget_factory)
