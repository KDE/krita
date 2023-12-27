#
#  SPDX-FileCopyrightText: 2023 Freya Lupen <penguinflyer2222@gmail.com>
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

# This script creates a dialog for manually testing the functions
# of the SliderSpinBox and DoubleSliderSpinBox.

from krita import SliderSpinBox, DoubleSliderSpinBox
from PyQt5.QtWidgets import QDialog, QHBoxLayout, QFormLayout, QLabel, QSpinBox, QDoubleSpinBox, QCheckBox

dialog = QDialog()
hLayout = QHBoxLayout()

# SliderSpinBox --
layout = QFormLayout()

sliderSpinBox = SliderSpinBox()
layout.addRow("SliderSpinBox.widget():", sliderSpinBox.widget())

finishedLabel = QLabel("0")
layout.addRow("draggingFinished:", finishedLabel)
def intChanged():
    finishedLabel.setText(str(sliderSpinBox.widget().value()))
sliderSpinBox.draggingFinished.connect(intChanged)

isDraggingLabel = QLabel(str(sliderSpinBox.isDragging()))
def setDraggingLabel():
    isDraggingLabel.setText(str(sliderSpinBox.isDragging()))
sliderSpinBox.widget().valueChanged.connect(setDraggingLabel)
sliderSpinBox.draggingFinished.connect(setDraggingLabel)
layout.addRow("Is Dragging:", isDraggingLabel)

layout.addWidget(QLabel("Pass values to functions here:"))

setIntBox = QSpinBox()
setIntBox.setValue(sliderSpinBox.widget().value())
setIntBox.valueChanged.connect(sliderSpinBox.setValue)
layout.addRow("Value:", setIntBox)

layout.addRow("Range", QLabel("(min, max, computeNewFastSliderStep):"))
rangeMinBox = QSpinBox()
rangeMinBox.setMaximum(999)
rangeMaxBox = QSpinBox()
rangeMaxBox.setMaximum(999)
rangeRecomputeStepBox = QCheckBox()
def setRange():
    sliderSpinBox.setRange(rangeMinBox.value(), rangeMaxBox.value(), rangeRecomputeStepBox.isChecked())
rangeMinBox.valueChanged.connect(setRange)
rangeMaxBox.valueChanged.connect(setRange)
rangeRecomputeStepBox.clicked.connect(setRange)
layout.addWidget(rangeMinBox)
layout.addWidget(rangeMaxBox)
layout.addWidget(rangeRecomputeStepBox)
layout.addWidget(QLabel()) # empty line to somewhat align with the double side

layout.addRow("Minimum", QLabel("(computeNewFastSliderStep):"))
minBox = QSpinBox()
minBox.setMaximum(999)
minRecomputeStepBox = QCheckBox()
def setMin():
    sliderSpinBox.setMinimum(minBox.value(), minRecomputeStepBox.isChecked())
minBox.valueChanged.connect(setMin)
minRecomputeStepBox.clicked.connect(setMin)
layout.addWidget(minBox)
layout.addWidget(minRecomputeStepBox)

layout.addRow("Maximum", QLabel("(computeNewFastSliderStep):"))
maxBox = QSpinBox()
maxBox.setMaximum(999)
maxRecomputeStepBox = QCheckBox()
def setMax():
    sliderSpinBox.setMaximum(maxBox.value(), maxRecomputeStepBox.isChecked())
maxBox.valueChanged.connect(setMax)
maxRecomputeStepBox.clicked.connect(setMax)
layout.addWidget(maxBox)
layout.addWidget(maxRecomputeStepBox)

exponentRatioBox = QDoubleSpinBox()
exponentRatioBox.setValue(1) # 1 is the default value (there's no getter function)
exponentRatioBox.valueChanged.connect(sliderSpinBox.setExponentRatio)
layout.addRow("Exponent Ratio:", exponentRatioBox)

blockSignalOnDragBox = QCheckBox()
blockSignalOnDragBox.clicked.connect(sliderSpinBox.setBlockUpdateSignalOnDrag)
layout.addRow("Block Update Signal On Drag:", blockSignalOnDragBox)

