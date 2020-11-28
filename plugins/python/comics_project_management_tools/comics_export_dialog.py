"""
SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

This file is part of the Comics Project Management Tools(CPMT).

SPDX-License-Identifier: GPL-3.0-or-later
"""

"""
A dialog for editing the exporter settings.
"""

from enum import IntEnum
from PyQt5.QtGui import QStandardItem, QStandardItemModel, QColor, QFont, QIcon, QPixmap
from PyQt5.QtWidgets import QDialog, QDialogButtonBox, QGroupBox, QFormLayout, QCheckBox, QComboBox, QSpinBox, QWidget, QVBoxLayout, QHBoxLayout, QTabWidget, QPushButton, QLineEdit, QLabel, QListView, QTableView, QFontComboBox, QSpacerItem, QColorDialog, QStyledItemDelegate
from PyQt5.QtCore import Qt, QUuid
from krita import *

"""
A generic widget to make selecting size easier.
It works by initialising with a config name(like "scale"), and then optionally setting the config with a dictionary.
Then, afterwards, you get the config with a dictionary, with the config name being the entry the values are under.
"""


class comic_export_resize_widget(QGroupBox):
    configName = ""

    def __init__(self, configName, batch=False, fileType=True):
        super().__init__()
        self.configName = configName
        self.setTitle(i18n("Adjust Working File"))
        formLayout = QFormLayout()
        self.setLayout(formLayout)
        self.crop = QCheckBox(i18n("Crop files before resize"))
        self.cmbFile = QComboBox()
        self.cmbFile.addItems(["png", "jpg", "webp"])
        self.resizeMethod = QComboBox()
        self.resizeMethod.addItems([i18n("Percentage"), i18n("DPI"), i18n("Maximum Width"), i18n("Maximum Height")])
        self.resizeMethod.currentIndexChanged.connect(self.slot_set_enabled)
        self.spn_DPI = QSpinBox()
        self.spn_DPI.setMaximum(1200)
        self.spn_DPI.setSuffix(i18n(" DPI"))
        self.spn_DPI.setValue(72)
        self.spn_PER = QSpinBox()
        if batch is True:
            self.spn_PER.setMaximum(1000)
        else:
            self.spn_PER.setMaximum(100)
        self.spn_PER.setSuffix(" %")
        self.spn_PER.setValue(100)
        self.spn_width = QSpinBox()
        self.spn_width.setMaximum(99999)
        self.spn_width.setSuffix(" px")
        self.spn_width.setValue(800)
        self.spn_height = QSpinBox()
        self.spn_height.setMaximum(99999)
        self.spn_height.setSuffix(" px")
        self.spn_height.setValue(800)

        if batch is False:
            formLayout.addRow("", self.crop)
        if fileType is True and configName != "TIFF":
            formLayout.addRow(i18n("File Type"), self.cmbFile)
        formLayout.addRow(i18n("Method:"), self.resizeMethod)
        formLayout.addRow(i18n("DPI:"), self.spn_DPI)
        formLayout.addRow(i18n("Percentage:"), self.spn_PER)
        formLayout.addRow(i18n("Width:"), self.spn_width)
        formLayout.addRow(i18n("Height:"), self.spn_height)
        self.slot_set_enabled()

    def slot_set_enabled(self):
        method = self.resizeMethod.currentIndex()
        self.spn_DPI.setEnabled(False)
        self.spn_PER.setEnabled(False)
        self.spn_width.setEnabled(False)
        self.spn_height.setEnabled(False)

        if method == 0:
            self.spn_PER.setEnabled(True)
        if method == 1:
            self.spn_DPI.setEnabled(True)
        if method == 2:
            self.spn_width.setEnabled(True)
        if method == 3:
            self.spn_height.setEnabled(True)

    def set_config(self, config):
        if self.configName in config.keys():
            mConfig = config[self.configName]
            if "Method" in mConfig.keys():
                self.resizeMethod.setCurrentIndex(mConfig["Method"])
            if "FileType" in mConfig.keys():
                self.cmbFile.setCurrentText(mConfig["FileType"])
            if "Crop" in mConfig.keys():
                self.crop.setChecked(mConfig["Crop"])
            if "DPI" in mConfig.keys():
                self.spn_DPI.setValue(mConfig["DPI"])
            if "Percentage" in mConfig.keys():
                self.spn_PER.setValue(mConfig["Percentage"])
            if "Width" in mConfig.keys():
                self.spn_width.setValue(mConfig["Width"])
            if "Height" in mConfig.keys():
                self.spn_height.setValue(mConfig["Height"])
            self.slot_set_enabled()

    def get_config(self, config):
        mConfig = {}
        mConfig["Method"] = self.resizeMethod.currentIndex()
        if self.configName == "TIFF":
            mConfig["FileType"] = "tiff"
        else:
            mConfig["FileType"] = self.cmbFile.currentText()
        mConfig["Crop"] = self.crop.isChecked()
        mConfig["DPI"] = self.spn_DPI.value()
        mConfig["Percentage"] = self.spn_PER.value()
        mConfig["Width"] = self.spn_width.value()
        mConfig["Height"] = self.spn_height.value()
        config[self.configName] = mConfig
        return config


