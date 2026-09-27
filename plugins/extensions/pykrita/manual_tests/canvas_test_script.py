#
#  SPDX-FileCopyrightText: 2026 Freya Lupen <penguinflyer2222@gmail.com>
#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

# This script creates a dialog for manually testing the functions of the Canvas.

from krita import Krita, AngleSelector, Canvas

try:
    from PyQt6.QtWidgets import QDialog, QFormLayout, QLabel, QDoubleSpinBox, QCheckBox, QPushButton
    from PyQt6.QtCore import Qt, QPoint, QPointF, QSize, QRectF
except ModuleNotFoundError:
    from PyQt5.QtWidgets import QDialog, QFormLayout, QLabel, QDoubleSpinBox, QCheckBox, QPushButton
    from PyQt5.QtCore import Qt, QPoint, QPointF, QSize, QRectF

class CanvasTest:

    # Variables to prevent being deleted
    canvas: Canvas
    angleSelector : AngleSelector

    def __init__(self):
        kritaInstance = Krita.instance()
        assert kritaInstance
        window = kritaInstance.activeWindow()
        assert window
        view = window.activeView()
        assert view
        canvas = view.canvas()
        assert canvas
        self.canvas = canvas

        dialog = QDialog(window.qwindow())
        layout = QFormLayout()

        # Zoom level
        zoomLevelBox = QDoubleSpinBox()
        def setZoomLevelBox():
            zoomLevelBox.setValue(self.canvas.zoomLevel())
        setZoomLevelBox()
        self.canvas.effectiveZoomChanged.connect(setZoomLevelBox)
        zoomLevelBox.valueChanged.connect(self.canvas.setZoomLevel)
        layout.addRow("Zoom Level:", zoomLevelBox)

        # Effective zoom
        effectiveZoomLabel = QLabel("unknown")
        def onEffectiveZoomChanged(effectiveZoom: float):
            effectiveZoomLabel.setText(f"{effectiveZoom:.2f}")
        self.canvas.effectiveZoomChanged.connect(onEffectiveZoomChanged)
        layout.addRow("Effective zoom:", effectiveZoomLabel)

        # Reset zoom
        resetZoomButton = QPushButton("Reset zoom")
        resetZoomButton.clicked.connect(self.canvas.resetZoom)
        layout.addWidget(resetZoomButton)

        # Zoom state (mode, zoom, min, max)
        zoomModeLabel = QLabel("unknown")
        zoomLabel = QLabel("unknown")
        zoomMinLabel = QLabel("unknown")
        zoomMaxLabel = QLabel("unknown")
        def onZoomStateChanged(zoomMode: Canvas.ZoomMode, zoom: float, minZoom: float, maxZoom: float):
            zoomModeLabel.setText(str(zoomMode))
            zoomLabel.setText(f"{zoom:.2f}")
            zoomMinLabel.setText(f"{minZoom:.2f}")
            zoomMaxLabel.setText(f"{maxZoom:.2f}")
        self.canvas.zoomStateChanged.connect(onZoomStateChanged)
        layout.addRow("Zoom State: Mode:", zoomModeLabel)
        layout.addRow("Zoom State: Zoom:", zoomLabel)
        layout.addRow("Zoom State: Min:", zoomMinLabel)
        layout.addRow("Zoom State: Max:", zoomMaxLabel)

        # Rotation
        self.angleSelector = AngleSelector()
        self.angleSelector.setFlipOptionsMode("NoFlipOptions")
        def setAngle(angle: float):
            angle, ok = self.angleSelector.closestCoterminalAngleInRange(angle)
            if ok:
                self.angleSelector.setAngle(angle)
        setAngle(self.canvas.rotation())
        self.canvas.rotationChanged.connect(setAngle)
        self.angleSelector.angleChanged.connect(self.canvas.setRotation)
        layout.addRow("Rotation:", self.angleSelector.widget())

        # Reset rotation
        resetRotationButton = QPushButton("Reset rotation")
        resetRotationButton.clicked.connect(self.canvas.resetRotation)
        layout.addWidget(resetRotationButton)

        # Mirror
        hMirrorBox = QCheckBox("Horizontal mirror")
        vMirrorBox = QCheckBox("Vertical mirror (noop)")
        def onMirrorChanged(hMirror: bool, vMirror: bool):
            hMirrorBox.setChecked(hMirror)
            vMirrorBox.setChecked(vMirror)
        self.canvas.mirrorChanged.connect(onMirrorChanged)
        hMirrorBox.toggled.connect(self.canvas.setMirror)
        # there is no corresponding vertical mirror function...
        layout.addWidget(hMirrorBox)
        layout.addWidget(vMirrorBox)

        # Preferred center
        centerBoxX = QDoubleSpinBox()
        centerBoxX.setRange(0.0, 9999.99)
        centerBoxY = QDoubleSpinBox()
        centerBoxY.setRange(0.0, 9999.99)
        def setPreferredCenterBox():
            centerBoxX.setValue(self.canvas.preferredCenter().x())
            centerBoxY.setValue(self.canvas.preferredCenter().y())
        setPreferredCenterBox()
        self.canvas.offsetChanged.connect(setPreferredCenterBox)
        centerBoxX.editingFinished.connect(lambda: self.canvas.setPreferredCenter(
                                            QPointF(centerBoxX.value(), centerBoxY.value())))
        centerBoxY.editingFinished.connect(lambda: self.canvas.setPreferredCenter(
                                            QPointF(centerBoxX.value(), centerBoxY.value())))
        layout.addRow("Preferred center:", centerBoxX)
        layout.addWidget(centerBoxY)
        layout.addWidget(QLabel("(on editingFinished)"))

        # Cursor position
        cursorLabel = QLabel("unknown")
        def onCursorPositionChanged(pos: QPoint):
            cursorLabel.setText(f"({pos.x()}, {pos.y()})")
        self.canvas.cursorPositionChanged.connect(onCursorPositionChanged)
        layout.addRow("Cursor position (viewport):", cursorLabel)

        # Document cursor position
        docCursorLabel = QLabel("unknown")
        def onDocCursorPositionChanged(pos: QPointF):
            docCursorLabel.setText(f"({pos.x():.2f}, {pos.y():.2f})")
        self.canvas.documentCursorPositionChanged.connect(onDocCursorPositionChanged)
        layout.addRow("Cursor position (image):", docCursorLabel)

        # Size of viewport
        sizeLabel = QLabel("unknown")
        def onSizeChanged(size: QSize):
            sizeLabel.setText(f"({size.width()}, {size.height()})")
        self.canvas.sizeChanged.connect(onSizeChanged)
        layout.addRow("Viewport size:", sizeLabel)

        # Offset
        offsetNumLabel = QLabel("0")
        def onOffsetChanged():
            offsetNumLabel.setText(str(int(offsetNumLabel.text()) + 1))
        self.canvas.offsetChanged.connect(onOffsetChanged)
        layout.addRow("Times offset changed:", offsetNumLabel)

        # Document offset from the viewport
        docOffsetOldLabel = QLabel("unknown")
        docOffsetNewLabel = QLabel("unknown")
        def onDocumentOffsetChanged(oldOffset: QPointF, newOffset: QPointF):
            docOffsetOldLabel.setText(f"({oldOffset.x():.2f}, {oldOffset.y():.2f})")
            docOffsetNewLabel.setText(f"({newOffset.x():.2f}, {newOffset.y():.2f})")
        self.canvas.documentOffsetChanged.connect(onDocumentOffsetChanged)
        layout.addRow("Document offset (old):", docOffsetOldLabel)
        layout.addRow("Document offset (new):", docOffsetNewLabel)

        # Pan
        panBoxX = QDoubleSpinBox()
        panBoxX.setRange(-9999.99, 9999.99)
        panBoxY = QDoubleSpinBox()
        panBoxY.setRange(-9999.99, 9999.99)
        panButton = QPushButton("Pan");
        panButton.clicked.connect(lambda: self.canvas.pan(
                                      QPoint(int(panBoxX.value()), int(panBoxY.value()))))
        layout.addRow("Pan by:", panBoxX)
        layout.addWidget(panBoxY)
        layout.addWidget(panButton)

        # Document rect in widget pixels
        rectLabel = QLabel("unknown")
        def onRectChanged(rect: QRectF):
            rectLabel.setText(f"({rect.x():.2f}, {rect.y():.2f}, {rect.width():.2f}, {rect.height():.2f})")
        self.canvas.documentRectInWidgetPixelsChanged.connect(onRectChanged)
        layout.addRow("Rect:", rectLabel)

        # State
        stateNumLabel = QLabel("0")
        def onStateChanged():
            stateNumLabel.setText(str(int(stateNumLabel.text()) + 1))
        self.canvas.stateChanged.connect(onStateChanged)
        layout.addRow("Times state changed:", stateNumLabel)

        # Wrap around mode
        wrapBox = QCheckBox("Wrap around (slot only)")
        def setWrapBox():
            wrapBox.setChecked(self.canvas.wrapAroundMode())
        setWrapBox()
        # no update signal to connect to
        wrapBox.toggled.connect(self.canvas.setWrapAroundMode)
        layout.addWidget(wrapBox)

        # Level of detail mode
        loDBox = QCheckBox("Level of detail (slot only)")
        def setLoDBox():
            loDBox.setChecked(self.canvas.levelOfDetailMode())
        setLoDBox()
        # no update signal to connect to
        loDBox.toggled.connect(self.canvas.setLevelOfDetailMode)
        layout.addWidget(loDBox)

        dialog.setLayout(layout)
        dialog.setWindowFlag(Qt.WindowType.WindowStaysOnTopHint)

        # Make sure the canvas gets disconnected if it's been closed,
        # to avoid crashing when trying to interact with it.
        self.canvas.removed.connect(self.canvas.deleteLater)

        dialog.show()


test = CanvasTest()
