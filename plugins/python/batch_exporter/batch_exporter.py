"""
GDQuest Batch Exporter
-----------------
Batch export art assets from Krita using layer metadata.
Updates and reads metadata in Krita's layer names, and uses it to smartly process and export layers.
Export to the Blender Cut-Out Animation tools for modular 2d game animation.
SPDX-License-Identifier: GPL-3.0-only
"""

from functools import partial
from krita import DockWidget, DockWidgetFactory, DockWidgetFactoryBase, Krita
from PyQt5.QtWidgets import (
    QPushButton,
    QStatusBar,
    QLabel,
    QLineEdit,
    QHBoxLayout,
    QVBoxLayout,
    QGroupBox,
    QWidget,
)
import os
from .Config import CONFIG
from .Infrastructure import WNode
from .COATools import COAToolsFormat
from .Utils import kickstart, flip
from .Utils.Tree import iterPre

KI = Krita.instance()


def ensureRGBAU8(doc):
    ensured = doc.colorModel() == "RGBA" and doc.colorDepth() == "U8"
    if not ensured:
        raise ValueError("only RGBA 8-bit depth supported!")


def exportAllLayers(cfg, statusBar):
    msg, timeout = (cfg["done"]["msg"].format("Exported all layers."), cfg["done"]["timeout"])
    try:
        doc = KI.activeDocument()

        root = doc.rootNode()
        root = WNode(cfg, root)

        dirName = os.path.dirname(doc.fileName())
        it = filter(lambda n: n.isExportable() and n.isMarked(), iterPre(root))
        it = map(partial(flip(WNode.save), dirName), it)
        kickstart(it)
    except ValueError as e:
        msg, timeout = cfg["error"]["msg"].format(e), cfg["error"]["timeout"]
    statusBar.showMessage(msg, timeout)


def exportSelectedLayers(cfg, statusBar):
    msg, timeout = (cfg["done"]["msg"].format("Exported selected layers."), cfg["done"]["timeout"])
    try:
        doc = KI.activeDocument()

        dirName = os.path.dirname(doc.fileName())
        nodes = KI.activeWindow().activeView().selectedNodes()
        it = map(partial(WNode, cfg), nodes)
        it = map(partial(flip(WNode.save), dirName), it)
        kickstart(it)
    except ValueError as e:
        msg, timeout = cfg["error"]["msg"].format(e), cfg["error"]["timeout"]
    statusBar.showMessage(msg, timeout)


def exportCOATools(mode, cfg, statusBar):
    msg, timeout = (
        cfg["done"]["msg"].format("Exported %s layers to COA Tools format." % (mode)),
        cfg["done"]["timeout"],
    )
    try:
        doc = KI.activeDocument()
        ensureRGBAU8(doc)

        coat_format = COAToolsFormat(cfg, statusBar)
        dirName = os.path.dirname(doc.fileName())
        nodes = KI.activeWindow().activeView().selectedNodes()

        # If mode is document or no nodes are selected, use document root
        if mode == "document" or len(nodes) == 0:
            nodes = [doc.rootNode()]

        it = map(partial(WNode, cfg), nodes)
        # By convention all selected nodes should be Group Layers
        # This is to represent a logical root for each export in COATools format
        it = filter(lambda n: n.isGroupLayer(), it)
        it = map(coat_format.collect, it)
        kickstart(it)
        coat_format.save(dirName)

    except ValueError as e:
        msg, timeout = cfg["error"]["msg"].format(e), cfg["error"]["timeout"]
    statusBar.showMessage(msg, timeout)


def renameLayers(cfg, statusBar, lineEdit):
    msg, timeout = (cfg["done"]["msg"].format("Renaming successful!"), cfg["done"]["timeout"])
    try:
        nodes = KI.activeWindow().activeView().selectedNodes()
        it = map(partial(WNode, cfg), nodes)
        it = map(partial(flip(WNode.rename), lineEdit.text()), it)
        kickstart(it)
    except ValueError as e:
        msg, timeout = cfg["error"]["msg"].format(e), cfg["error"]["timeout"]
    statusBar.showMessage(msg, timeout)


class GameArtTools(DockWidget):
    title = "Batch Exporter"

    def __init__(self):
        super().__init__()
        KI.setBatchmode(True)
        self.setWindowTitle(self.title)
        self.createInterface()

    def createInterface(self):
        uiContainer = QWidget(self)

        exportLabel = QLabel("Export")
        exportAllLayersButton = QPushButton("All Layers")
        exportSelectedLayersButton = QPushButton("Selected Layers")
        renameLabel = QLabel("Update Name and Metadata")
        renameLineEdit = QLineEdit()
        renameButton = QPushButton()
        renameButton.setIcon(KI.icon("view-refresh"))
        statusBar = QStatusBar()

        exportLabel.setToolTip("Export individual images")
        exportAllLayersButton.setToolTip("Export all layers with metadata")
        exportSelectedLayersButton.setToolTip("Export selected layers only")
        renameButton.setToolTip("Batch update selected layer names and metadata")

        # COA Tools GroupBox
        coaToolsGroupBox = QGroupBox("COA Tools")
        coaToolsHBoxLayout = QHBoxLayout()
        coaToolsExportSelectedLayersButton = QPushButton("Selected Layers")
        coaToolsExportDocumentButton = QPushButton("Document")

        coaToolsGroupBox.setToolTip("Blender Cut-Out Animation Tools")
        coaToolsExportSelectedLayersButton.setToolTip("Export selected layers only")
        coaToolsExportDocumentButton.setToolTip("Export all layers with metadata")

        coaToolsHBoxLayout.addWidget(coaToolsExportDocumentButton)
        coaToolsHBoxLayout.addWidget(coaToolsExportSelectedLayersButton)
        coaToolsGroupBox.setLayout(coaToolsHBoxLayout)

        vboxlayout = QVBoxLayout()
        vboxlayout.addWidget(exportLabel)

        vboxlayout.addWidget(exportAllLayersButton)
        vboxlayout.addWidget(exportSelectedLayersButton)

        vboxlayout.addWidget(coaToolsGroupBox)
        vboxlayout.addWidget(renameLabel)

        hboxlayout = QHBoxLayout()
        hboxlayout.addWidget(renameLineEdit)
        hboxlayout.addWidget(renameButton)

        vboxlayout.addLayout(hboxlayout)
        vboxlayout.addStretch()
        vboxlayout.addWidget(statusBar)

        uiContainer.setLayout(vboxlayout)
        self.setWidget(uiContainer)

        exportSelectedLayersButton.released.connect(
            partial(exportSelectedLayers, CONFIG, statusBar)
        )
        exportAllLayersButton.released.connect(partial(exportAllLayers, CONFIG, statusBar))
        coaToolsExportSelectedLayersButton.released.connect(
            partial(exportCOATools, "selected", CONFIG, statusBar)
        )
        coaToolsExportDocumentButton.released.connect(
            partial(exportCOATools, "document", CONFIG, statusBar)
        )
        renameLineEdit.returnPressed.connect(
            partial(renameLayers, CONFIG, statusBar, renameLineEdit)
        )
        renameButton.released.connect(partial(renameLayers, CONFIG, statusBar, renameLineEdit))

    def canvasChanged(self, canvas):
        pass


def registerDocker():
    docker = DockWidgetFactory(
        "pykrita_gdquest_art_tools", DockWidgetFactoryBase.DockRight, GameArtTools
    )
    KI.addDockWidgetFactory(docker)
