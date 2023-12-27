#
#  SPDX-FileCopyrightText: 2023 Freya Lupen <penguinflyer2222@gmail.com>
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

# This script creates a dialog for manually testing the functions
# of the IntParseSpinBox and DoubleParseSpinBox.

from krita import IntParseSpinBox, DoubleParseSpinBox
from PyQt5.QtWidgets import QDialog, QFormLayout, QHBoxLayout, QSpinBox, QDoubleSpinBox, QLabel, QCheckBox

dialog = QDialog()
hLayout = QHBoxLayout()

# IntParseSpinBox --
layout = QFormLayout()

intParseSpinBox = IntParseSpinBox()
layout.addRow("IntParseSpinBox.widget():", intParseSpinBox.widget())

intValueChangedLabel = QLabel()
def setIntVal(value):
    intValueChangedLabel.setText(str(value))
intParseSpinBox.widget().valueChanged.connect(setIntVal)
layout.addRow("widget().valueChanged:", intValueChangedLabel)

isValidLabel = QLabel()
def setValid(_value):
    isValidLabel.setText(str(intParseSpinBox.isLastValid()))
intParseSpinBox.widget().valueChanged.connect(setValid)
layout.addRow("Is Last Valid:", isValidLabel)

statusLabel = QLabel("Valid")
def setError():
    statusLabel.setText("Error")
def setOkay():
    statusLabel.setText("Valid")
intParseSpinBox.errorWhileParsing.connect(setError)
intParseSpinBox.noMoreParsingError.connect(setOkay)
layout.addRow("Status:", statusLabel)

layout.addWidget(QLabel("Pass values to functions here:"))

stepByBox = QSpinBox()
stepByBox.valueChanged.connect(intParseSpinBox.stepBy)
layout.addRow("Step By:", stepByBox)

valueBox = QSpinBox()
valueBox.setMaximum(100)
overwriteCheckBox = QCheckBox()
def setValue(_value):
    intParseSpinBox.setValue(valueBox.value(), overwriteCheckBox.isChecked())
valueBox.valueChanged.connect(setValue)
overwriteCheckBox.clicked.connect(setValue)
layout.addRow("Value", valueBox)
layout.addRow("  (overWriteExisting):", overwriteCheckBox)
testOverwriteCheckBox = QCheckBox()
def toggleOverwriteTest():
    if testOverwriteCheckBox.isChecked():
        intParseSpinBox.widget().textChanged.connect(setValue)
    else:
        intParseSpinBox.widget().textChanged.disconnect(setValue)
testOverwriteCheckBox.clicked.connect(toggleOverwriteTest)
layout.addRow("  Test overwrite while typing?", testOverwriteCheckBox)

hLayout.addLayout(layout)

#
hLayout.addWidget(QLabel()) # add a little spacing

# DoubleParseSpinBox --
dblLayout = QFormLayout()

doubleParseSpinBox = DoubleParseSpinBox()
dblLayout.addRow("DoubleParseSpinBox.widget():", doubleParseSpinBox.widget())

dblValueChangedLabel = QLabel()
def setDoubleVal(value):
    dblValueChangedLabel.setText(str(value))
doubleParseSpinBox.widget().valueChanged.connect(setDoubleVal)
dblLayout.addRow("widget().valueChanged:", dblValueChangedLabel)

dblIsValidLabel = QLabel()
def dblSetValid(_value):
    dblIsValidLabel.setText(str(doubleParseSpinBox.isLastValid()))
doubleParseSpinBox.widget().valueChanged.connect(dblSetValid)
dblLayout.addRow("Is Last Valid:", dblIsValidLabel)

dblStatusLabel = QLabel("Valid")
def dblSetError():
    dblStatusLabel.setText("Error")
def dblSetOkay():
    dblStatusLabel.setText("Valid")
doubleParseSpinBox.errorWhileParsing.connect(dblSetError)
doubleParseSpinBox.noMoreParsingError.connect(dblSetOkay)
dblLayout.addRow("Status:", dblStatusLabel)

dblLayout.addWidget(QLabel("Pass values to functions here:"))

dblStepByBox = QSpinBox()
dblStepByBox.valueChanged.connect(doubleParseSpinBox.stepBy)
dblLayout.addRow("Step By:", dblStepByBox)

dblValueBox = QDoubleSpinBox()
dblOverwriteCheckBox = QCheckBox()
def dblSetValue(_value):
    doubleParseSpinBox.setValue(dblValueBox.value(), dblOverwriteCheckBox.isChecked())
dblValueBox.valueChanged.connect(dblSetValue)
dblOverwriteCheckBox.clicked.connect(dblSetValue)
dblLayout.addRow("Value", dblValueBox)
dblLayout.addRow("  (overwriteExisting):", dblOverwriteCheckBox)
dblTestOverwriteCheckBox = QCheckBox()
def dblToggleOverwriteTest():
    if dblTestOverwriteCheckBox.isChecked():
        doubleParseSpinBox.widget().textChanged.connect(dblSetValue)
    else:
        doubleParseSpinBox.widget().textChanged.disconnect(dblSetValue)
dblTestOverwriteCheckBox.clicked.connect(dblToggleOverwriteTest)
dblLayout.addRow("  Test overwrite while typing?", dblTestOverwriteCheckBox)

hLayout.addLayout(dblLayout)

#--

dialog.setLayout(hLayout)
dialog.exec()
