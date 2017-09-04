"""
Part of the comics project management tools (CPMT).

A dialog for editing the exporter settings.
"""

from PyQt5.QtGui import QStandardItem, QStandardItemModel, QColor
from PyQt5.QtWidgets import QDialog, QDialogButtonBox, QGroupBox, QFormLayout, QCheckBox, QComboBox, QSpinBox, QWidget, QVBoxLayout, QTabWidget, QPushButton, QLineEdit, QLabel, QListView
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
        self.setTitle("Adjust Workingfile")
        formLayout = QFormLayout()
        self.setLayout(formLayout)
        self.crop = QCheckBox(i18n("Crop files before resize."))
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

        if method is 0:
            self.spn_PER.setEnabled(True)
        if method is 1:
            self.spn_DPI.setEnabled(True)
        if method is 2:
            self.spn_width.setEnabled(True)
        if method is 3:
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

    def __init__(self):
        super().__init__()
        self.setLayout(QVBoxLayout())
        self.setWindowTitle(i18n("Export settings"))
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
        groupExportCrop = QGroupBox(i18n("Crop settings"))
        formCrop = QFormLayout()
        groupExportCrop.setLayout(formCrop)
        self.chk_toOutmostGuides = QCheckBox(i18n("Crop to outmost guides"))
        self.chk_toOutmostGuides.setChecked(True)
        self.chk_toOutmostGuides.setToolTip(i18n("This will crop to the outmost guides if possible and otherwise use the underlying crop settings."))
        formCrop.addRow("", self.chk_toOutmostGuides)
        btn_fromSelection = QPushButton(i18n("Set margins from active selection"))
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
        CBZgroupMeta = QGroupBox(i18n("Metadata to add"))
        # CBZexportSettings.layout().addWidget(CBZgroupMeta)
        CBZgroupMeta.setLayout(QFormLayout())

        mainWidget.addTab(CBZexportSettings, "CBZ")

        # ACBF, crop, resize, creator name, version history, panel layer, text layers.
        ACBFExportSettings = QWidget()
        ACBFform = QFormLayout()
        ACBFExportSettings.setLayout(QVBoxLayout())
        ACBFdocInfo = QGroupBox()
        ACBFdocInfo.setTitle(i18n("ACBF Document Info"))
        ACBFdocInfo.setLayout(ACBFform)
        self.lnACBFAuthor = QLineEdit()
        self.lnACBFAuthor.setToolTip(i18n("The person responsible for the generation of the CBZ."))
        self.lnACBFSource = QLineEdit()
        self.lnACBFSource.setToolTip(i18n("Whether the acbf file is an adaption of an existing source, and if so, how to find information about that source. So for example, for an adapted webcomic, the official website url should go here."))
        self.lnACBFID = QLabel()
        self.lnACBFID.setToolTip(i18n("By default this will be filled with a generated universal unique identifier. The ID by itself is merely so that comic book library management programs can figure out if this particular comic is already in their database and whether it has been rated. Of course, the UUID can be changed into something else by manually changing the json, but this is advanced usage."))
        self.spnACBFVersion = QSpinBox()
        self.ACBFhistoryModel = QStandardItemModel()
        acbfHistoryList = QListView()
        acbfHistoryList.setModel(self.ACBFhistoryModel)
        btn_add_history = QPushButton(i18n("Add history entry"))
        btn_add_history.clicked.connect(self.slot_add_history_item)

        ACBFform.addRow(i18n("Author-name:"), self.lnACBFAuthor)
        ACBFform.addRow(i18n("Source:"), self.lnACBFSource)
        ACBFform.addRow(i18n("ACBF UID:"), self.lnACBFID)
        ACBFform.addRow(i18n("Version:"), self.spnACBFVersion)
        ACBFform.addRow(i18n("Version History:"), acbfHistoryList)
        ACBFform.addRow("", btn_add_history)

        ACBFExportSettings.layout().addWidget(ACBFdocInfo)
        mainWidget.addTab(ACBFExportSettings, "ACBF")

        # Epub export, crop, resize, other questions.
        EPUBexportSettings = QWidget()
        EPUBexportSettings.setLayout(QVBoxLayout())
        self.EPUBactive = QCheckBox(i18n("Export to EPUB"))
        EPUBexportSettings.layout().addWidget(self.EPUBactive)
        self.EPUBgroupResize = comic_export_resize_widget("EPUB")
        EPUBexportSettings.layout().addWidget(self.EPUBgroupResize)
        self.EPUBactive.clicked.connect(self.EPUBgroupResize.setEnabled)
        mainWidget.addTab(EPUBexportSettings, "EPUB")

        # For Print. Crop, no resize.
        TIFFExportSettings = QWidget()
        TIFFExportSettings.setLayout(QVBoxLayout())
        self.TIFFactive = QCheckBox(i18n("Export to TIFF"))
        TIFFExportSettings.layout().addWidget(self.TIFFactive)
        self.TIFFgroupResize = comic_export_resize_widget("TIFF")
        TIFFExportSettings.layout().addWidget(self.TIFFgroupResize)
        self.TIFFactive.clicked.connect(self.TIFFgroupResize.setEnabled)
        mainWidget.addTab(TIFFExportSettings, "TIFF")

        # SVG, crop, resize, embed vs link.
        #SVGExportSettings = QWidget()

        #mainWidget.addTab(SVGExportSettings, "SVG")

    """
    Add a history item to the acbf version history list.
    """

    def slot_add_history_item(self):
        newItem = QStandardItem()
        newItem.setText("v" + str(self.spnACBFVersion.value()) + "-" + i18n("in this version..."))
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
            self.lnACBFAuthor.setText(config["acbfAuthor"])
        if "acbfSource" in config.keys():
            self.lnACBFSource.setText(config["acbfSource"])
        if "acbfID" in config.keys():
            self.lnACBFID.setText(config["acbfID"])
        else:
            self.lnACBFID.setText(QUuid.createUuid().toString())
        if "acbfVersion" in config.keys():
            self.spnACBFVersion.setValue(config["acbfVersion"])
        if "acbfHistory" in config.keys():
            for h in config["acbfHistory"]:
                item = QStandardItem()
                item.setText(h)
                self.ACBFhistoryModel.appendRow(item)
        self.CBZgroupResize.setEnabled(self.CBZactive.isChecked())

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
        config["acbfAuthor"] = self.lnACBFAuthor.text()
        config["acbfSource"] = self.lnACBFSource.text()
        config["acbfID"] = self.lnACBFID.text()
        config["acbfVersion"] = self.spnACBFVersion.value()
        versionList = []
        for r in range(self.ACBFhistoryModel.rowCount()):
            index = self.ACBFhistoryModel.index(r, 0)
            versionList.append(self.ACBFhistoryModel.data(index, Qt.DisplayRole))
        config["acbfHistory"] = versionList
        return config
