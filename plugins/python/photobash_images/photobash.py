# SPDX-License-Identifier: CC0-1.0

from krita import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
import math

class PhotobashDocker(DockWidget):
    applicationName = "Photobash"
    referencesSetting = "referencesDirectory"
    fitCanvasSetting = "fitToCanvas"
    
    foundImages = []

    directoryPath = ""
    currPage = 0
    currImageScale = 100
    fitCanvasChecked = True
    
    filterTextEdit = None
    mainWidget = None
    changePathButton = None
    sliderLabel = None
    imagesButtons = []

    def __init__(self):
        super().__init__()
        self.setWindowTitle("Photobash Images")

        # Read settings 
        self.directoryPath = Application.readSetting(self.applicationName, self.referencesSetting, "")
        
        if Application.readSetting(self.applicationName, self.fitCanvasSetting, "true") == "true":
            self.fitCanvasChecked = True
        else: 
            self.fitCanvasChecked = False

        self.currImageScale = 100

        self.setLayout()

    
    def canvasChanged(self, canvas):
        pass

    def setLayout(self):
        self.mainWidget = QWidget(self)
        self.setWidget(self.mainWidget)

        # Filtering text
        self.filterTextEdit = QLineEdit(self.mainWidget)
        self.filterTextEdit.setPlaceholderText("Filter images by words...")
        self.filterTextEdit.textChanged.connect(self.updateTextFilter)

        if self.directoryPath != "":
            self.changePathButton = QPushButton("Change References Directory", self.mainWidget)
        else:
            self.changePathButton = QPushButton("Set References Directory", self.mainWidget)
        
        self.changePathButton.clicked.connect(self.changePath)

        mainLayout = QVBoxLayout()
        
        topLayout = QHBoxLayout()
        topLayout.addWidget(self.filterTextEdit)
        topLayout.addWidget(self.changePathButton)

        imagesLayout = QVBoxLayout()

        for i in range(0,3):
            rowLayout = QHBoxLayout()
            
            for j in range(0,3):
                button = QToolButton(self.mainWidget)
                button.setMaximumHeight(3000)
                button.setMaximumWidth(3000)
                button.setMinimumHeight(self.mainWidget.height() / 3)
                button.setMinimumWidth(self.mainWidget.width() / 3)
                
                self.imagesButtons.append(button)
                
                rowLayout.addWidget(button)

            imagesLayout.addLayout(rowLayout)
        
        self.imagesButtons[0].clicked.connect(lambda: self.buttonClick(0))
        self.imagesButtons[1].clicked.connect(lambda: self.buttonClick(1))
        self.imagesButtons[2].clicked.connect(lambda: self.buttonClick(2))
        self.imagesButtons[3].clicked.connect(lambda: self.buttonClick(3))
        self.imagesButtons[4].clicked.connect(lambda: self.buttonClick(4))
        self.imagesButtons[5].clicked.connect(lambda: self.buttonClick(5))
        self.imagesButtons[6].clicked.connect(lambda: self.buttonClick(6))
        self.imagesButtons[7].clicked.connect(lambda: self.buttonClick(7))
        self.imagesButtons[8].clicked.connect(lambda: self.buttonClick(8))

        mainLayout.addLayout(topLayout)
        mainLayout.addLayout(imagesLayout)

        bottomLayout = QHBoxLayout()
        previousButton = QToolButton(self.mainWidget)
        previousButton.setMaximumWidth(3000)
        previousButton.clicked.connect(lambda: self.updateCurrPage(-1))
        previousButton.setArrowType(Qt.ArrowType.LeftArrow)

        nextButton = QToolButton(self.mainWidget)
        nextButton.setMaximumWidth(3000)
        nextButton.clicked.connect(lambda: self.updateCurrPage(1))
        nextButton.setArrowType(Qt.ArrowType.RightArrow)

        self.sliderLabel = QLabel(self.mainWidget)
        self.sliderLabel.setText(f"Scale : {self.currImageScale}%")
        self.sliderLabel.setMaximumWidth(self.sliderLabel.fontMetrics().width(self.sliderLabel.text()))
        
        slider = QSlider(Qt.Horizontal, self)
        slider.setValue(self.currImageScale)
        slider.setMaximum(100)
        slider.setMinimum(10)
        slider.setMaximumWidth(3000)
        slider.valueChanged.connect(self.updateScale)

        fitBordersLabel = QLabel(self.mainWidget)
        fitBordersLabel.setText("Fit Canvas")
        fitBordersLabel.setMaximumWidth(fitBordersLabel.fontMetrics().width(fitBordersLabel.text()))

        fitCanvasCheckBox = QCheckBox(self.mainWidget)
        fitCanvasCheckBox.setCheckState(self.fitCanvasChecked)
        fitCanvasCheckBox.stateChanged.connect(self.changedFitCanvas)
        fitCanvasCheckBox.setTristate(False)
        
        bottomLayout.addWidget(previousButton)
        bottomLayout.addWidget(nextButton)
        bottomLayout.addWidget(self.sliderLabel)
        bottomLayout.addWidget(slider)
        bottomLayout.addWidget(fitBordersLabel)
        bottomLayout.addWidget(fitCanvasCheckBox)

        mainLayout.addLayout(bottomLayout)

        self.mainWidget.setLayout(mainLayout)

        self.updateTextFilter()

    def changedFitCanvas(self, state):
        if state == Qt.Checked: 
            self.fitCanvasChecked = True
            Application.writeSetting(self.applicationName, self.fitCanvasSetting, "true")
        else: 
            self.fitCanvasChecked = False
            Application.writeSetting(self.applicationName, self.fitCanvasSetting, "false")

    def updateScale(self, value):
        self.currImageScale = value
        self.sliderLabel.setText(f"Image Scale : {self.currImageScale}%")

    def updateCurrPage(self, increment):
        if (self.currPage == 0 and increment == -1) or \
            ((self.currPage + 1) * len(self.imagesButtons) > len(self.foundImages) and increment == 1) or \
            len(self.foundImages) == 0:
            return 

        self.currPage += increment
        self.updateImages()

    def updateTextFilter(self):
        newImages = []
        self.currPage = 0

        if self.directoryPath != "":
            it = QDirIterator(self.directoryPath, QDirIterator.Subdirectories)

            while(it.hasNext()): 
                
                stringsInText = self.filterTextEdit.text().lower().split(" ")

                for word in stringsInText: 
                    if word in it.filePath().lower() and (".png" in it.filePath() or ".jpg" in it.filePath() or ".jpeg" in it.filePath()):
                        newImages.append(it.filePath())
                    
                it.next()
            
            if len(self.foundImages) != len(newImages):
                self.foundImages = newImages
                self.updateImages()
            else:
                for i in range(0, len(newImages)):
                    if self.foundImages[i] != newImages[i]:
                        self.foundImages = newImages
                        self.updateImages()
                        return

    def buttonClick(self, position):
        if position < len(self.foundImages) - len(self.imagesButtons) * self.currPage:
            self.addImageLayer(self.foundImages[position + len(self.imagesButtons) * self.currPage])
        
    def updateImages(self):
        maxWidth = 0
        maxHeight = 0
        
        buttonsSize = len(self.imagesButtons)

        for i in range(0, buttonsSize):
            if maxWidth < self.imagesButtons[i].width():
                maxWidth = self.imagesButtons[i].width()
            if maxHeight < self.imagesButtons[i].height():
                maxHeight = self.imagesButtons[i].height()
        
        maxRange = min(len(self.foundImages) - self.currPage * buttonsSize, buttonsSize)

        for i in range(0, len(self.imagesButtons)):
            if i < maxRange:
                icon = QIcon(self.foundImages[i + buttonsSize * self.currPage])

                self.imagesButtons[i].setIcon(icon)
                self.imagesButtons[i].setIconSize(QSize(int(maxWidth), int(maxHeight)))
            else: 
                self.imagesButtons[i].setIconSize(QSize(0,0))

    def changePath(self):
        fileDialog = QFileDialog(QWidget(self));
        fileDialog.setFileMode(QFileDialog.DirectoryOnly);
            
        if self.directoryPath == "":
            self.directoryPath = fileDialog.getExistingDirectory(self.mainWidget, "Change Directory for Images", QStandardPaths.writableLocation(QStandardPaths.PicturesLocation))
            Application.writeSetting(self.applicationName, self.referencesSetting, self.directoryPath)
        else: 
            self.directoryPath = fileDialog.getExistingDirectory(self.mainWidget, "Change Directory for Images", self.directoryPath)
            Application.writeSetting(self.applicationName, self.referencesSetting, self.directoryPath)
        
        self.changePathButton.setText("Change References Directory")
        self.updateTextFilter()
    
    def addImageLayer(self, photoPath):
        # Get the document:
        doc = Krita.instance().activeDocument()
        
        # Saving a non-existent document causes crashes, so lets check for that first.
        if doc is not None:
            root = doc.activeNode().parentNode();
            
            layerName = photoPath.split("/")[len(photoPath.split("/")) - 1].split(".png")[0].split(".jpg")[0].split(".jpeg")[0]
            
            tmpLayer = doc.createNode(layerName, "paintLayer")
            newLayer = doc.createFileLayer(layerName, photoPath, "ImageToPPI")
            
            root.addChildNode(newLayer, None)
            root.addChildNode(tmpLayer, None)

            doc.activeNode().mergeDown()
            
            activeNode = None

            for node in root.childNodes():
                if node.name() == layerName:
                    activeNode = node

            if self.fitCanvasChecked:
                if activeNode.bounds().width() / activeNode.bounds().height() > doc.bounds().width() / doc.bounds().height():
                    scalingFactor = doc.bounds().width() / activeNode.bounds().width()
                    newWidth = doc.bounds().width() * self.currImageScale / 100 
                    newHeight = activeNode.bounds().height() * scalingFactor * self.currImageScale / 100
                else:
                    scalingFactor = doc.bounds().height() / activeNode.bounds().height() 
                    newWidth = activeNode.bounds().width() * scalingFactor * self.currImageScale / 100
                    newHeight = doc.bounds().height() * self.currImageScale / 100 

                    activeNode.scaleNode(QPoint(activeNode.bounds().center().x(),activeNode.bounds().center().y()), int(newWidth), int(newHeight), "Bicubic")
            else: 
                newWidth = activeNode.bounds().width() * self.currImageScale / 100
                newHeight = activeNode.bounds().height() * self.currImageScale / 100 

                activeNode.scaleNode(QPoint(activeNode.bounds().center().x(),activeNode.bounds().center().y()), int(newWidth), int(newHeight), "Bicubic")
            
            # Center image
            offsetX = doc.bounds().width()/2 - activeNode.bounds().center().x() 
            offsetY = doc.bounds().height()/2 - activeNode.bounds().center().y() 

            activeNode.move(int(offsetX), int(offsetY))

            Krita.instance().activeDocument().refreshProjection()
        

