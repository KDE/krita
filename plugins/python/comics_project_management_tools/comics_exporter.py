"""
Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

This file is part of the Comics Project Management Tools(CPMT).

CPMT is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CPMT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the CPMT.  If not, see <http://www.gnu.org/licenses/>.
"""

"""
An exporter that take the comicsConfig and uses it to generate several files.
"""
import sys
from pathlib import Path
import zipfile
from xml.dom import minidom
from xml.etree import ElementTree as ET
import types
import re
from PyQt5.QtWidgets import QLabel, QProgressDialog, qApp  # For the progress dialog.
from PyQt5.QtCore import QElapsedTimer, QLocale, Qt, QRectF, QPointF
from PyQt5.QtGui import QImage, QTransform, QPainterPath
from krita import *
from . import exporters

"""
The sizesCalculator is a convenience class for interpretting the resize configuration
from the export settings dialog. It is also used for batch resize.
"""


class sizesCalculator():

    def __init__(self):
        pass

    def get_scale_from_resize_config(self, config, listSizes):
        listScaleTo = listSizes
        oldWidth = listSizes[0]
        oldHeight = listSizes[1]
        oldXDPI = listSizes[2]
        oldYDPI = listSizes[3]
        if "Method" in config.keys():
            method = config["Method"]
            if method is 0:
                # percentage
                percentage = config["Percentage"] / 100
                listScaleTo[0] = round(oldWidth * percentage)
                listScaleTo[1] = round(oldHeight * percentage)
            if method is 1:
                # dpi
                DPI = config["DPI"]
                listScaleTo[0] = round((oldWidth / oldXDPI) * DPI)
                listScaleTo[1] = round((oldHeight / oldYDPI) * DPI)
                listScaleTo[2] = DPI
                listScaleTo[3] = DPI
            if method is 2:
                # maximum width
                width = config["Width"]
                listScaleTo[0] = width
                listScaleTo[1] = round((oldHeight / oldWidth) * width)
            if method is 3:
                # maximum height
                height = config["Height"]
                listScaleTo[1] = height
                listScaleTo[0] = round((oldWidth / oldHeight) * height)
        return listScaleTo


"""
The comicsExporter is a class that batch exports to all the requested formats.
Make it, set_config with the right data, and then call up "export".

The majority of the functions are meta-data encoding functions.
"""


