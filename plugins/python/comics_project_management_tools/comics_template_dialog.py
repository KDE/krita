"""
SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

This file is part of the Comics Project Management Tools(CPMT).

SPDX-License-Identifier: GPL-3.0-or-later
"""

"""
Template dialog
"""
import os
import shutil
#from PyQt5.QtGui import *
from PyQt5.QtWidgets import QDialog, QComboBox, QDialogButtonBox, QVBoxLayout, QFormLayout, QGridLayout, QWidget, QPushButton, QHBoxLayout, QLabel, QSpinBox, QDoubleSpinBox, QLineEdit, QTabWidget, QColorDialog, QProgressDialog
from PyQt5.QtCore import QLocale, Qt, QByteArray, QRectF
from PyQt5.QtGui import QImage, QPainter, QPixmap
from krita import *
"""
Quick and dirty QComboBox subclassing that handles unitconversion for us.
"""


class simpleUnitBox(QComboBox):
    pixels = i18n("Pixels")
    inches = i18n("Inches")
    centimeter = i18n("Centimeter")
    millimeter = i18n("millimeter")

    def __init__(self):
        super(simpleUnitBox, self).__init__()
        self.addItem(self.pixels)
        self.addItem(self.inches)
        self.addItem(self.centimeter)
        self.addItem(self.millimeter)

        if QLocale().system().measurementSystem() is QLocale.MetricSystem:
            self.setCurrentIndex(2)  # set to centimeter if metric system.
        else:
            self.setCurrentIndex(1)

    def pixelsForUnit(self, unit, DPI):
        if (self.currentText() == self.pixels):
            return unit
        elif (self.currentText() == self.inches):
            return self.inchesToPixels(unit, DPI)
        elif (self.currentText() == self.centimeter):
            return self.centimeterToPixels(unit, DPI)
        elif (self.currentText() == self.millimeter):
            return self.millimeterToPixels(unit, DPI)

    def inchesToPixels(self, inches, DPI):
        return DPI * inches

    def centimeterToInches(self, cm):
        return cm / 2.54

    def centimeterToPixels(self, cm, DPI):
        return self.inchesToPixels(self.centimeterToInches(cm), DPI)

    def millimeterToCentimeter(self, mm):
        return mm / 10

    def millimeterToPixels(self, mm, DPI):
        return self.inchesToPixels(self.centimeterToInches(self.millimeterToCentimeter(mm)), DPI)


class comics_template_dialog(QDialog):
    templateDirectory = str()
    templates = QComboBox()
    buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)

    def __init__(self, templateDirectory):
        super().__init__()
        self.templateDirectory = templateDirectory
        self.setWindowTitle(i18n("Add new Template"))
        self.setLayout(QVBoxLayout())

        self.templates = QComboBox()
        self.templates.setEnabled(False)

        self.buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        self.buttons.accepted.connect(self.accept)
        self.buttons.rejected.connect(self.reject)
        self.buttons.button(QDialogButtonBox.Ok).setEnabled(False)
        mainWidget = QWidget()
        self.layout().addWidget(mainWidget)
        self.layout().addWidget(self.buttons)
        mainWidget.setLayout(QVBoxLayout())

        btn_create = QPushButton(i18n("Create Template"))
        btn_create.clicked.connect(self.slot_create_template)
        btn_import = QPushButton(i18n("Import Templates"))
        btn_import.clicked.connect(self.slot_import_template)
        mainWidget.layout().addWidget(self.templates)
        mainWidget.layout().addWidget(btn_create)
        mainWidget.layout().addWidget(btn_import)
        self.fill_templates()

    def fill_templates(self):
        self.templates.clear()
        for entry in os.scandir(self.templateDirectory):
            if entry.name.endswith('.kra') and entry.is_file():
                name = os.path.relpath(entry.path, self.templateDirectory)
                self.templates.addItem(name)
        if self.templates.model().rowCount() > 0:
            self.templates.setEnabled(True)
            self.buttons.button(QDialogButtonBox.Ok).setEnabled(True)

    def slot_create_template(self):
        create = comics_template_create(self.templateDirectory)

        if create.exec_() == QDialog.Accepted:
            if (create.prepare_krita_file()):
                self.fill_templates()

    def slot_import_template(self):
        filenames = QFileDialog.getOpenFileNames(caption=i18n("Which files should be added to the template folder?"), directory=self.templateDirectory, filter=str(i18n("Krita files") + "(*.kra)"))[0]
        for file in filenames:
            shutil.copy2(file, self.templateDirectory)
        self.fill_templates()

    def url(self):
        return os.path.join(self.templateDirectory, self.templates.currentText())