fastStepBox = QSpinBox()
fastStepBox.setValue(sliderSpinBox.fastSliderStep())
fastStepBox.valueChanged.connect(sliderSpinBox.setFastSliderStep)
layout.addRow("Fast Slider Step:", fastStepBox)

layout.addRow("Soft Range", QLabel("(min, max):"))
softRangeMinBox = QSpinBox()
softRangeMinBox.setMaximum(999)
softRangeMaxBox = QSpinBox()
softRangeMaxBox.setMaximum(999)
softRangeRecomputeStepBox = QCheckBox()
def setSoftRange():
    sliderSpinBox.setSoftRange(softRangeMinBox.value(), softRangeMaxBox.value())
softRangeMinBox.valueChanged.connect(setSoftRange)
softRangeMaxBox.valueChanged.connect(setSoftRange)
layout.addWidget(softRangeMinBox)
layout.addWidget(softRangeMaxBox)

softMinBox = QSpinBox()
softMinBox.setMaximum(999)
softMinBox.setValue(sliderSpinBox.softMinimum())
softMinBox.valueChanged.connect(sliderSpinBox.setSoftMinimum)
layout.addRow("Soft Minimum:", softMinBox)

softMaxBox = QSpinBox()
softMaxBox.setMaximum(999)
softMaxBox.setValue(sliderSpinBox.softMaximum())
softMaxBox.valueChanged.connect(sliderSpinBox.setSoftMaximum)
layout.addRow("Soft Maximum:", softMaxBox)

hLayout.addLayout(layout)

#
hLayout.addWidget(QLabel()) # add a little spacing

# DoubleSliderSpinBox --
dblLayout = QFormLayout()

doubleSliderSpinBox = DoubleSliderSpinBox()
dblLayout.addRow("DoubleSliderSpinBox.widget():", doubleSliderSpinBox.widget())

dblFinishedLabel = QLabel("0.00")
dblLayout.addRow("draggingFinished:", dblFinishedLabel)
def doubleChanged():
    dblFinishedLabel.setText(str(doubleSliderSpinBox.widget().value()))
doubleSliderSpinBox.draggingFinished.connect(doubleChanged)

dblIsDraggingBox = QLabel(str(doubleSliderSpinBox.isDragging()))
def dblSetDraggingLabel():
    dblIsDraggingBox.setText(str(doubleSliderSpinBox.isDragging()))
doubleSliderSpinBox.widget().valueChanged.connect(dblSetDraggingLabel)
doubleSliderSpinBox.draggingFinished.connect(dblSetDraggingLabel)
dblLayout.addRow("Is Dragging:", dblIsDraggingBox)

dblLayout.addWidget(QLabel("Pass values to functions here:"))

setDoubleBox = QDoubleSpinBox()
setDoubleBox.setValue(doubleSliderSpinBox.widget().value())
setDoubleBox.valueChanged.connect(doubleSliderSpinBox.setValue)
dblLayout.addRow("Value:", setDoubleBox)

dblLayout.addRow("Range", QLabel("(min, max, numDecimals, computeNewFastSliderStep):"))
dblRangeMinBox = QDoubleSpinBox()
dblRangeMinBox.setMaximum(999)
dblRangeMaxBox = QDoubleSpinBox()
dblRangeMaxBox.setMaximum(999)
dblNumDecimalsBox = QSpinBox()
dblRangeRecomputeStepBox = QCheckBox()
def dblSetRange():
    doubleSliderSpinBox.setRange(dblRangeMinBox.value(), dblRangeMaxBox.value(),
                                 dblNumDecimalsBox.value(), dblRangeRecomputeStepBox.isChecked())
dblRangeMinBox.valueChanged.connect(dblSetRange)
dblRangeMaxBox.valueChanged.connect(dblSetRange)
dblNumDecimalsBox.valueChanged.connect(dblSetRange)
dblRangeRecomputeStepBox.clicked.connect(dblSetRange)
dblLayout.addWidget(dblRangeMinBox)
dblLayout.addWidget(dblRangeMaxBox)
dblLayout.addWidget(dblNumDecimalsBox)
dblLayout.addWidget(dblRangeRecomputeStepBox)

