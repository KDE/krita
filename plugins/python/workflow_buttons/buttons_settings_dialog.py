# SPDX-FileCopyrightText: 2024 Timothée Giet <animtim@gmail.com>
# Authors:
#   Timothée Giet <animtim@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

from PyQt5.QtCore import QSize
from PyQt5.QtGui import QIcon, QPixmap, QColor, QPen, QBrush, QPainter, QImageReader
from PyQt5.QtWidgets import (QDialog, QVBoxLayout, QHBoxLayout, QWidget, QScrollArea, QPushButton,
                             QToolButton, QLabel, QLineEdit, QComboBox, QDialogButtonBox,
                             QFileDialog, QFrame, QWidget, QSizePolicy)
from krita import Krita, PresetChooser, ManagedColor
from .flow_layout import FlowLayout
import copy

INSTANCE = Krita.instance()

LISTOFTOOLS = [
    { "toolName": "", "toolIcon": "", "toolString": "" },
    { "toolName": "InteractionTool", "toolIcon": "select", "toolString": "Select Shapes Tool" },
    { "toolName": "SvgTextToo", "toolIcon": "draw-text", "toolString": "Text Tool" },
    { "toolName": "PathTool", "toolIcon": "shape_handling", "toolString": "Edit Shapes Tool" },
    { "toolName": "KarbonCalligraphyTool", "toolIcon": "calligraphy", "toolString": "Calligraphy" },
    { "toolName": "KritaShape/KisToolBrush", "toolIcon": "krita_tool_freehand", "toolString": "Freehand Brush Tool" },
    { "toolName": "KritaShape/KisToolLine", "toolIcon": "krita_tool_line", "toolString": "Line Tool" },
    { "toolName": "KritaShape/KisToolRectangle", "toolIcon": "krita_tool_rectangle", "toolString": "Rectangle Tool" },
    { "toolName": "KritaShape/KisToolEllipse", "toolIcon": "krita_tool_ellipse", "toolString": "Ellipse Tool" },
    { "toolName": "KisToolPolygon", "toolIcon": "krita_tool_polygon", "toolString": "Polygon Tool" },
    { "toolName": "KisToolPolyline", "toolIcon": "polyline", "toolString": "Polyline Tool" },
    { "toolName": "KisToolPath", "toolIcon": "krita_draw_path", "toolString": "Bezier Curve Tool" },
    { "toolName": "KisToolPencil", "toolIcon": "krita_tool_freehandvector", "toolString": "Freehand Path Tool" },
    { "toolName": "KritaShape/KisToolDyna", "toolIcon": "krita_tool_dyna", "toolString": "Dynamic Brush Tool" },
    { "toolName": "KritaShape/KisToolMultiBrush", "toolIcon": "krita_tool_multihand", "toolString": "Multibrush Tool" },
    { "toolName": "KisToolTransform", "toolIcon": "krita_tool_transform", "toolString": "Transform Tool" },
    { "toolName": "KritaTransform/KisToolMove", "toolIcon": "krita_tool_move", "toolString": "Move Tool" },
    { "toolName": "KisToolCrop", "toolIcon": "tool_crop", "toolString": "Crop Tool" },
    { "toolName": "KritaFill/KisToolGradient", "toolIcon": "krita_tool_gradient", "toolString": "Gradient Tool" },
    { "toolName": "KritaSelected/KisToolColorSampler", "toolIcon": "krita_tool_color_sampler", "toolString": "Color Sampler" },
    { "toolName": "KritaShape/KisToolLazyBrush", "toolIcon": "krita_tool_lazybrush", "toolString": "Colorize Mask Tool" },
    { "toolName": "KritaShape/KisToolSmartPatch", "toolIcon": "krita_tool_smart_patch", "toolString": "Smart Patch Tool" },
    { "toolName": "KritaFill/KisToolFill", "toolIcon": "krita_tool_color_fill", "toolString": "Fill Tool" },
    { "toolName": "KisToolEncloseAndFill", "toolIcon": "krita_tool_enclose_and_fill", "toolString": "Enclose and Fill Tool" },
    { "toolName": "KisAssistantTool", "toolIcon": "krita_tool_assistant", "toolString": "Assistant Tool" },
    { "toolName": "KritaShape/KisToolMeasure", "toolIcon": "krita_tool_measure", "toolString": "Measurement Tool" },
    { "toolName": "ToolReferenceImages", "toolIcon": "krita_tool_reference_images", "toolString": "Reference Images Tool" },
    { "toolName": "KisToolSelectRectangular", "toolIcon": "tool_rect_selection", "toolString": "Rectangular Selection Tool" },
    { "toolName": "KisToolSelectElliptical", "toolIcon": "tool_elliptical_selection", "toolString": "Elliptical Selection Tool" },
    { "toolName": "KisToolSelectPolygonal", "toolIcon": "tool_polygonal_selection", "toolString": "Polygonal Selection Tool" },
    { "toolName": "KisToolSelectOutline", "toolIcon": "tool_outline_selection", "toolString": "Freehand Selection Tool" },
    { "toolName": "KisToolSelectContiguous", "toolIcon": "tool_contiguous_selection", "toolString": "Contiguous Selection Tool" },
    { "toolName": "KisToolSelectSimilar", "toolIcon": "tool_similar_selection", "toolString": "Similar Color Selection Tool" },
    { "toolName": "KisToolSelectPath", "toolIcon": "tool_path_selection", "toolString": "Bezier Curve Selection Tool" },
    { "toolName": "KisToolSelectMagnetic", "toolIcon": "tool_magnetic_selection", "toolString": "Magnetic Selection Tool" },
    { "toolName": "ZoomTool", "toolIcon": "tool_zoom", "toolString": "Zoom Tool" },
    { "toolName": "PanTool", "toolIcon": "tool_pan", "toolString": "Pan Tool" }
    ]