"""
Quick combobox for selecting the color label.
"""


class labelSelector(QComboBox):
    def __init__(self):
        super(labelSelector, self).__init__()
        lisOfColors = []
        lisOfColors.append(Qt.transparent)
        lisOfColors.append(QColor(91, 173, 220))
        lisOfColors.append(QColor(151, 202, 63))
        lisOfColors.append(QColor(247, 229, 61))
        lisOfColors.append(QColor(255, 170, 63))
        lisOfColors.append(QColor(177, 102, 63))
        lisOfColors.append(QColor(238, 50, 51))
        lisOfColors.append(QColor(191, 106, 209))
        lisOfColors.append(QColor(118, 119, 114))

        self.itemModel = QStandardItemModel()
        for color in lisOfColors:
            item = QStandardItem()
            item.setFlags(Qt.ItemIsUserCheckable | Qt.ItemIsEnabled)
            item.setCheckState(Qt.Unchecked)
            item.setText(" ")
            item.setData(color, Qt.BackgroundColorRole)
            self.itemModel.appendRow(item)
        self.setModel(self.itemModel)

    def getLabels(self):
        listOfIndexes = []
        for i in range(self.itemModel.rowCount()):
            index = self.itemModel.index(i, 0)
            item = self.itemModel.itemFromIndex(index)
            if item.checkState():
                listOfIndexes.append(i)
        return listOfIndexes

    def setLabels(self, listOfIndexes):
        for i in listOfIndexes:
            index = self.itemModel.index(i, 0)
            item = self.itemModel.itemFromIndex(index)
            item.setCheckState(True)

"""
Little Enum to keep track of where in the item we add styles.
"""
class styleEnum(IntEnum):
    FONT = Qt.UserRole + 1
    FONTLIST = Qt.UserRole + 2
    FONTGENERIC = Qt.UserRole + 3
    BOLD = Qt.UserRole + 4
    ITALIC = Qt.UserRole + 5
"""
A simple delegate to allows editing fonts with a QFontComboBox
"""
class font_list_delegate(QStyledItemDelegate):
    def __init__(self, parent=None):
        super(QStyledItemDelegate, self).__init__(parent)
    def createEditor(self, parent, option, index):
        editor = QFontComboBox(parent)
        return editor

"""
The comic export settings dialog will allow configuring the export.

This config consists of...

* Crop settings. for removing bleeds.
* Selecting layer labels to remove.
* Choosing which formats to export to.
    * Choosing how to resize these
    * Whether to crop.
    * Which file type to use.

And for ACBF, it gives the ability to edit acbf document info.

"""