class comics_template_create(QDialog):
    urlSavedTemplate = str()
    templateDirectory = str()
    currentColor = QColor(Qt.white)

    def __init__(self, templateDirectory):
        super().__init__()
        self.templateDirectory = templateDirectory
        self.setWindowTitle(i18n("Create new Template"))
        self.setLayout(QVBoxLayout())
        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        mainWidget = QWidget()
        explanation = QLabel(i18n("This allows you to make a template document with guides.\nThe width and height are the size of the live-area, the safe area is the live area minus the margins, and the full image is the live area plus the bleeds."))
        explanation.setWordWrap(True)
        self.layout().addWidget(explanation)
        self.layout().addWidget(mainWidget)
        self.layout().addWidget(buttons)
        mainWidget.setLayout(QHBoxLayout())
        elements = QWidget()
        elements.setLayout(QVBoxLayout())
        previewLayout = QWidget()
        previewLayout.setLayout(QVBoxLayout())
        mainWidget.layout().addWidget(elements)
        mainWidget.layout().addWidget(previewLayout)
        self.imagePreview = QLabel()
        self.imagePreview.setMinimumSize(256, 256)
        previewLayout.layout().addWidget(self.imagePreview)

        self.templateName = QLineEdit()
        self.templateName.setPlaceholderText("...")
        elements.layout().addWidget(self.templateName)
        
        templateBGColor = QPushButton(i18n("Select background color"))
        templateBGColor.clicked.connect(self.slot_call_color_dialog)
        previewLayout.layout().addWidget(templateBGColor)

        self.DPI = QSpinBox()
        self.DPI.setMaximum(1200)
        self.DPI.setValue(300)
        self.spn_width = QDoubleSpinBox()
        self.spn_width.setMaximum(10000)
        self.spn_height = QDoubleSpinBox()
        self.spn_height.setMaximum(10000)
        self.widthUnit = simpleUnitBox()
        self.heightUnit = simpleUnitBox()

        widgetSize = QWidget()
        sizeForm = QFormLayout()
        sizeForm.addRow(i18n("DPI:"), self.DPI)
        widthLayout = QHBoxLayout()
        widthLayout.addWidget(self.spn_width)
        widthLayout.addWidget(self.widthUnit)
        sizeForm.addRow(i18n("Width:"), widthLayout)
        heightLayout = QHBoxLayout()
        heightLayout.addWidget(self.spn_height)
        heightLayout.addWidget(self.heightUnit)
        sizeForm.addRow(i18n("Height:"), heightLayout)
        widgetSize.setLayout(sizeForm)
        elements.layout().addWidget(widgetSize)

        marginAndBleed = QTabWidget()
        elements.layout().addWidget(marginAndBleed)

        margins = QWidget()
        marginForm = QGridLayout()
        margins.setLayout(marginForm)
        self.marginLeft = QDoubleSpinBox()
        self.marginLeft.setMaximum(1000)
        self.marginLeftUnit = simpleUnitBox()
        self.marginRight = QDoubleSpinBox()
        self.marginRight.setMaximum(1000)
        self.marginRightUnit = simpleUnitBox()
        self.marginTop = QDoubleSpinBox()
        self.marginTop.setMaximum(1000)
        self.marginTopUnit = simpleUnitBox()
        self.marginBottom = QDoubleSpinBox()
        self.marginBottom.setMaximum(1000)
        self.marginBottomUnit = simpleUnitBox()
        marginForm.addWidget(QLabel(i18n("Left:")), 0, 0, Qt.AlignRight)
        marginForm.addWidget(self.marginLeft, 0, 1)
        marginForm.addWidget(self.marginLeftUnit, 0, 2)
        marginForm.addWidget(QLabel(i18n("Top:")), 1, 0, Qt.AlignRight)
        marginForm.addWidget(self.marginTop, 1, 1)
        marginForm.addWidget(self.marginTopUnit, 1, 2)
        marginForm.addWidget(QLabel(i18n("Right:")), 2, 0, Qt.AlignRight)
        marginForm.addWidget(self.marginRight, 2, 1)
        marginForm.addWidget(self.marginRightUnit, 2, 2)
        marginForm.addWidget(QLabel(i18n("Bottom:")), 3, 0, Qt.AlignRight)
        marginForm.addWidget(self.marginBottom, 3, 1)
        marginForm.addWidget(self.marginBottomUnit, 3, 2)
        marginAndBleed.addTab(margins, i18n("Margins"))

        bleeds = QWidget()
        bleedsForm = QGridLayout()
        bleeds.setLayout(bleedsForm)
        self.bleedLeft = QDoubleSpinBox()
        self.bleedLeft.setMaximum(1000)
        self.bleedLeftUnit = simpleUnitBox()
        self.bleedRight = QDoubleSpinBox()
        self.bleedRight.setMaximum(1000)
        self.bleedRightUnit = simpleUnitBox()
        self.bleedTop = QDoubleSpinBox()
        self.bleedTop.setMaximum(1000)
        self.bleedTopUnit = simpleUnitBox()
        self.bleedBottom = QDoubleSpinBox()
        self.bleedBottom.setMaximum(1000)
        self.bleedBottomUnit = simpleUnitBox()
        bleedsForm.addWidget(QLabel(i18n("Left:")), 0, 0, Qt.AlignRight)
        bleedsForm.addWidget(self.bleedLeft, 0, 1)
        bleedsForm.addWidget(self.bleedLeftUnit, 0, 2)
        bleedsForm.addWidget(QLabel(i18n("Top:")), 1, 0, Qt.AlignRight)
        bleedsForm.addWidget(self.bleedTop, 1, 1)
        bleedsForm.addWidget(self.bleedTopUnit, 1, 2)
        bleedsForm.addWidget(QLabel(i18n("Right:")), 2, 0, Qt.AlignRight)
        bleedsForm.addWidget(self.bleedRight, 2, 1)
        bleedsForm.addWidget(self.bleedRightUnit, 2, 2)
        bleedsForm.addWidget(QLabel(i18n("Bottom:")), 3, 0, Qt.AlignRight)
        bleedsForm.addWidget(self.bleedBottom, 3, 1)
        bleedsForm.addWidget(self.bleedBottomUnit, 3, 2)

        marginAndBleed.addTab(bleeds, i18n("Bleeds"))

        if QLocale().system().measurementSystem() is QLocale.MetricSystem:
            self.setDefaults("European")
        else:
            self.setDefaults("American")

        self.spn_width.valueChanged.connect(self.updateImagePreview)
        self.widthUnit.currentIndexChanged.connect(self.updateImagePreview)
        self.spn_height.valueChanged.connect(self.updateImagePreview)
        self.heightUnit.currentIndexChanged.connect(self.updateImagePreview)
        self.marginLeft.valueChanged.connect(self.updateImagePreview)
        self.marginLeftUnit.currentIndexChanged.connect(self.updateImagePreview)
        self.marginRight.valueChanged.connect(self.updateImagePreview)
        self.marginRightUnit.currentIndexChanged.connect(self.updateImagePreview)
        self.marginTop.valueChanged.connect(self.updateImagePreview)
        self.marginTopUnit.currentIndexChanged.connect(self.updateImagePreview)
        self.marginBottom.valueChanged.connect(self.updateImagePreview)
        self.marginBottomUnit.currentIndexChanged.connect(self.updateImagePreview)
        self.bleedLeft.valueChanged.connect(self.updateImagePreview)
        self.bleedLeftUnit.currentIndexChanged.connect(self.updateImagePreview)
        self.bleedRight.valueChanged.connect(self.updateImagePreview)
        self.bleedRightUnit.currentIndexChanged.connect(self.updateImagePreview)
        self.bleedTop.valueChanged.connect(self.updateImagePreview)
        self.bleedTopUnit.currentIndexChanged.connect(self.updateImagePreview)
        self.bleedBottom.valueChanged.connect(self.updateImagePreview)
        self.bleedBottomUnit.currentIndexChanged.connect(self.updateImagePreview)

        self.updateImagePreview()

    def slot_call_color_dialog(self):
        dialog = QColorDialog(self)
        dialog.setCurrentColor(self.currentColor)
        
        if dialog.exec_() == QDialog.Accepted:
            self.currentColor = dialog.currentColor()
            self.updateImagePreview()

    def prepare_krita_file(self):
        wBase = max(self.widthUnit.pixelsForUnit(self.spn_width.value(), self.DPI.value()), 1)
        bL = self.bleedLeftUnit.pixelsForUnit(self.bleedLeft.value(), self.DPI.value())
        bR = self.bleedRightUnit.pixelsForUnit(self.bleedRight.value(), self.DPI.value())
        mL = self.marginLeftUnit.pixelsForUnit(self.marginLeft.value(), self.DPI.value())
        mR = self.marginRightUnit.pixelsForUnit(self.marginRight.value(), self.DPI.value())

        hBase = max(self.heightUnit.pixelsForUnit(self.spn_height.value(), self.DPI.value()), 1)
        bT = self.bleedTopUnit.pixelsForUnit(self.bleedTop.value(), self.DPI.value())
        bB = self.bleedBottomUnit.pixelsForUnit(self.bleedBottom.value(), self.DPI.value())
        mT = self.marginTopUnit.pixelsForUnit(self.marginTop.value(), self.DPI.value())
        mB = self.marginBottomUnit.pixelsForUnit(self.marginBottom.value(), self.DPI.value())

        template = Application.createDocument(int(wBase + bL + bR), int(hBase + bT + bB), self.templateName.text(), "RGBA", "U8", "sRGB built-in", self.DPI.value())

        backgroundName = i18n("Background")
        if len(template.topLevelNodes()) > 0:
            backgroundNode = template.topLevelNodes()[0]
            backgroundNode.setName(backgroundName)
        else:
            backgroundNode = template.createNode(backgroundName, "paintlayer")
            template.rootNode().addChildNode(backgroundNode, None)
        
        pixelByteArray = QByteArray()
        if self.currentColor == Qt.white:
            pixelByteArray = backgroundNode.pixelData(0, 0, template.width(), template.height())
            pixelByteArray.fill(int(255).to_bytes(1, byteorder='little'))
        else:
            red   = int(self.currentColor.redF()*255).to_bytes(1, byteorder='little')
            green = int(self.currentColor.greenF()*255).to_bytes(1, byteorder='little')
            blue  = int(self.currentColor.blueF()*255).to_bytes(1, byteorder='little')
            alpha = int(self.currentColor.alphaF()*255).to_bytes(1, byteorder='little')
        
            progress = QProgressDialog(i18n("Creating template"), str(), 0, template.width()*template.height())
            progress.setCancelButton(None)
            progress.show()
            qApp.processEvents()
            for byteNumber in range(template.width()*template.height()):
                pixelByteArray.append( blue)
                pixelByteArray.append(green)
                pixelByteArray.append(  red)
                pixelByteArray.append(alpha)
                progress.setValue(byteNumber)
        backgroundNode.setPixelData(pixelByteArray, 0, 0, template.width(), template.height())
        backgroundNode.setOpacity(255)
        backgroundNode.setLocked(True)

        sketchNode = template.createNode(i18n("Sketch"), "paintlayer")
        template.rootNode().addChildNode(sketchNode, backgroundNode)

        verticalGuides = []
        verticalGuides.append(bL)
        verticalGuides.append(bL + mL)
        verticalGuides.append((bL + wBase) - mR)
        verticalGuides.append(bL + wBase)

        horizontalGuides = []
        horizontalGuides.append(bT)
        horizontalGuides.append(bT + mT)
        horizontalGuides.append((bT + hBase) - mB)
        horizontalGuides.append(bT + hBase)

        template.setHorizontalGuides(horizontalGuides)
        template.setVerticalGuides(verticalGuides)
        template.setGuidesVisible(True)
        template.setGuidesLocked(True)
        template.refreshProjection()

        self.urlSavedTemplate = os.path.join(self.templateDirectory, self.templateName.text() + ".kra")
        success = template.exportImage(self.urlSavedTemplate, InfoObject())
        print("CPMT: Template", self.templateName.text(), "made and saved.")
        template.waitForDone()
        template.close()

        return success

    def updateImagePreview(self):
        maxSize = 256

        wBase = max(self.widthUnit.pixelsForUnit(self.spn_width.value(), self.DPI.value()), 1)
        bL = self.bleedLeftUnit.pixelsForUnit(self.bleedLeft.value(), self.DPI.value())
        bR = self.bleedRightUnit.pixelsForUnit(self.bleedRight.value(), self.DPI.value())
        mL = self.marginLeftUnit.pixelsForUnit(self.marginLeft.value(), self.DPI.value())
        mR = self.marginRightUnit.pixelsForUnit(self.marginRight.value(), self.DPI.value())

        hBase = max(self.heightUnit.pixelsForUnit(self.spn_height.value(), self.DPI.value()), 1)
        bT = self.bleedTopUnit.pixelsForUnit(self.bleedTop.value(), self.DPI.value())
        bB = self.bleedBottomUnit.pixelsForUnit(self.bleedBottom.value(), self.DPI.value())
        mT = self.marginTopUnit.pixelsForUnit(self.marginTop.value(), self.DPI.value())
        mB = self.marginBottomUnit.pixelsForUnit(self.marginBottom.value(), self.DPI.value())

        scaleRatio = maxSize / (hBase + bT + bB)
        if wBase > hBase:
            scaleRatio = maxSize / (wBase + bR + bL)

        width = (wBase + bL + bR) * scaleRatio
        height = (hBase + bT + bB) * scaleRatio
        topLeft = [max((maxSize - width) / 2, 0), max((maxSize - height) / 2, 0)]

        image = QImage(maxSize, maxSize, QImage.Format_ARGB32)
        image.fill(Qt.transparent)

        p = QPainter(image)

        p.setBrush(self.currentColor)

        CanvasSize = QRectF(topLeft[0], topLeft[1], width, height)
        p.drawRect(CanvasSize.toRect())

        # Draw bleeds.
        PageSize = CanvasSize
        PageSize.setWidth(width - (bR * scaleRatio))
        PageSize.setHeight(height - (bB * scaleRatio))
        PageSize.setX(PageSize.x() + (bL * scaleRatio))
        PageSize.setY(PageSize.y() + (bT * scaleRatio))

        p.setPen(Qt.blue)
        p.setBrush(Qt.transparent)
        p.drawRect(PageSize.toRect())

        # Draw liveArea
        LiveArea = PageSize
        LiveArea.setWidth(LiveArea.width() - (mR * scaleRatio))
        LiveArea.setHeight(LiveArea.height() - (mB * scaleRatio))
        LiveArea.setX(LiveArea.x() + (mL * scaleRatio))
        LiveArea.setY(LiveArea.y() + (mT * scaleRatio))

        p.setPen(Qt.blue)
        p.drawRect(LiveArea.toRect())

        p.end()

        self.imagePreview.setPixmap(QPixmap.fromImage(image))

    def setDefaults(self, type):

        if type == "American":
            # American 11x17 inch
            self.spn_width.setValue(11)
            self.widthUnit.setCurrentIndex(1)
            self.spn_height.setValue(17)
            self.heightUnit.setCurrentIndex(1)
            self.bleedBottom.setValue(1)
            self.bleedBottomUnit.setCurrentIndex(1)
            self.bleedTop.setValue(1)
            self.bleedTopUnit.setCurrentIndex(1)
            self.bleedLeft.setValue(0.5)
            self.bleedLeftUnit.setCurrentIndex(1)
            self.bleedRight.setValue(0.5)
            self.bleedRightUnit.setCurrentIndex(1)
            self.marginBottom.setValue(0.745)
            self.marginBottomUnit.setCurrentIndex(1)
            self.marginTop.setValue(0.745)
            self.marginTopUnit.setCurrentIndex(1)
            self.marginRight.setValue(0.5)
            self.marginRightUnit.setCurrentIndex(1)
            self.marginLeft.setValue(0.5)
            self.marginLeftUnit.setCurrentIndex(1)
        if type == "European":
            # European A4
            self.spn_width.setValue(21)
            self.widthUnit.setCurrentIndex(2)
            self.spn_height.setValue(29.7)
            self.heightUnit.setCurrentIndex(2)
            self.bleedBottom.setValue(5)
            self.bleedBottomUnit.setCurrentIndex(3)
            self.bleedTop.setValue(5)
            self.bleedTopUnit.setCurrentIndex(3)
            self.bleedLeft.setValue(5)
            self.bleedLeftUnit.setCurrentIndex(3)
            self.bleedRight.setValue(5)
            self.bleedRightUnit.setCurrentIndex(3)
            self.marginBottom.setValue(1.5)
            self.marginBottomUnit.setCurrentIndex(2)
            self.marginTop.setValue(1.5)
            self.marginTopUnit.setCurrentIndex(2)
            self.marginRight.setValue(1)
            self.marginRightUnit.setCurrentIndex(2)
            self.marginLeft.setValue(1)
            self.marginLeftUnit.setCurrentIndex(2)

    def url(self):
        return self.urlSavedTemplate