dblLayout.addRow("Minimum", QLabel("(computeNewFastSliderStep):"))
dblMinBox = QDoubleSpinBox()
dblMinBox.setMaximum(999)
dblMinRecomputeStepBox = QCheckBox()
def dblSetMin():
    doubleSliderSpinBox.setMinimum(dblMinBox.value(), dblMinRecomputeStepBox.isChecked())
dblMinBox.valueChanged.connect(dblSetMin)
dblMinRecomputeStepBox.clicked.connect(dblSetMin)
dblLayout.addWidget(dblMinBox)
dblLayout.addWidget(dblMinRecomputeStepBox)

dblLayout.addRow("Maximum", QLabel("(computeNewFastSliderStep):"))
dblMaxBox = QDoubleSpinBox()
dblMaxBox.setMaximum(999)
dblMaxRecomputeStepBox = QCheckBox()
def dblSetMax():
    doubleSliderSpinBox.setMaximum(dblMaxBox.value(), dblMaxRecomputeStepBox.isChecked())
dblMaxBox.valueChanged.connect(dblSetMax)
dblMaxRecomputeStepBox.clicked.connect(dblSetMax)
dblLayout.addWidget(dblMaxBox)
dblLayout.addWidget(dblMaxRecomputeStepBox)

dblExponentRatioBox = QDoubleSpinBox()
dblExponentRatioBox.setValue(1) # 1 is the default value (there's no getter function)
dblExponentRatioBox.valueChanged.connect(doubleSliderSpinBox.setExponentRatio)
dblLayout.addRow("Exponent Ratio:", dblExponentRatioBox)

dblBlockSignalOnDragBox = QCheckBox()
dblBlockSignalOnDragBox.clicked.connect(doubleSliderSpinBox.setBlockUpdateSignalOnDrag)
dblLayout.addRow("Block Update Signal On Drag:", dblBlockSignalOnDragBox)

dblFastStepBox = QDoubleSpinBox()
dblFastStepBox.setValue(doubleSliderSpinBox.fastSliderStep())
dblFastStepBox.valueChanged.connect(doubleSliderSpinBox.setFastSliderStep)
dblLayout.addRow("Fast Slider Step:", dblFastStepBox)

dblLayout.addRow("Soft Range", QLabel("(min, max):"))
dblSoftRangeMinBox = QDoubleSpinBox()
dblSoftRangeMinBox.setMaximum(999)
dblSoftRangeMaxBox = QDoubleSpinBox()
dblSoftRangeMaxBox.setMaximum(999)
def dblSetSoftRange():
    doubleSliderSpinBox.setSoftRange(dblSoftRangeMinBox.value(), dblSoftRangeMaxBox.value())
dblSoftRangeMinBox.valueChanged.connect(dblSetSoftRange)
dblSoftRangeMaxBox.valueChanged.connect(dblSetSoftRange)
dblLayout.addWidget(dblSoftRangeMinBox)
dblLayout.addWidget(dblSoftRangeMaxBox)

dblSoftMinBox = QDoubleSpinBox()
dblSoftMinBox.setMaximum(999)
dblSoftMinBox.setValue(doubleSliderSpinBox.softMinimum())
dblSoftMinBox.valueChanged.connect(doubleSliderSpinBox.setSoftMinimum)
dblLayout.addRow("Soft Minimum:", dblSoftMinBox)

dblSoftMaxBox = QDoubleSpinBox()
dblSoftMaxBox.setMaximum(999)
dblSoftMaxBox.setValue(doubleSliderSpinBox.softMaximum())
dblSoftMaxBox.valueChanged.connect(doubleSliderSpinBox.setSoftMaximum)
dblLayout.addRow("Soft Maximum:", dblSoftMaxBox)

hLayout.addLayout(dblLayout)

# --

dialog.setLayout(hLayout)
dialog.exec()