LISTOFSIZES = [16, 22, 32, 48, 64]

LISTOFICONMODES = [i18n("Custom icon"), i18n("Tool icon"), i18n("Brush preset icon")]

class ButtonsSettingsDialog(QDialog):
    def __init__(self, parent=None, buttonsContentList=[], sizeIndex=2, settingsButtonPosition=0):
        super().__init__(parent)
        self.setWindowTitle(i18n("Workflow Buttons settings"))
        self.allBrushPresets = INSTANCE.resources('preset')

        mainLayout = QVBoxLayout(self)

        self.buttonsContentList = copy.deepcopy(buttonsContentList)
        self.defaultButtonContent = { "iconMode": 0, "icon" : "", "tooltip": "", "toolIndex" : 0, "presetName" : "", "FGColorValues" : { "model":"","depth":"","components":[],"profile":"" }, "BGColorValues" : { "model":"","depth":"","components":[],"profile":"" }, "script" : "" }

        # button ID starts at 1, 0 is no button selected when list is empty
        self.selectedButtonID = 0
        # if list is not empty, start with last button selected
        if len(self.buttonsContentList) > 0:
            self.selectedButtonID = len(self.buttonsContentList)

        # list of custom buttons
        buttonsScrollArea = QScrollArea(self)
        self.buttonsWidget = QWidget(self)
        self.buttonsLayout = FlowLayout(self.buttonsWidget)
        buttonsScrollArea.setWidgetResizable(True)
        buttonsScrollArea.setWidget(self.buttonsWidget)

        mainLayout.addWidget(buttonsScrollArea)

        self.sizeIndex = sizeIndex
        self.globalButtonSize = QSize(LISTOFSIZES[self.sizeIndex], LISTOFSIZES[self.sizeIndex])

        self.settingsButtonPosition = settingsButtonPosition

        layoutForSelectorControls = QHBoxLayout()
        controlsSize = QSize(22,22)

        # label to show selected button id:
        selectedButtonIDLabelTitle = QLabel(i18n("Selected button:"), self)
        self.selectedButtonIDLabel = QLabel(str(self.selectedButtonID), self)
        layoutForSelectorControls.addWidget(selectedButtonIDLabelTitle)
        layoutForSelectorControls.addWidget(self.selectedButtonIDLabel)

        spacer1 = QWidget(self)
        spacer1.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        layoutForSelectorControls.addWidget(spacer1)

        # button to add a custom button
        addButtonButton = QToolButton(self)
        addButtonButton.setIconSize(controlsSize)
        addButtonButton.setIcon(INSTANCE.icon('addlayer'))
        addButtonButton.setToolTip(i18n("Add a workflow button"))
        addButtonButton.clicked.connect(self.addButton)
        layoutForSelectorControls.addWidget(addButtonButton)

        # buttons to move selected button to left/right
        moveButtonLeftButton = QToolButton(self)
        moveButtonLeftButton.setIconSize(controlsSize)
        moveButtonLeftButton.setIcon(INSTANCE.icon('arrow-left'))
        moveButtonLeftButton.setToolTip(i18n("Move selected workflow button to the left"))
        moveButtonLeftButton.clicked.connect(self.moveButtonLeft)
        layoutForSelectorControls.addWidget(moveButtonLeftButton)
        moveButtonRightButton = QToolButton(self)
        moveButtonRightButton.setIconSize(controlsSize)
        moveButtonRightButton.setIcon(INSTANCE.icon('arrow-right'))
        moveButtonRightButton.setToolTip(i18n("Move selected workflow button to the right"))
        moveButtonRightButton.clicked.connect(self.moveButtonRight)
        layoutForSelectorControls.addWidget(moveButtonRightButton)

        # button to delete selected button
        deleteButtonButton = QToolButton(self)
        deleteButtonButton.setIconSize(controlsSize)
        deleteButtonButton.setIcon(INSTANCE.icon('deletelayer'))
        deleteButtonButton.setToolTip(i18n("Delete selected workflow button"))
        deleteButtonButton.clicked.connect(self.deleteButton)
        layoutForSelectorControls.addWidget(deleteButtonButton)
        mainLayout.addLayout(layoutForSelectorControls)

        # set selected button's icon mode
        layoutForIconMode = QHBoxLayout()
        iconModeLabel = QLabel(i18n("Select icon mode:"), self)
        layoutForIconMode.addWidget(iconModeLabel)
        self.iconModeSelector = QComboBox(self)
        self.populateIconModeList()
        self.iconModeSelector.activated.connect(self.iconModeChanged)
        layoutForIconMode.addWidget(self.iconModeSelector)

        spacer2 = QWidget(self)
        spacer2.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        layoutForIconMode.addWidget(spacer2)

        mainLayout.addLayout(layoutForIconMode)

        # set selected button's custom icon
        layoutForIconSelection = QHBoxLayout()

        addIconLabel = QLabel(i18n("Select a custom icon:"), self)
        layoutForIconSelection.addWidget(addIconLabel)
        self.iconPathInput = QLineEdit(self)
        self.iconPathInput.setToolTip(i18n("Custom icon path"))
        self.iconPathInput.editingFinished.connect(self.iconPathChanged)
        layoutForIconSelection.addWidget(self.iconPathInput)

        iconPathDialogButton = QPushButton(i18n("..."), self)
        iconPathDialogButton.setToolTip(i18n("Select the custom icon"))
        iconPathDialogButton.clicked.connect(self.selectIcon)
        layoutForIconSelection.addWidget(iconPathDialogButton)

        mainLayout.addLayout(layoutForIconSelection)

        # set custom button's tooltip
        layoutForToolTip = QHBoxLayout()
        toolTipLabel = QLabel(i18n("Enter a Tooltip:"), self)
        layoutForToolTip.addWidget(toolTipLabel)
        self.toolTipInput = QLineEdit(self)
        self.toolTipInput.setToolTip(i18n("Tooltip text"))
        self.toolTipInput.editingFinished.connect(self.toolTipChanged)
        layoutForToolTip.addWidget(self.toolTipInput)

        mainLayout.addLayout(layoutForToolTip)

        # set custom button's' toolName
        layoutForToolSelector = QHBoxLayout()
        toolSelectorLabel = QLabel(i18n("Select a tool:"), self)
        layoutForToolSelector.addWidget(toolSelectorLabel)
        self.toolSelector = QComboBox(self)
        self.populateToolList()
        self.toolSelector.activated.connect(self.toolChanged)
        layoutForToolSelector.addWidget(self.toolSelector)

        spacer3 = QWidget(self)
        spacer3.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        layoutForToolSelector.addWidget(spacer3)

        mainLayout.addLayout(layoutForToolSelector)

        # set custom button's presetName
        layoutForPresetSelector = QHBoxLayout()
        presetSelectorLabel = QLabel(i18n("Select a brush preset:"), self)
        layoutForPresetSelector.addWidget(presetSelectorLabel)
        self.presetSelectorInput = QLineEdit(self)
        self.presetSelectorInput.setToolTip(i18n("Brush preset name"))
        self.presetSelectorInput.editingFinished.connect(self.presetChanged)
        layoutForPresetSelector.addWidget(self.presetSelectorInput)

        presetDialogButton = QPushButton(i18n("..."), self)
        presetDialogButton.setToolTip(i18n("Select the brush preset"))
        presetDialogButton.clicked.connect(self.selectPreset)
        layoutForPresetSelector.addWidget(presetDialogButton)

        mainLayout.addLayout(layoutForPresetSelector)

        # set custom button's FGColorValues
        layoutForFGColorInput = QHBoxLayout()
        FGColorLabel = QLabel(i18n("Select foreground color:"), self)
        layoutForFGColorInput.addWidget(FGColorLabel)
        FGColorInputSelector = QPushButton(i18n("Load current foreground color"), self)
        FGColorInputSelector.clicked.connect(self.selectFGColor)
        layoutForFGColorInput.addWidget(FGColorInputSelector)
        FGColorClear = QPushButton(i18n("Clear foreground color"), self)
        FGColorClear.clicked.connect(self.clearFGColor)
        layoutForFGColorInput.addWidget(FGColorClear)

        spacer4 = QWidget(self)
        spacer4.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        layoutForFGColorInput.addWidget(spacer4)

        mainLayout.addLayout(layoutForFGColorInput)

        colorInfoToolTip = i18n("Color model ; depth ; profile ; components")

        layoutForFGColorInfo = QHBoxLayout()
        self.FGColorPreview = SelectedColorPreview(self, self.defaultButtonContent["FGColorValues"])
        layoutForFGColorInfo.addWidget(self.FGColorPreview)
        self.FGColorInfoLabel = QLabel(self)
        self.FGColorInfoLabel.setToolTip(colorInfoToolTip)
        layoutForFGColorInfo.addWidget(self.FGColorInfoLabel)

        spacer5 = QWidget(self)
        spacer5.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        layoutForFGColorInfo.addWidget(spacer5)

        mainLayout.addLayout(layoutForFGColorInfo)

        # set custom button's BGColorValues
        layoutForBGColorInput = QHBoxLayout()
        BGColorLabel = QLabel(i18n("Select background color:"), self)
        layoutForBGColorInput.addWidget(BGColorLabel)
        BGColorInputSelector = QPushButton(i18n("Load current background color"), self)
        BGColorInputSelector.clicked.connect(self.selectBGColor)
        layoutForBGColorInput.addWidget(BGColorInputSelector)
        BGColorClear = QPushButton(i18n("Clear background color"), self)
        BGColorClear.clicked.connect(self.clearBGColor)
        layoutForBGColorInput.addWidget(BGColorClear)

        spacer6 = QWidget(self)
        spacer6.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        layoutForBGColorInput.addWidget(spacer6)

        mainLayout.addLayout(layoutForBGColorInput)

        layoutForBGColorInfo = QHBoxLayout()
        self.BGColorPreview = SelectedColorPreview(self, self.defaultButtonContent["BGColorValues"])
        layoutForBGColorInfo.addWidget(self.BGColorPreview)
        self.BGColorInfoLabel = QLabel(self)
        self.BGColorInfoLabel.setToolTip(colorInfoToolTip)
        layoutForBGColorInfo.addWidget(self.BGColorInfoLabel)

        spacer7 = QWidget(self)
        spacer7.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        layoutForBGColorInfo.addWidget(spacer7)

        mainLayout.addLayout(layoutForBGColorInfo)

        # set custom button's extra script
        layoutForScriptSelection = QHBoxLayout()

        scriptLabel = QLabel(i18n("Select a script file:"), self)
        layoutForScriptSelection.addWidget(scriptLabel)
        self.scriptPathInput = QLineEdit(self)
        self.scriptPathInput.setToolTip(i18n("Script path"))
        self.scriptPathInput.editingFinished.connect(self.scriptPathChanged)
        layoutForScriptSelection.addWidget(self.scriptPathInput)

        scriptPathDialogButton = QPushButton(i18n("..."), self)
        scriptPathDialogButton.setToolTip(i18n("Select the script"))
        scriptPathDialogButton.clicked.connect(self.selectScript)
        layoutForScriptSelection.addWidget(scriptPathDialogButton)

        mainLayout.addLayout(layoutForScriptSelection)

        # spacer for global controls
        spacerLine = QFrame(self)
        spacerLine.setFrameShape(QFrame.HLine)
        spacerLine.setFrameShadow(QFrame.Sunken)
        mainLayout.addWidget(spacerLine)

        # button's size selector
        layoutForButtonsSize = QHBoxLayout()
        buttonsSizeLabel = QLabel(i18n("Buttons size:"), self)
        self.buttonsSizeSelector = QComboBox(self)
        self.populateSizeList()
        self.buttonsSizeSelector.setCurrentIndex(self.sizeIndex)
        self.buttonsSizeSelector.activated.connect(self.buttonsSizeChanged)
        layoutForButtonsSize.addWidget(buttonsSizeLabel)
        layoutForButtonsSize.addWidget(self.buttonsSizeSelector)

        spacer8 = QWidget(self)
        spacer8.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        layoutForButtonsSize.addWidget(spacer8)

        mainLayout.addLayout(layoutForButtonsSize)

        # layout for settings button position option
        layoutForSettingsButtonOption = QHBoxLayout()
        settingsButtonOptionLabel = QLabel(i18n("Settings button position:"), self)
        self.settingsButtonPositionSelector = QComboBox(self)
        self.settingsButtonPositionSelector.insertItem(0, i18n("Bottom bar"))
        self.settingsButtonPositionSelector.insertItem(1, i18n("Inline"))
        self.settingsButtonPositionSelector.setCurrentIndex(self.settingsButtonPosition)
        self.settingsButtonPositionSelector.activated.connect(self.settingsButtonPositionChanged)
        layoutForSettingsButtonOption.addWidget(settingsButtonOptionLabel)
        layoutForSettingsButtonOption.addWidget(self.settingsButtonPositionSelector)

        spacer9 = QWidget(self)
        spacer9.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        layoutForSettingsButtonOption.addWidget(spacer9)

        mainLayout.addLayout(layoutForSettingsButtonOption)

        # main dialog's default buttons
        layoutForBottom = QHBoxLayout()
        buttonBox = QDialogButtonBox(self)
        buttonBox.setStandardButtons( QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttonBox.accepted.connect(self.accept)
        buttonBox.rejected.connect(self.reject)
        layoutForBottom.addWidget(buttonBox)
        mainLayout.addLayout(layoutForBottom)

        self.populateButtons()
        self.resize(500, 500)
        # end of __init__

    def populateIconModeList(self):
        listIndex = -1
        for mode in LISTOFICONMODES:
            listIndex += 1
            self.iconModeSelector.insertItem(listIndex,str(LISTOFICONMODES[listIndex]) + " ")
            # extra " " to workaround possible cut of last letter...

    def populateToolList(self):
        toolNumber = 0
        # Workaround to avoid passing an empty string to i18n(), which triggers a warning.
        self.toolSelector.insertItem(toolNumber,
                                     INSTANCE.icon(LISTOFTOOLS[toolNumber]["toolIcon"]),
                                     LISTOFTOOLS[toolNumber]["toolString"])
        for tool in LISTOFTOOLS[1:]:
            toolNumber += 1
            self.toolSelector.insertItem(toolNumber,
                                         INSTANCE.icon(LISTOFTOOLS[toolNumber]["toolIcon"]),
                                         i18n(LISTOFTOOLS[toolNumber]["toolString"]))

    def populateSizeList(self):
        listIndex = -1
        for size in LISTOFSIZES:
            listIndex += 1
            self.buttonsSizeSelector.insertItem(listIndex,str(LISTOFSIZES[listIndex]))

    def createButton(self, buttonToCreate, buttonID):
        # print("create button with buttonID: " + str(buttonID))
        if buttonToCreate["iconMode"] == 0:
            buttonIcon = QIcon(buttonToCreate["icon"])
        elif buttonToCreate["iconMode"] == 1 and buttonToCreate["toolIndex"] != 0:
            buttonIcon = INSTANCE.icon(LISTOFTOOLS[buttonToCreate["toolIndex"]]["toolIcon"])
        elif buttonToCreate["iconMode"] == 2 and buttonToCreate["presetName"] != "":
            brushPreset = self.allBrushPresets[buttonToCreate["presetName"]]
            buttonIcon = QIcon(QPixmap.fromImage(brushPreset.image()))
        else:
            buttonIcon = QIcon()
        button = CustomButtonForSettings(self.buttonsWidget, buttonID)
        button.setIconSize(self.globalButtonSize)
        button.setIcon(buttonIcon)
        button.clicked.connect(lambda state, x=button.buttonID: self.selectButton(x))
        self.buttonsLayout.addWidget(button)

    def clearButtons(self):
        # print("clearButtons called")
        for widget in self.buttonsWidget.children():
            if isinstance(widget, CustomButtonForSettings):
                widget.deleteLater()

    def populateButtons(self):
        # print("populateButtons called")
        buttonID = 0
        for buttonToCreate in self.buttonsContentList:
            buttonID += 1
            # print("create button id " + str(buttonID))
            self.createButton(buttonToCreate, buttonID)
        self.selectButton(self.selectedButtonID)

    def refreshButtons(self):
        # print("refreshButtons called")
        self.clearButtons()
        self.populateButtons()

    def updateDialogFields(self):
        # print("updateDialogFields called")
        self.selectedButtonIDLabel.setText(str(self.selectedButtonID))
        # add a line here for new dialog fields...
        if self.selectedButtonID > 0:
            self.iconModeSelector.setCurrentIndex(self.buttonsContentList[self.selectedButtonID - 1]["iconMode"])
            self.iconPathInput.setText(self.buttonsContentList[self.selectedButtonID - 1]["icon"])
            self.toolTipInput.setText(self.buttonsContentList[self.selectedButtonID - 1]["tooltip"])
            self.toolSelector.setCurrentIndex(self.buttonsContentList[self.selectedButtonID - 1]["toolIndex"])
            self.presetSelectorInput.setText(self.buttonsContentList[self.selectedButtonID - 1]["presetName"])
            self.setFGColorInfoLabel(self.buttonsContentList[self.selectedButtonID - 1]["FGColorValues"])
            self.setBGColorInfoLabel(self.buttonsContentList[self.selectedButtonID - 1]["BGColorValues"])
            self.scriptPathInput.setText(self.buttonsContentList[self.selectedButtonID - 1]["script"])

    def selectButton(self, buttonID):
        # print("selectButton " + str(buttonID))
        self.selectedButtonID = buttonID
        for widget in self.buttonsWidget.children():
            if isinstance(widget, CustomButtonForSettings):
                if widget.buttonID == self.selectedButtonID:
                    widget.selectButton()
                else:
                    widget.deselectButton()
        self.updateDialogFields()

    def addButton(self):
        # print("addButton pressed")
        buttonID = len(self.buttonsContentList) + 1
        self.selectedButtonID = buttonID
        buttonContent = copy.deepcopy(self.defaultButtonContent)
        self.buttonsContentList.append(buttonContent)
        self.createButton(self.defaultButtonContent, buttonID)
        self.refreshButtons()

    def iconModeChanged(self, iconModeIndex):
        if self.selectedButtonID < 1:
            return
        self.buttonsContentList[self.selectedButtonID - 1]["iconMode"] = iconModeIndex
        self.refreshButtons()

    def selectIcon(self):
        # print("select icon dialog started")
        if self.selectedButtonID < 1:
            return
        dialog = QFileDialog(self)
        formatList = []
        for formatBytes in QImageReader.supportedImageFormats():
            formatList.append(f"*.{str(formatBytes, 'utf-8')}")
        formatsString = " ".join(formatList)
        # The format string cannot be translated, only the description
        dialog.setNameFilter(i18n("Icon files ") + "(" + formatsString + ")")

        if dialog.exec_():
            selectedFile = dialog.selectedFiles()[0]
            self.iconPathInput.setText(selectedFile)
            self.iconPathChanged()

    def iconPathChanged(self):
        # print("icon path edited...")
        if self.selectedButtonID < 1:
            return
        self.buttonsContentList[self.selectedButtonID - 1]["icon"] = self.iconPathInput.text()
        self.refreshButtons()

    def toolTipChanged(self):
        # print("tooltip edited...")
        if self.selectedButtonID < 1:
            return
        self.buttonsContentList[self.selectedButtonID - 1]["tooltip"] = self.toolTipInput.text()

    def toolChanged(self, toolIndex):
        # print("tool changed")
        if self.selectedButtonID < 1:
            return
        self.buttonsContentList[self.selectedButtonID - 1]["toolIndex"] = toolIndex
        if self.iconModeSelector.currentIndex() == 1:
            self.refreshButtons()

    def selectPreset(self):
        # print("select preset dialog started")
        if self.selectedButtonID < 1:
            return
        dialog = QDialog(self)
        dialogLayout = QVBoxLayout(dialog)
        presetChooser = PresetChooser(dialog)
        dialogLayout.addWidget(presetChooser)
        buttonBox = QDialogButtonBox(dialog)
        buttonBox.setStandardButtons( QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttonBox.accepted.connect(dialog.accept)
        buttonBox.rejected.connect(dialog.reject)
        dialogLayout.addWidget(buttonBox)

        dialog.exec_()
        if dialog.result() == 1 and presetChooser.currentPreset():
            self.presetSelectorInput.setText(presetChooser.currentPreset().name())
            self.presetChanged()

    def presetChanged(self):
        # print("icon path edited...")
        if self.selectedButtonID < 1:
            return
        self.buttonsContentList[self.selectedButtonID - 1]["presetName"] = self.presetSelectorInput.text()
        if self.iconModeSelector.currentIndex() == 2:
            self.refreshButtons()

    def setFGColorInfoLabel(self, values):
        colorInfoLabel = i18n("Color info: ")
        colorModel = values["model"]
        if colorModel == "":
            self.FGColorInfoLabel.setText(colorInfoLabel)
        else:
            colorDepth = values["depth"]
            colorProfile = values["profile"]
            colorComponents = values["components"]
            shortenedComponents = [ "%.3f" % component for component in colorComponents ]
            separator = " ; "
            self.FGColorInfoLabel.setText(colorInfoLabel + colorModel + separator + colorDepth + separator + colorProfile + separator + str(shortenedComponents))
        self.FGColorPreview.setColor(values)

    def selectFGColor(self):
        if self.selectedButtonID < 1:
            return
        # print("FGColor edited...")
        savedColor = { "model":"", "depth":"", "components":[] }
        currentFGColor = INSTANCE.activeWindow().activeView().foregroundColor()
        savedColor["model"] = currentFGColor.colorModel()
        savedColor["depth"] = currentFGColor.colorDepth()
        savedColor["components"] = currentFGColor.components()
        savedColor["profile"] = currentFGColor.colorProfile()
        self.buttonsContentList[self.selectedButtonID - 1]["FGColorValues"] = savedColor
        self.setFGColorInfoLabel(savedColor)

    def clearFGColor(self):
        if self.selectedButtonID < 1:
            return
        self.buttonsContentList[self.selectedButtonID - 1]["FGColorValues"] = self.defaultButtonContent["FGColorValues"]
        self.setFGColorInfoLabel(self.buttonsContentList[self.selectedButtonID - 1]["FGColorValues"])

    def setBGColorInfoLabel(self, values):
        colorInfoLabel = i18n("Color info: ")
        colorModel = values["model"]
        if colorModel == "":
            self.BGColorInfoLabel.setText(colorInfoLabel)
        else:
            colorDepth = values["depth"]
            colorProfile = values["profile"]
            colorComponents = values["components"]
            shortenedComponents = [ "%.3f" % component for component in colorComponents ]
            separator = " ; "
            self.BGColorInfoLabel.setText(colorInfoLabel + colorModel + separator + colorDepth + separator + colorProfile + separator + str(shortenedComponents))
        self.BGColorPreview.setColor(values)

    def selectBGColor(self):
        if self.selectedButtonID < 1:
            return
        # print("BGColor edited...")
        savedColor = { "model":"", "depth":"", "components":[] }
        currentBGColor = INSTANCE.activeWindow().activeView().backgroundColor()
        savedColor["model"] = currentBGColor.colorModel()
        savedColor["depth"] = currentBGColor.colorDepth()
        savedColor["components"] = currentBGColor.components()
        savedColor["profile"] = currentBGColor.colorProfile()
        self.buttonsContentList[self.selectedButtonID - 1]["BGColorValues"] = savedColor
        self.setBGColorInfoLabel(savedColor)

    def clearBGColor(self):
        if self.selectedButtonID < 1:
            return
        self.buttonsContentList[self.selectedButtonID - 1]["BGColorValues"] = self.defaultButtonContent["BGColorValues"]
        self.setBGColorInfoLabel(self.buttonsContentList[self.selectedButtonID - 1]["BGColorValues"])

    def selectScript(self):
        # print("select script dialog started")
        if self.selectedButtonID < 1:
            return
        dialog = QFileDialog(self)
        dialog.setNameFilter(i18n("Script files ") + "(*.py)")

        if dialog.exec_():
            selectedFile = dialog.selectedFiles()[0]
            self.scriptPathInput.setText(selectedFile)
            self.scriptPathChanged()

    def scriptPathChanged(self):
        # print("icon path edited...")
        if self.selectedButtonID < 1:
            return
        self.buttonsContentList[self.selectedButtonID - 1]["script"] = self.scriptPathInput.text()

    def clearDialogFields(self):
        # print("clear all dialog fields...")
        # add a line here for new dialog fields...
        self.iconModeSelector.setCurrentIndex(0)
        self.selectedButtonIDLabel.setText("0")
        self.iconPathInput.setText("")
        self.toolTipInput.setText("")
        self.toolSelector.setCurrentIndex(0)
        self.presetSelectorInput.setText("")
        self.setFGColorInfoLabel(self.defaultButtonContent["FGColorValues"])
        self.setBGColorInfoLabel(self.defaultButtonContent["BGColorValues"])
        self.scriptPathInput.setText("")

    def deleteButton(self):
        # print("delete button " + str(self.selectedButtonID))
        if self.selectedButtonID < 1 :
            return
        del self.buttonsContentList[self.selectedButtonID - 1]
        self.selectedButtonID -= 1
        if len(self.buttonsContentList) < 1:
            self.selectedButtonID = 0
            self.refreshButtons()
            self.clearDialogFields()
            return
        if self.selectedButtonID == 0:
            self.selectedButtonID = 1
        self.refreshButtons()

    def moveButtonLeft(self):
        # print("moveLeft button " + str(self.selectedButtonID))
        if self.selectedButtonID <= 1:
            return
        self.buttonsContentList.insert(self.selectedButtonID - 2, self.buttonsContentList.pop(self.selectedButtonID - 1))
        self.selectedButtonID -= 1
        self.refreshButtons()

    def moveButtonRight(self):
        # print("moveRight button " + str(self.selectedButtonID))
        if self.selectedButtonID >= len(self.buttonsContentList):
            return
        self.buttonsContentList.insert(self.selectedButtonID, self.buttonsContentList.pop(self.selectedButtonID - 1))
        self.selectedButtonID += 1
        self.refreshButtons()

    def buttonsSizeChanged(self, sizeIndex):
        # print("size changed")
        self.sizeIndex = sizeIndex
        self.globalButtonSize = QSize(LISTOFSIZES[sizeIndex], LISTOFSIZES[sizeIndex])
        if self.selectedButtonID > 0:
            self.refreshButtons()

    def settingsButtonPositionChanged(self, position):
        # print("settings button position changed")
        self.settingsButtonPosition = position


class CustomButtonForSettings(QToolButton):
    # Class to define the custom buttons inside the dialog
    def __init__(self, parent=None, buttonID=-1):
        super().__init__(parent)
        self.buttonID = buttonID
        self.highlightSquare = selectedButtonHighlight(self, self.height())

    def resizeEvent(self, event):
        self.highlightSquare.setMinimumSize(QSize(self.height(), self.height()))

    def selectButton(self):
        self.highlightSquare.setVisible(True)

    def deselectButton(self):
        self.highlightSquare.setVisible(False)


class selectedButtonHighlight(QWidget):
    # Class to define the highlight for selected button
    def __init__(self, parent=None, size=32):
        super().__init__(parent)
        self.colorPen = self.palette().highlight().color()
        self.colorBrush = self.palette().highlight().color()
        self.colorBrush.setAlpha(64)
        self.setMinimumSize(QSize(size,size))

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setPen(QPen(self.colorPen, 4, 1)) # last 1 is for Qt.SolidLine
        painter.setBrush(QBrush(self.colorBrush, 1)) # last 1 is for Qt.SolidPattern
        painter.drawRect(2, 2, self.minimumSize().width() - 3, self.minimumSize().height() - 4)


class SelectedColorPreview(QWidget):
    # Class to define the widget used to visualize selected FG and BG colors
    def __init__(self, parent=None, colorInfo={}):
        super().__init__(parent)
        self.setMinimumSize(32, 32)
        self.canvas = INSTANCE.activeWindow().activeView().canvas()
        self.colorInfo = {}
        self.color = QColor(0,0,0,0)
        self.emptyColor = QColor(0,0,0,0)
        self.outlineColor = self.palette().text().color()
        self.setColor(colorInfo)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setPen(QPen(self.outlineColor, 1, 1)) # last 1 is for Qt.SolidLine
        painter.setBrush(QBrush(self.color, 1)) # last 1 is for Qt.SolidPattern
        painter.drawRect(1, 1, 30, 30)

    def convertColorInfoToQColor(self):
        if(self.colorInfo["model"] != ""):
            managedColor = ManagedColor(self.colorInfo["model"],
                                        self.colorInfo["depth"],
                                        self.colorInfo["profile"])
            colorComponents = self.colorInfo["components"]
            managedColor.setComponents(colorComponents)
            self.color = managedColor.colorForCanvas(self.canvas)
        else:
            self.color = self.emptyColor

    def setColor(self, colorInfo):
        self.colorInfo = colorInfo
        self.convertColorInfoToQColor()
        self.update()