class comic_export_setting_dialog(QDialog):
    acbfStylesList = ["speech", "commentary", "formal", "letter", "code", "heading", "audio", "thought", "sign", "sound", "emphasis", "strong"]

    def __init__(self):
        super().__init__()
        self.setLayout(QVBoxLayout())
        self.setWindowTitle(i18n("Export Settings"))
        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)

        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        mainWidget = QTabWidget()
        self.layout().addWidget(mainWidget)
        self.layout().addWidget(buttons)

        # Set basic crop settings
        # Set which layers to remove before export.
        mainExportSettings = QWidget()
        mainExportSettings.setLayout(QVBoxLayout())
        groupExportCrop = QGroupBox(i18n("Crop Settings"))
        formCrop = QFormLayout()
        groupExportCrop.setLayout(formCrop)
        self.chk_toOutmostGuides = QCheckBox(i18n("Crop to outmost guides"))
        self.chk_toOutmostGuides.setChecked(True)
        self.chk_toOutmostGuides.setToolTip(i18n("This will crop to the outmost guides if possible and otherwise use the underlying crop settings."))
        formCrop.addRow("", self.chk_toOutmostGuides)
        btn_fromSelection = QPushButton(i18n("Set Margins from Active Selection"))
        btn_fromSelection.clicked.connect(self.slot_set_margin_from_selection)
        # This doesn't work.
        formCrop.addRow("", btn_fromSelection)
        self.spn_marginLeft = QSpinBox()
        self.spn_marginLeft.setMaximum(99999)
        self.spn_marginLeft.setSuffix(" px")
        formCrop.addRow(i18n("Left:"), self.spn_marginLeft)
        self.spn_marginTop = QSpinBox()
        self.spn_marginTop.setMaximum(99999)
        self.spn_marginTop.setSuffix(" px")
        formCrop.addRow(i18n("Top:"), self.spn_marginTop)
        self.spn_marginRight = QSpinBox()
        self.spn_marginRight.setMaximum(99999)
        self.spn_marginRight.setSuffix(" px")
        formCrop.addRow(i18n("Right:"), self.spn_marginRight)
        self.spn_marginBottom = QSpinBox()
        self.spn_marginBottom.setMaximum(99999)
        self.spn_marginBottom.setSuffix(" px")
        formCrop.addRow(i18n("Bottom:"), self.spn_marginBottom)
        groupExportLayers = QGroupBox(i18n("Layers"))
        formLayers = QFormLayout()
        groupExportLayers.setLayout(formLayers)
        self.cmbLabelsRemove = labelSelector()
        formLayers.addRow(i18n("Label for removal:"), self.cmbLabelsRemove)
        self.ln_text_layer_name = QLineEdit()
        self.ln_text_layer_name.setToolTip(i18n("These are keywords that can be used to identify text layers. A layer only needs to contain the keyword to be recognized. Keywords should be comma separated."))
        self.ln_panel_layer_name = QLineEdit()
        self.ln_panel_layer_name.setToolTip(i18n("These are keywords that can be used to identify panel layers. A layer only needs to contain the keyword to be recognized. Keywords should be comma separated."))
        formLayers.addRow(i18n("Text Layer Key:"), self.ln_text_layer_name)
        formLayers.addRow(i18n("Panel Layer Key:"), self.ln_panel_layer_name)

        mainExportSettings.layout().addWidget(groupExportCrop)
        mainExportSettings.layout().addWidget(groupExportLayers)
        mainWidget.addTab(mainExportSettings, i18n("General"))

        # CBZ, crop, resize, which metadata to add.
        CBZexportSettings = QWidget()
        CBZexportSettings.setLayout(QVBoxLayout())
        self.CBZactive = QCheckBox(i18n("Export to CBZ"))
        CBZexportSettings.layout().addWidget(self.CBZactive)
        self.CBZgroupResize = comic_export_resize_widget("CBZ")
        CBZexportSettings.layout().addWidget(self.CBZgroupResize)
        self.CBZactive.clicked.connect(self.CBZgroupResize.setEnabled)
        CBZgroupMeta = QGroupBox(i18n("Metadata to Add"))
        # CBZexportSettings.layout().addWidget(CBZgroupMeta)
        CBZgroupMeta.setLayout(QFormLayout())

        mainWidget.addTab(CBZexportSettings, i18n("CBZ"))

        # ACBF, crop, resize, creator name, version history, panel layer, text layers.
        ACBFExportSettings = QWidget()
        ACBFform = QFormLayout()
        ACBFExportSettings.setLayout(QVBoxLayout())
        ACBFdocInfo = QGroupBox()
        ACBFdocInfo.setTitle(i18n("ACBF Document Info"))
        ACBFdocInfo.setLayout(ACBFform)
        self.lnACBFID = QLabel()
        self.lnACBFID.setToolTip(i18n("By default this will be filled with a generated universal unique identifier. The ID by itself is merely so that comic book library management programs can figure out if this particular comic is already in their database and whether it has been rated. Of course, the UUID can be changed into something else by manually changing the JSON, but this is advanced usage."))
        self.spnACBFVersion = QSpinBox()
        self.ACBFhistoryModel = QStandardItemModel()
        acbfHistoryList = QListView()
        acbfHistoryList.setModel(self.ACBFhistoryModel)
        btn_add_history = QPushButton(i18n("Add History Entry"))
        btn_add_history.clicked.connect(self.slot_add_history_item)
        self.chkIncludeTranslatorComments = QCheckBox()
        self.chkIncludeTranslatorComments.setText(i18n("Include translator's comments"))
        self.chkIncludeTranslatorComments.setToolTip(i18n("A PO file can contain translator's comments. If this is checked, the translations comments will be added as references into the ACBF file."))
        self.lnTranslatorHeader = QLineEdit()

        ACBFform.addRow(i18n("ACBF UID:"), self.lnACBFID)
        ACBFform.addRow(i18n("Version:"), self.spnACBFVersion)
        ACBFform.addRow(i18n("Version history:"), acbfHistoryList)
        ACBFform.addRow("", btn_add_history)
        ACBFform.addRow("", self.chkIncludeTranslatorComments)
        ACBFform.addRow(i18n("Translator header:"), self.lnTranslatorHeader)

        ACBFAuthorInfo = QWidget()
        acbfAVbox = QVBoxLayout(ACBFAuthorInfo)
        infoLabel = QLabel(i18n("The people responsible for the generation of the CBZ/ACBF files."))
        infoLabel.setWordWrap(True)
        ACBFAuthorInfo.layout().addWidget(infoLabel)
        self.ACBFauthorModel = QStandardItemModel(0, 6)
        labels = [i18n("Nick Name"), i18n("Given Name"), i18n("Middle Name"), i18n("Family Name"), i18n("Email"), i18n("Homepage")]
        self.ACBFauthorModel.setHorizontalHeaderLabels(labels)
        self.ACBFauthorTable = QTableView()
        acbfAVbox.addWidget(self.ACBFauthorTable)
        self.ACBFauthorTable.setModel(self.ACBFauthorModel)
        self.ACBFauthorTable.verticalHeader().setDragEnabled(True)
        self.ACBFauthorTable.verticalHeader().setDropIndicatorShown(True)
        self.ACBFauthorTable.verticalHeader().setSectionsMovable(True)
        self.ACBFauthorTable.verticalHeader().sectionMoved.connect(self.slot_reset_author_row_visual)
        AuthorButtons = QHBoxLayout()
        btn_add_author = QPushButton(i18n("Add Author"))
        btn_add_author.clicked.connect(self.slot_add_author)
        AuthorButtons.addWidget(btn_add_author)
        btn_remove_author = QPushButton(i18n("Remove Author"))
        btn_remove_author.clicked.connect(self.slot_remove_author)
        AuthorButtons.addWidget(btn_remove_author)
        acbfAVbox.addLayout(AuthorButtons)
        
        ACBFStyle = QWidget()
        ACBFStyle.setLayout(QHBoxLayout())
        self.ACBFStylesModel = QStandardItemModel()
        self.ACBFStyleClass = QListView()
        self.ACBFStyleClass.setModel(self.ACBFStylesModel)
        ACBFStyle.layout().addWidget(self.ACBFStyleClass)
        ACBFStyleEdit = QWidget()
        ACBFStyleEditVB = QVBoxLayout(ACBFStyleEdit)
        self.ACBFuseFont = QCheckBox(i18n("Use font"))
        self.ACBFFontList = QListView()
        self.ACBFFontList.setItemDelegate(font_list_delegate())
        self.ACBFuseFont.toggled.connect(self.font_slot_enable_font_view)
        self.ACBFFontListModel = QStandardItemModel()
        self.ACBFFontListModel.rowsRemoved.connect(self.slot_font_current_style)
        self.ACBFFontListModel.itemChanged.connect(self.slot_font_current_style)
        self.btnAcbfAddFont = QPushButton()
        self.btnAcbfAddFont.setIcon(Application.icon("list-add"))
        self.btnAcbfAddFont.clicked.connect(self.font_slot_add_font)
        self.btn_acbf_remove_font = QPushButton()
        self.btn_acbf_remove_font.setIcon(Application.icon("edit-delete"))
        self.btn_acbf_remove_font.clicked.connect(self.font_slot_remove_font)
        self.ACBFFontList.setModel(self.ACBFFontListModel)
        self.ACBFdefaultFont = QComboBox()
        self.ACBFdefaultFont.addItems(["sans-serif", "serif", "monospace", "cursive", "fantasy"])
        acbfFontButtons = QHBoxLayout()
        acbfFontButtons.addWidget(self.btnAcbfAddFont)
        acbfFontButtons.addWidget(self.btn_acbf_remove_font)
        self.ACBFBold = QCheckBox(i18n("Bold"))
        self.ACBFItal = QCheckBox(i18n("Italic"))
        self.ACBFStyleClass.clicked.connect(self.slot_set_style)
        self.ACBFStyleClass.selectionModel().selectionChanged.connect(self.slot_set_style)
        self.ACBFStylesModel.itemChanged.connect(self.slot_set_style)
        self.ACBFBold.toggled.connect(self.slot_font_current_style)
        self.ACBFItal.toggled.connect(self.slot_font_current_style)
        colorWidget = QGroupBox(self)
        colorWidget.setTitle(i18n("Text Colors"))
        colorWidget.setLayout(QVBoxLayout())
        self.regularColor = QColorDialog()
        self.invertedColor = QColorDialog()
        self.btn_acbfRegColor = QPushButton(i18n("Regular Text"), self)
        self.btn_acbfRegColor.clicked.connect(self.slot_change_regular_color)
        self.btn_acbfInvColor = QPushButton(i18n("Inverted Text"), self)
        self.btn_acbfInvColor.clicked.connect(self.slot_change_inverted_color)
        colorWidget.layout().addWidget(self.btn_acbfRegColor)
        colorWidget.layout().addWidget(self.btn_acbfInvColor)
        ACBFStyleEditVB.addWidget(colorWidget)
        ACBFStyleEditVB.addWidget(self.ACBFuseFont)
        ACBFStyleEditVB.addWidget(self.ACBFFontList)
        ACBFStyleEditVB.addLayout(acbfFontButtons)
        ACBFStyleEditVB.addWidget(self.ACBFdefaultFont)
        ACBFStyleEditVB.addWidget(self.ACBFBold)
        ACBFStyleEditVB.addWidget(self.ACBFItal)
        ACBFStyleEditVB.addStretch()
        ACBFStyle.layout().addWidget(ACBFStyleEdit)

        ACBFTabwidget = QTabWidget()
        ACBFTabwidget.addTab(ACBFdocInfo, i18n("Document Info"))
        ACBFTabwidget.addTab(ACBFAuthorInfo, i18n("Author Info"))
        ACBFTabwidget.addTab(ACBFStyle, i18n("Style Sheet"))
        ACBFExportSettings.layout().addWidget(ACBFTabwidget)
        mainWidget.addTab(ACBFExportSettings, i18n("ACBF"))

        # Epub export, crop, resize, other questions.
        EPUBexportSettings = QWidget()
        EPUBexportSettings.setLayout(QVBoxLayout())
        self.EPUBactive = QCheckBox(i18n("Export to EPUB"))
        EPUBexportSettings.layout().addWidget(self.EPUBactive)
        self.EPUBgroupResize = comic_export_resize_widget("EPUB")
        EPUBexportSettings.layout().addWidget(self.EPUBgroupResize)
        self.EPUBactive.clicked.connect(self.EPUBgroupResize.setEnabled)
        mainWidget.addTab(EPUBexportSettings, i18n("EPUB"))

        # For Print. Crop, no resize.
        TIFFExportSettings = QWidget()
        TIFFExportSettings.setLayout(QVBoxLayout())
        self.TIFFactive = QCheckBox(i18n("Export to TIFF"))
        TIFFExportSettings.layout().addWidget(self.TIFFactive)
        self.TIFFgroupResize = comic_export_resize_widget("TIFF")
        TIFFExportSettings.layout().addWidget(self.TIFFgroupResize)
        self.TIFFactive.clicked.connect(self.TIFFgroupResize.setEnabled)
        mainWidget.addTab(TIFFExportSettings, i18n("TIFF"))

        # SVG, crop, resize, embed vs link.
        #SVGExportSettings = QWidget()

        #mainWidget.addTab(SVGExportSettings, i18n("SVG"))

    """
    Add a history item to the acbf version history list.
    """

    def slot_add_history_item(self):
        newItem = QStandardItem()
        newItem.setText(str(i18n("v{version}-in this version...")).format(version=str(self.spnACBFVersion.value())))
        self.ACBFhistoryModel.appendRow(newItem)

    """
    Get the margins by treating the active selection in a document as the trim area.
    This allows people to snap selections to a vector or something, and then get the margins.
    """

    def slot_set_margin_from_selection(self):
        doc = Application.activeDocument()
        if doc is not None:
            if doc.selection() is not None:
                self.spn_marginLeft.setValue(doc.selection().x())
                self.spn_marginTop.setValue(doc.selection().y())
                self.spn_marginRight.setValue(doc.width() - (doc.selection().x() + doc.selection().width()))
                self.spn_marginBottom.setValue(doc.height() - (doc.selection().y() + doc.selection().height()))

    """
    Add an author with default values initialised.
    """

    def slot_add_author(self):
        listItems = []
        listItems.append(QStandardItem(i18n("Anon")))  # Nick name
        listItems.append(QStandardItem(i18n("John")))  # First name
        listItems.append(QStandardItem())  # Middle name
        listItems.append(QStandardItem(i18n("Doe")))  # Last name
        listItems.append(QStandardItem())  # email
        listItems.append(QStandardItem())  # homepage
        self.ACBFauthorModel.appendRow(listItems)

    """
    Remove the selected author from the author list.
    """

    def slot_remove_author(self):
        self.ACBFauthorModel.removeRow(self.ACBFauthorTable.currentIndex().row())

    """
    Ensure that the drag and drop of authors doesn't mess up the labels.
    """

    def slot_reset_author_row_visual(self):
        headerLabelList = []
        for i in range(self.ACBFauthorTable.verticalHeader().count()):
            headerLabelList.append(str(i))
        for i in range(self.ACBFauthorTable.verticalHeader().count()):
            logicalI = self.ACBFauthorTable.verticalHeader().logicalIndex(i)
            headerLabelList[logicalI] = str(i + 1)
        self.ACBFauthorModel.setVerticalHeaderLabels(headerLabelList)
    """
    Set the style item to the gui item's style.
    """
    
    def slot_set_style(self):
        index = self.ACBFStyleClass.currentIndex()
        if index.isValid():
            item = self.ACBFStylesModel.item(index.row())
            fontUsed = item.data(role=styleEnum.FONT)
            if fontUsed is not None:
                self.ACBFuseFont.setChecked(fontUsed)
            else:
                self.ACBFuseFont.setChecked(False)
            self.font_slot_enable_font_view()
            fontList = item.data(role=styleEnum.FONTLIST)
            self.ACBFFontListModel.clear()
            for font in fontList:
                NewItem = QStandardItem(font)
                NewItem.setEditable(True)
                self.ACBFFontListModel.appendRow(NewItem)
            self.ACBFdefaultFont.setCurrentText(str(item.data(role=styleEnum.FONTGENERIC)))
            bold = item.data(role=styleEnum.BOLD)
            if bold is not None:
                self.ACBFBold.setChecked(bold)
            else:
                self.ACBFBold.setChecked(False)
            italic = item.data(role=styleEnum.ITALIC)
            if italic is not None:
                self.ACBFItal.setChecked(italic)
            else:
                self.ACBFItal.setChecked(False)
    
    """
    Set the gui items to the currently selected style.
    """
    
    def slot_font_current_style(self):
        index = self.ACBFStyleClass.currentIndex()
        if index.isValid():
            item = self.ACBFStylesModel.item(index.row())
            fontList = []
            for row in range(self.ACBFFontListModel.rowCount()):
                font = self.ACBFFontListModel.item(row)
                fontList.append(font.text())
            item.setData(self.ACBFuseFont.isChecked(), role=styleEnum.FONT)
            item.setData(fontList, role=styleEnum.FONTLIST)
            item.setData(self.ACBFdefaultFont.currentText(), role=styleEnum.FONTGENERIC)
            item.setData(self.ACBFBold.isChecked(), role=styleEnum.BOLD)
            item.setData(self.ACBFItal.isChecked(), role=styleEnum.ITALIC)
            self.ACBFStylesModel.setItem(index.row(), item)
    """
    Change the regular color
    """
    
    def slot_change_regular_color(self):
        if (self.regularColor.exec_() == QDialog.Accepted):
            square = QPixmap(32, 32)
            square.fill(self.regularColor.currentColor())
            self.btn_acbfRegColor.setIcon(QIcon(square))
    """
    change the inverted color
    """
    
    def slot_change_inverted_color(self):
        if (self.invertedColor.exec_() == QDialog.Accepted):
            square = QPixmap(32, 32)
            square.fill(self.invertedColor.currentColor())
            self.btn_acbfInvColor.setIcon(QIcon(square))
            
    def font_slot_enable_font_view(self):
        self.ACBFFontList.setEnabled(self.ACBFuseFont.isChecked())
        self.btn_acbf_remove_font.setEnabled(self.ACBFuseFont.isChecked())
        self.btnAcbfAddFont.setEnabled(self.ACBFuseFont.isChecked())
        self.ACBFdefaultFont.setEnabled(self.ACBFuseFont.isChecked())
        if self.ACBFFontListModel.rowCount() < 2:
                self.btn_acbf_remove_font.setEnabled(False)
        
    def font_slot_add_font(self):
        NewItem = QStandardItem(QFont().family())
        NewItem.setEditable(True)
        self.ACBFFontListModel.appendRow(NewItem)
    
    def font_slot_remove_font(self):
        index  = self.ACBFFontList.currentIndex()
        if index.isValid():
            self.ACBFFontListModel.removeRow(index.row())
            if self.ACBFFontListModel.rowCount() < 2:
                self.btn_acbf_remove_font.setEnabled(False)

    """
    Load the UI values from the config dictionary given.
    """

    def setConfig(self, config):
        if "cropToGuides" in config.keys():
            self.chk_toOutmostGuides.setChecked(config["cropToGuides"])
        if "cropLeft" in config.keys():
            self.spn_marginLeft.setValue(config["cropLeft"])
        if "cropTop" in config.keys():
            self.spn_marginTop.setValue(config["cropTop"])
        if "cropRight" in config.keys():
            self.spn_marginRight.setValue(config["cropRight"])
        if "cropBottom" in config.keys():
            self.spn_marginBottom.setValue(config["cropBottom"])
        if "labelsToRemove" in config.keys():
            self.cmbLabelsRemove.setLabels(config["labelsToRemove"])
        if "textLayerNames" in config.keys():
            self.ln_text_layer_name.setText(", ".join(config["textLayerNames"]))
        else:
            self.ln_text_layer_name.setText("text")
        if "panelLayerNames" in config.keys():
            self.ln_panel_layer_name.setText(", ".join(config["panelLayerNames"]))
        else:
            self.ln_panel_layer_name.setText("panels")
        self.CBZgroupResize.set_config(config)
        if "CBZactive" in config.keys():
            self.CBZactive.setChecked(config["CBZactive"])
        self.EPUBgroupResize.set_config(config)
        if "EPUBactive" in config.keys():
            self.EPUBactive.setChecked(config["EPUBactive"])
        self.TIFFgroupResize.set_config(config)
        if "TIFFactive" in config.keys():
            self.TIFFactive.setChecked(config["TIFFactive"])

        if "acbfAuthor" in config.keys():
            if isinstance(config["acbfAuthor"], list):
                for author in config["acbfAuthor"]:
                    listItems = []
                    listItems.append(QStandardItem(author.get("nickname", "")))
                    listItems.append(QStandardItem(author.get("first-name", "")))
                    listItems.append(QStandardItem(author.get("initials", "")))
                    listItems.append(QStandardItem(author.get("last-name", "")))
                    listItems.append(QStandardItem(author.get("email", "")))
                    listItems.append(QStandardItem(author.get("homepage", "")))
                    self.ACBFauthorModel.appendRow(listItems)
                pass
            else:
                listItems = []
                listItems.append(QStandardItem(config["acbfAuthor"]))  # Nick name
                for i in range(0, 5):
                    listItems.append(QStandardItem())  # First name
                self.ACBFauthorModel.appendRow(listItems)

        if "uuid" in config.keys():
            self.lnACBFID.setText(config["uuid"])
        elif "acbfID" in config.keys():
            self.lnACBFID.setText(config["acbfID"])
        else:
            config["uuid"] = QUuid.createUuid().toString()
            self.lnACBFID.setText(config["uuid"])
        if "acbfVersion" in config.keys():
            self.spnACBFVersion.setValue(config["acbfVersion"])
        if "acbfHistory" in config.keys():
            for h in config["acbfHistory"]:
                item = QStandardItem()
                item.setText(h)
                self.ACBFhistoryModel.appendRow(item)
        if "acbfStyles" in config.keys():
            styleDict = config.get("acbfStyles", {})
            for key in self.acbfStylesList:
                keyDict = styleDict.get(key, {})
                style = QStandardItem(key.title())
                style.setCheckable(True)
                if key in styleDict.keys():
                    style.setCheckState(Qt.Checked)
                else:
                    style.setCheckState(Qt.Unchecked)
                fontOn = False
                if "font" in keyDict.keys() or "genericfont" in keyDict.keys():
                    fontOn = True
                style.setData(fontOn, role=styleEnum.FONT)
                if "font" in keyDict:
                    fontlist = keyDict["font"]
                    if isinstance(fontlist, list):
                        font = keyDict.get("font", QFont().family())
                        style.setData(font, role=styleEnum.FONTLIST)
                    else:
                        style.setData([fontlist], role=styleEnum.FONTLIST)
                else:
                   style.setData([QFont().family()], role=styleEnum.FONTLIST)
                style.setData(keyDict.get("genericfont", "sans-serif"), role=styleEnum.FONTGENERIC)
                style.setData(keyDict.get("bold", False), role=styleEnum.BOLD)
                style.setData(keyDict.get("ital", False), role=styleEnum.ITALIC)
                self.ACBFStylesModel.appendRow(style)
            keyDict = styleDict.get("general", {})
            self.regularColor.setCurrentColor(QColor(keyDict.get("color", "#000000")))
            square = QPixmap(32, 32)
            square.fill(self.regularColor.currentColor())
            self.btn_acbfRegColor.setIcon(QIcon(square))
            keyDict = styleDict.get("inverted", {})
            self.invertedColor.setCurrentColor(QColor(keyDict.get("color", "#FFFFFF")))
            square.fill(self.invertedColor.currentColor())
            self.btn_acbfInvColor.setIcon(QIcon(square))
        else:
            for key in self.acbfStylesList:
                style = QStandardItem(key.title())
                style.setCheckable(True)
                style.setCheckState(Qt.Unchecked)
                style.setData(False, role=styleEnum.FONT)
                style.setData(QFont().family(), role=styleEnum.FONTLIST)
                style.setData("sans-serif", role=styleEnum.FONTGENERIC)
                style.setData(False, role=styleEnum.BOLD) #Bold
                style.setData(False, role=styleEnum.ITALIC) #Italic
                self.ACBFStylesModel.appendRow(style)
        self.CBZgroupResize.setEnabled(self.CBZactive.isChecked())
        self.lnTranslatorHeader.setText(config.get("translatorHeader", "Translator's Notes"))
        self.chkIncludeTranslatorComments.setChecked(config.get("includeTranslComment", False))

    """
    Store the GUI values into the config dictionary given.
    
    @return the config diactionary filled with new values.
    """

    def getConfig(self, config):

        config["cropToGuides"] = self.chk_toOutmostGuides.isChecked()
        config["cropLeft"] = self.spn_marginLeft.value()
        config["cropTop"] = self.spn_marginTop.value()
        config["cropBottom"] = self.spn_marginRight.value()
        config["cropRight"] = self.spn_marginBottom.value()
        config["labelsToRemove"] = self.cmbLabelsRemove.getLabels()
        config["CBZactive"] = self.CBZactive.isChecked()
        config = self.CBZgroupResize.get_config(config)
        config["EPUBactive"] = self.EPUBactive.isChecked()
        config = self.EPUBgroupResize.get_config(config)
        config["TIFFactive"] = self.TIFFactive.isChecked()
        config = self.TIFFgroupResize.get_config(config)
        authorList = []
        for row in range(self.ACBFauthorTable.verticalHeader().count()):
            logicalIndex = self.ACBFauthorTable.verticalHeader().logicalIndex(row)
            listEntries = ["nickname", "first-name", "initials", "last-name", "email", "homepage"]
            author = {}
            for i in range(len(listEntries)):
                entry = self.ACBFauthorModel.data(self.ACBFauthorModel.index(logicalIndex, i))
                if entry is None:
                    entry = " "
                if entry.isspace() is False and len(entry) > 0:
                    author[listEntries[i]] = entry
                elif listEntries[i] in author.keys():
                    author.pop(listEntries[i])
            authorList.append(author)
        config["acbfAuthor"] = authorList
        config["acbfVersion"] = self.spnACBFVersion.value()
        versionList = []
        for r in range(self.ACBFhistoryModel.rowCount()):
            index = self.ACBFhistoryModel.index(r, 0)
            versionList.append(self.ACBFhistoryModel.data(index, Qt.DisplayRole))
        config["acbfHistory"] = versionList
        
        acbfStylesDict = {}
        for row in range(0, self.ACBFStylesModel.rowCount()):
            entry = self.ACBFStylesModel.item(row)
            if entry.checkState() == Qt.Checked:
                key = entry.text().lower()
                style = {}
                if entry.data(role=styleEnum.FONT):
                    font = entry.data(role=styleEnum.FONTLIST)
                    if font is not None:
                        style["font"] = font
                    genericfont = entry.data(role=styleEnum.FONTGENERIC)
                    if font is not None:
                        style["genericfont"] = genericfont
                bold = entry.data(role=styleEnum.BOLD)
                if bold is not None:
                    style["bold"] = bold
                italic = entry.data(role=styleEnum.ITALIC)
                if italic is not None:
                    style["ital"] = italic
                acbfStylesDict[key] = style
        acbfStylesDict["general"] = {"color": self.regularColor.currentColor().name()}
        acbfStylesDict["inverted"] = {"color": self.invertedColor.currentColor().name()}
        config["acbfStyles"] = acbfStylesDict
        config["translatorHeader"] = self.lnTranslatorHeader.text()
        config["includeTranslComment"] = self.chkIncludeTranslatorComments.isChecked()

        # Turn this into something that retrieves from a line-edit when string freeze is over.
        config["textLayerNames"] = self.ln_text_layer_name.text().split(",")
        config["panelLayerNames"] = self.ln_panel_layer_name.text().split(",")
        return config