class comicsExporter():
    acbfLocation = str()
    acbfPageData = []
    cometLocation = str()
    comicRackInfo = str()
    pagesLocationList = {}

    # set of keys used to define specific export behaviour for this page.
    pageKeys = ["acbf_title", "acbf_none", "acbf_fade", "acbf_blend", "acbf_horizontal", "acbf_vertical"]

    def __init__(self):
        pass

    """
    The the configuration of the exporter.

    @param config: A dictionary containing all the config.

    @param projectUrl: the main location of the project folder.
    """

    def set_config(self, config, projectURL):
        self.configDictionary = config
        self.projectURL = projectURL
        self.pagesLocationList = {}
        self.acbfLocation = str()
        self.acbfPageData = []
        self.cometLocation = str()
        self.comicRackInfo = str()

    """
    Export everything according to config and get yourself a coffee.
    This won't work if the config hasn't been set.
    """

    def export(self):
        export_success = False

        path = Path(self.projectURL)
        if path.exists():
            # Make a meta-data folder so we keep the export folder nice and clean.
            exportPath = path / self.configDictionary["exportLocation"]
            if Path(exportPath / "metadata").exists() is False:
                Path(exportPath / "metadata").mkdir()

            # Get to which formats to export, and set the sizeslist.
            sizesList = {}
            if "CBZ" in self.configDictionary.keys():
                if self.configDictionary["CBZactive"]:
                    sizesList["CBZ"] = self.configDictionary["CBZ"]
            if "EPUB" in self.configDictionary.keys():
                if self.configDictionary["EPUBactive"]:
                    sizesList["EPUB"] = self.configDictionary["EPUB"]
            if "TIFF" in self.configDictionary.keys():
                if self.configDictionary["TIFFactive"]:
                    sizesList["TIFF"] = self.configDictionary["TIFF"]
            # Export the pngs according to the sizeslist.
            export_success = self.save_out_pngs(sizesList)

            # Export acbf metadata.
            if export_success:
                if "CBZ" in sizesList.keys():
                    title = self.configDictionary["projectName"]
                    if "title" in self.configDictionary.keys():
                        title = self.configDictionary["title"]
                    
                    self.acbfLocation = str(exportPath / "metadata"/ str(title + ".acbf"))
                    
                    locationStandAlone = str(exportPath / str(title + ".acbf"))
                    
                    export_success = exporters.ACBF.write_xml(self.configDictionary, self.acbfPageData, self.pagesLocationList["CBZ"], self.acbfLocation, locationStandAlone)
                    print("CPMT: Exported to ACBF", export_success)

            # Export and package CBZ and Epub.
            if export_success:
                if "CBZ" in sizesList.keys():
                    export_success = self.export_to_cbz(exportPath)
                    print("CPMT: Exported to CBZ", export_success)
                if "EPUB" in sizesList.keys():
                    export_success = exporters.EPUB.export(self.configDictionary, self.projectURL, self.pagesLocationList["EPUB"])
                    print("CPMT: Exported to EPUB", export_success)
        else:
            print("CPMT: Nothing to export, url not set.")

        return export_success

    """
    This calls up all the functions necessary for making a cbz.
    """

    def export_to_cbz(self, exportPath):
        title = self.configDictionary["projectName"]
        if "title" in self.configDictionary.keys():
            title = self.configDictionary["title"]
        self.cometLocation = str(exportPath / "metadata"/ str(title + " CoMet.xml"))
        export_success = exporters.CoMet.write_xml(self.configDictionary, self.pagesLocationList["CBZ"], self.cometLocation)
        self.comicRackInfo = str(exportPath / "metadata"/ "ComicInfo.xml")
        export_success = exporters.comic_rack_xml.write_xml(self.configDictionary, self.pagesLocationList["CBZ"], self.comicRackInfo)
        self.package_cbz(exportPath)
        return export_success

    def save_out_pngs(self, sizesList):
        # A small fix to ensure crop to guides is set.
        if "cropToGuides" not in self.configDictionary.keys():
            self.configDictionary["cropToGuides"] = False

        # Check if we have pages at all...
        if "pages" in self.configDictionary.keys():

            # Check if there's export methods, and if so make sure the appropriate dictionaries are initialised.
            if len(sizesList.keys()) < 1:
                print("CPMT: Export failed because there's no export methods set.")
                return False
            else:
                for key in sizesList.keys():
                    self.pagesLocationList[key] = []

            # Get the appropriate paths.
            path = Path(self.projectURL)
            exportPath = path / self.configDictionary["exportLocation"]
            pagesList = self.configDictionary["pages"]
            fileName = str(exportPath)

            # Create a progress dialog.
            progress = QProgressDialog("Preparing export.", str(), 0, len(pagesList))
            progress.setWindowTitle("Exporting comic...")
            progress.setCancelButton(None)
            timer = QElapsedTimer()
            timer.start()
            progress.show()
            qApp.processEvents()

            for p in range(0, len(pagesList)):

                # Update the label in the progress dialog.
                progress.setValue(p)
                timePassed = timer.elapsed()
                if (p > 0):
                    timeEstimated = (len(pagesList) - p) * (timePassed / p)
                    passedString = str(int(timePassed / 60000)) + ":" + format(int(timePassed / 1000), "02d") + ":" + format(timePassed % 1000, "03d")
                    estimatedString = str(int(timeEstimated / 60000)) + ":" + format(int(timeEstimated / 1000), "02d") + ":" + format(int(timeEstimated % 1000), "03d")
                    progress.setLabelText(str(i18n("{pages} of {pagesTotal} done. \nTime passed: {passedString}:\n Estimated:{estimated}")).format(pages=p, pagesTotal=len(pagesList), passedString=passedString, estimated=estimatedString))
                qApp.processEvents()
                # Get the appropriate url and open the page.
                url = str(Path(self.projectURL)/ pagesList[p])
                page = Application.openDocument(url)
                page.waitForDone()

                # remove layers and flatten.
                labelList = self.configDictionary["labelsToRemove"]
                panelsAndText = []

                # These three lines are what is causing the page not to close.
                root = page.rootNode()
                self.getPanelsAndText(root, panelsAndText)
                self.removeLayers(labelList, root)
                page.refreshProjection()
                # We'll need the offset and scale for aligning the panels and text correctly. We're getting this from the CBZ

                pageData = {}
                pageData["vector"] = panelsAndText
                tree = ET.fromstring(page.documentInfo())
                pageData["title"] = page.name()
                calligra = "{http://www.calligra.org/DTD/document-info}"
                about = tree.find(calligra + "about")
                keywords = about.find(calligra + "keyword")
                keys = str(keywords.text).split(",")
                pKeys = []
                for key in keys:
                    if key in self.pageKeys:
                        pKeys.append(key)
                pageData["keys"] = pKeys
                page.flatten()
                page.waitForDone()
                batchsave = Application.batchmode()
                Application.setBatchmode(True)
                # Start making the format specific copy.
                for key in sizesList.keys():
                    w = sizesList[key]
                    # copy over data
                    projection = page.clone()
                    projection.setBatchmode(True)
                    # Crop. Cropping per guide only happens if said guides have been found.
                    if w["Crop"] is True:
                        listHGuides = []
                        listHGuides = page.horizontalGuides()
                        listHGuides.sort()
                        for i in range(len(listHGuides) - 1, 0, -1):
                            if listHGuides[i] < 0 or listHGuides[i] > page.height():
                                listHGuides.pop(i)
                        listVGuides = page.verticalGuides()
                        listVGuides.sort()
                        for i in range(len(listVGuides) - 1, 0, -1):
                            if listVGuides[i] < 0 or listVGuides[i] > page.width():
                                listVGuides.pop(i)
                        if self.configDictionary["cropToGuides"] and len(listVGuides) > 1:
                            cropx = listVGuides[0]
                            cropw = listVGuides[-1] - cropx
                        else:
                            cropx = self.configDictionary["cropLeft"]
                            cropw = page.width() - self.configDictionary["cropRight"] - cropx
                        if self.configDictionary["cropToGuides"] and len(listHGuides) > 1:
                            cropy = listHGuides[0]
                            croph = listHGuides[-1] - cropy
                        else:
                            cropy = self.configDictionary["cropTop"]
                            croph = page.height() - self.configDictionary["cropBottom"] - cropy
                        projection.crop(cropx, cropy, cropw, croph)
                        projection.waitForDone()
                        qApp.processEvents()
                        # resize appropriately
                    else:
                        cropx = 0
                        cropy = 0
                    res = page.resolution()
                    listScales = [projection.width(), projection.height(), res, res]
                    projectionOldSize = [projection.width(), projection.height()]
                    sizesCalc = sizesCalculator()
                    listScales = sizesCalc.get_scale_from_resize_config(config=w, listSizes=listScales)
                    projection.unlock()
                    projection.scaleImage(listScales[0], listScales[1], listScales[2], listScales[3], "bicubic")
                    projection.waitForDone()
                    qApp.processEvents()
                    # png, gif and other webformats should probably be in 8bit srgb at maximum.
                    if key != "TIFF":
                        if (projection.colorModel() != "RGBA" and projection.colorModel() != "GRAYA") or projection.colorDepth() != "U8":
                            projection.setColorSpace("RGBA", "U8", "sRGB built-in")
                    else:
                        # Tiff on the other hand can handle all the colormodels, but can only handle integer bit depths.
                        # Tiff is intended for print output, and 16 bit integer will be sufficient.
                        if projection.colorDepth() != "U8" or projection.colorDepth() != "U16":
                            projection.setColorSpace(page.colorModel(), "U16", page.colorProfile())
                    # save
                    # Make sure the folder name for this export exists. It'll allow us to keep the
                    # export folders nice and clean.
                    folderName = str(key + "-" + w["FileType"])
                    if Path(exportPath / folderName).exists() is False:
                        Path.mkdir(exportPath / folderName)
                    # Get a nice and descriptive fle name.
                    fn = str(Path(exportPath / folderName) / str("page_" + format(p, "03d") + "_" + str(listScales[0]) + "x" + str(listScales[1]) + "." + w["FileType"]))
                    # Finally save and add the page to a list of pages. This will make it easy for the packaging function to
                    # find the pages and store them.
                    projection.exportImage(fn, InfoObject())
                    projection.waitForDone()
                    qApp.processEvents()
                    if key == "CBZ":
                        transform = {}
                        transform["offsetX"] = cropx
                        transform["offsetY"] = cropy
                        transform["resDiff"] = page.resolution() / 72
                        transform["scaleWidth"] = projection.width() / projectionOldSize[0]
                        transform["scaleHeight"] = projection.height() / projectionOldSize[1]
                        pageData["transform"] = transform
                    self.pagesLocationList[key].append(fn)
                    projection.close()
                self.acbfPageData.append(pageData)
                page.close()
            progress.setValue(len(pagesList))
            Application.setBatchmode(batchsave)
            # TODO: Check what or whether memory leaks are still caused and otherwise remove the entry below.
            print("CPMT: Export has finished. If there are memory leaks, they are caused by file layers.")
            return True
        print("CPMT: Export not happening because there aren't any pages.")
        return False

    """
    Function to get the panel and text data.
    """

    def getPanelsAndText(self, node, list):
        textLayersToSearch = ["text"]
        panelLayersToSearch = ["panels"]
        if "textLayerNames" in self.configDictionary.keys():
            textLayersToSearch = self.configDictionary["textLayerNames"]
        if "panelLayerNames" in self.configDictionary.keys():
            panelLayersToSearch = self.configDictionary["panelLayerNames"]
        if node.type() == "vectorlayer":
            for name in panelLayersToSearch:
                if str(name).lower() in str(node.name()).lower():
                    for shape in node.shapes():
                        if (shape.type() == "groupshape"):
                            self.getPanelsAndTextVector(shape, list)
                        else:
                            self.handleShapeDescription(shape, list)
            for name in textLayersToSearch:
                if str(name).lower() in str(node.name()).lower():
                    for shape in node.shapes():
                        if (shape.type() == "groupshape"):
                            self.getPanelsAndTextVector(shape, list, True)
                        else:
                            self.handleShapeDescription(shape, list, True)
        else:
            if node.childNodes():
                for child in node.childNodes():
                    self.getPanelsAndText(node=child, list=list)
    """
    Function to get the panel and text data from a group shape
    """

    def getPanelsAndTextVector(self, group, list, textOnly=False):
        for shape in group.shapes():
            if (shape.type() == "groupshape"):
                self.getPanelsAndTextVector(shape, list, textOnly)
            else:
                self.handleShapeDescription(shape, list, textOnly)
    """
    Function to get text and panels in a format that acbf will accept
    TODO: move this to a new file.
    """

    def handleShapeDescription(self, shape, list, textOnly=False):
        if (shape.type() != "KoSvgTextShapeID" and textOnly is True):
            return
        shapeDesc = {}
        shapeDesc["name"] = shape.name()
        rect = shape.boundingBox()
        listOfPoints = [rect.topLeft(), rect.topRight(), rect.bottomRight(), rect.bottomLeft()]
        shapeDoc = minidom.parseString(shape.toSvg())
        docElem = shapeDoc.documentElement
        svgRegExp = re.compile('[MLCSQHVATmlzcqshva]\d+\.?\d* \d+\.?\d*')
        transform = docElem.getAttribute("transform")
        coord = []
        adjust = QTransform()
        # TODO: If we get global transform api, use that instead of parsing manually.
        if "translate" in transform:
            transform = transform.replace('translate(', '')
            for c in transform[:-1].split(" "):
                coord.append(float(c))
            if len(coord) < 2:
                coord.append(coord[0])
            adjust = QTransform(1, 0, 0, 1, coord[0], coord[1])
        if "matrix" in transform:
            transform = transform.replace('matrix(', '')
            for c in transform[:-1].split(" "):
                coord.append(float(c))
            adjust = QTransform(coord[0], coord[1], coord[2], coord[3], coord[4], coord[5])
        path = QPainterPath()
        if docElem.localName == "path":
            dVal = docElem.getAttribute("d")
            listOfSvgStrings = [" "]
            listOfSvgStrings = svgRegExp.findall(dVal)
            if listOfSvgStrings:
                listOfPoints = []
                for l in listOfSvgStrings:
                    line = l[1:]
                    coordinates = line.split(" ")
                    if len(coordinates) < 2:
                        coordinates.append(coordinates[0])
                    x = float(coordinates[-2])
                    y = float(coordinates[-1])
                    offset = QPointF()
                    if l.islower():
                        offset = listOfPoints[0]
                    if l.lower().startswith("m"):
                        path.moveTo(QPointF(x, y) + offset)
                    elif l.lower().startswith("h"):
                        y = listOfPoints[-1].y()
                        path.lineTo(QPointF(x, y) + offset)
                    elif l.lower().startswith("v"):
                        x = listOfPoints[-1].x()
                        path.lineTo(QPointF(x, y) + offset)
                    else:
                        path.lineTo(QPointF(x, y) + offset)
                path.setFillRule(Qt.WindingFill)
                for polygon in path.simplified().toSubpathPolygons(adjust):
                    for point in polygon:
                        listOfPoints.append(point)
        elif docElem.localName == "rect":
            listOfPoints = []
            if (docElem.hasAttribute("x")):
                x = float(docElem.getAttribute("x"))
            else:
                x = 0
            if (docElem.hasAttribute("y")):
                y = float(docElem.getAttribute("y"))
            else:
                y = 0
            w = float(docElem.getAttribute("width"))
            h = float(docElem.getAttribute("height"))
            path.addRect(QRectF(x, y, w, h))
            for point in path.toFillPolygon(adjust):
                listOfPoints.append(point)
        elif docElem.localName == "ellipse":
            listOfPoints = []
            if (docElem.hasAttribute("cx")):
                x = float(docElem.getAttribute("cx"))
            else:
                x = 0
            if (docElem.hasAttribute("cy")):
                y = float(docElem.getAttribute("cy"))
            else:
                y = 0
            ry = float(docElem.getAttribute("ry"))
            rx = float(docElem.getAttribute("rx"))
            path.addEllipse(QPointF(x, y), rx, ry)
            for point in path.toFillPolygon(adjust):
                listOfPoints.append(point)
        shapeDesc["boundingBox"] = listOfPoints
        if (shape.type() == "KoSvgTextShapeID" and textOnly is True):
            shapeDesc["text"] = shape.toSvg()
        list.append(shapeDesc)

    """
    Function to remove layers when they have the given labels.

    If not, but the node does have children, check those too.
    """

    def removeLayers(self, labels, node):
        if node.colorLabel() in labels:
            node.remove()
        else:
            if node.childNodes():
                for child in node.childNodes():
                    self.removeLayers(labels, node=child)

    """
    package cbz puts all the meta-data and relevant files into an zip file ending with ".cbz"
    """

    def package_cbz(self, exportPath):

        # Use the project name if there's no title to avoid sillyness with unnamed zipfiles.
        title = self.configDictionary["projectName"]
        if "title" in self.configDictionary.keys():
            title = self.configDictionary["title"]

        # Get the appropriate path.
        url = str(exportPath / str(title + ".cbz"))

        # Create a zip file.
        cbzArchive = zipfile.ZipFile(url, mode="w", compression=zipfile.ZIP_STORED)

        # Add all the meta data files.
        cbzArchive.write(self.acbfLocation, Path(self.acbfLocation).name)
        cbzArchive.write(self.cometLocation, Path(self.cometLocation).name)
        cbzArchive.write(self.comicRackInfo, Path(self.comicRackInfo).name)
        comic_book_info_json_dump = str()
        comic_book_info_json_dump = exporters.comic_book_info.writeJson(self.configDictionary)
        cbzArchive.comment = comic_book_info_json_dump.encode("utf-8")

        # Add the pages.
        if "CBZ" in self.pagesLocationList.keys():
            for page in self.pagesLocationList["CBZ"]:
                if (Path(page).exists()):
                    cbzArchive.write(page, Path(page).name)

        # Close the zip file when done.
        cbzArchive.close()
