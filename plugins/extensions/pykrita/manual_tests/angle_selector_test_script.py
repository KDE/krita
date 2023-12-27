#
#  SPDX-FileCopyrightText: 2023 Freya Lupen <penguinflyer2222@gmail.com>
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

# This script creates a dialog for manually testing the functions of the AngleSelector.

from krita import AngleSelector
from PyQt5.QtWidgets import QDialog, QFormLayout, QLabel, QSpinBox, QPushButton, QDoubleSpinBox, QLineEdit, QCheckBox
from PyQt5.Qt import Qt

dialog = QDialog()
layout = QFormLayout()

angleSelector = AngleSelector()
layout.addRow("AngleSelector.widget():", angleSelector.widget())

angleChangedLabel = QLabel("0")
layout.addRow("angleChanged:", angleChangedLabel)
def setAngleNum(angle):
    angleChangedLabel.setText(str(angle))
angleSelector.angleChanged.connect(setAngleNum)

coterminalAngleLabel = QLabel("0")
layout.addRow("Closest Coterminal Angle in Range:", coterminalAngleLabel)
def setCoterminalAngle(angle):
    coterminalAngleLabel.setText(str(angleSelector.closestCoterminalAngleInRange(angle)))
angleSelector.angleChanged.connect(setCoterminalAngle)

layout.addWidget(QLabel("Pass values to functions here:"))

angleBox = QDoubleSpinBox()
angleBox.setValue(angleSelector.angle())
angleBox.setRange(0.0, 999.0)
angleBox.valueChanged.connect(angleSelector.setAngle)
layout.addRow("Angle:", angleBox)

resetButton = QPushButton()
resetButton.clicked.connect(angleSelector.reset)
layout.addRow("Reset:", resetButton)

snapAngleBox = QDoubleSpinBox()
snapAngleBox.setValue(angleSelector.snapAngle())
snapAngleBox.setRange(0.0, 360.0)
snapAngleBox.valueChanged.connect(angleSelector.setSnapAngle)
layout.addRow("Snap Angle:", snapAngleBox)

resetAngleBox = QDoubleSpinBox()
resetAngleBox.setValue(angleSelector.resetAngle())
resetAngleBox.setRange(0.0, 360.0)
resetAngleBox.valueChanged.connect(angleSelector.setResetAngle)
layout.addRow("Reset Angle:", resetAngleBox)

numDecimalsBox = QSpinBox()
numDecimalsBox.setValue(angleSelector.decimals())
numDecimalsBox.valueChanged.connect(angleSelector.setDecimals)
layout.addRow("Decimals:", numDecimalsBox)

maxBox = QDoubleSpinBox()
maxBox.setRange(0.0, 360.0)
maxBox.setValue(angleSelector.maximum())
maxBox.valueChanged.connect(angleSelector.setMaximum)
layout.addRow("Maximum:", maxBox)

minBox = QDoubleSpinBox()
minBox.setRange(0.0, 360.0)
minBox.setValue(angleSelector.minimum())
minBox.valueChanged.connect(angleSelector.setMinimum)
layout.addRow("Minimum:", minBox)

layout.addRow("Range", QLabel("(min, max):"))
rangeMinBox = QDoubleSpinBox()
rangeMinBox.setRange(0.0, 360.0)
rangeMinBox.setValue(angleSelector.minimum())
rangeMaxBox = QDoubleSpinBox()
rangeMaxBox.setRange(0.0, 360.0)
rangeMaxBox.setValue(angleSelector.maximum())
def setRange():
    angleSelector.setRange(rangeMinBox.value(), rangeMaxBox.value())
rangeMinBox.valueChanged.connect(setRange)
rangeMaxBox.valueChanged.connect(setRange)
layout.addWidget(rangeMinBox)
layout.addWidget(rangeMaxBox)

prefixLineEdit = QLineEdit()
prefixLineEdit.setText(angleSelector.prefix())
def setPrefix():
    angleSelector.setPrefix(prefixLineEdit.text())
prefixLineEdit.editingFinished.connect(setPrefix)
layout.addRow("Prefix:", prefixLineEdit)

wrappingCheckBox = QCheckBox()
wrappingCheckBox.setChecked(angleSelector.wrapping())
wrappingCheckBox.toggled.connect(angleSelector.setWrapping)
layout.addRow("Wrapping:", wrappingCheckBox)

layout.addRow("Flip Options Mode", QLabel("(NoFlipOptions, MenuButton, Buttons, ContextMenu):"))
flipOptionsModeLineEdit = QLineEdit()
flipOptionsModeLineEdit.setText(angleSelector.flipOptionsMode())
def setFlipMode():
    angleSelector.setFlipOptionsMode(flipOptionsModeLineEdit.text())
flipOptionsModeLineEdit.editingFinished.connect(setFlipMode)
layout.addWidget(flipOptionsModeLineEdit)

widgetsHeightBox = QSpinBox()
widgetsHeightBox.setValue(angleSelector.widgetsHeight())
widgetsHeightBox.valueChanged.connect(angleSelector.setWidgetsHeight)
layout.addRow("Widgets Height:", widgetsHeightBox)

layout.addRow("Increasing Direction", QLabel("(CounterClockwise, Clockwise):"))
increaseDirLineEdit = QLineEdit()
increaseDirLineEdit.setText(angleSelector.increasingDirection())
def setIncreaseDir():
    angleSelector.setIncreasingDirection(increaseDirLineEdit.text())
increaseDirLineEdit.editingFinished.connect(setIncreaseDir)
layout.addWidget(increaseDirLineEdit)

useFlatBoxCheckBox = QCheckBox()
useFlatBoxCheckBox.toggled.connect(angleSelector.useFlatSpinBox)
layout.addRow("Flat SpinBox:", useFlatBoxCheckBox)

flipHButton = QPushButton()
def flipHorizontal():
    angleSelector.flip(Qt.Orientation.Horizontal)
flipHButton.clicked.connect(flipHorizontal)
layout.addRow("Flip (horizontal):", flipHButton)

flipVButton = QPushButton()
def flipVertical():
    angleSelector.flip(Qt.Orientation.Vertical)
flipVButton.clicked.connect(flipVertical)
layout.addRow("Flip (vertical):", flipVButton)

dialog.setLayout(layout)
dialog.exec()
