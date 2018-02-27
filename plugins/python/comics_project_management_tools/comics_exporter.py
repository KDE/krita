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
import os
from pathlib import Path
import json
import zipfile
import xml.etree.ElementTree as ET
from xml.dom import minidom
import types
import shutil
import html
import re
from PyQt5.QtWidgets import QLabel, QProgressDialog, qApp  # For the progress dialog.
from PyQt5.QtCore import QElapsedTimer, QDate, QLocale, Qt, QRectF, QPointF, QByteArray, QBuffer
from PyQt5.QtGui import QImage, QTransform, QPainterPath
from krita import *

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
    comic_book_info_json_dump = str()
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
        self.comic_book_info_json_dump = str()

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
                export_success = self.export_to_acbf()

            # Export and package CBZ and Epub.
            if export_success:
                if "CBZ" in sizesList.keys():
                    export_success = self.export_to_cbz()
                    print("CPMT: Exported to CBZ", export_success)
                if "EPUB" in sizesList.keys():
                    export_success = self.export_to_epub()
                    print("CPMT: Exported to EPUB", export_success)
        else:
            print("CPMT: Nothing to export, url not set.")

        return export_success

    """
    This calls up all the functions necessary for making a acbf.
    """

    def export_to_acbf(self):
        self.write_acbf_meta_data()
        return True

    """
    This calls up all the functions necessary for making a cbz.
    """

    def export_to_cbz(self):
        export_success = self.write_comet_meta_data()
        export_success = self.write_comic_rack_info()
        export_success = self.write_comic_book_info_json()
        self.package_cbz()
        return export_success

    """
    Create an epub folder, finally, package to a epubzip.
    """

    def export_to_epub(self):
        path = Path(os.path.join(self.projectURL, self.configDictionary["exportLocation"]))
        exportPath = path / "EPUB-files"
        metaInf = exportPath / "META-INF"
        oebps = exportPath / "OEBPS"
        imagePath = oebps / "Images"
        stylesPath = oebps / "Styles"
        textPath = oebps / "Text"
        if exportPath.exists() is False:
            exportPath.mkdir()
            metaInf.mkdir()
            oebps.mkdir()
            imagePath.mkdir()
            stylesPath.mkdir()
            textPath.mkdir()

        mimetype = open(str(Path(exportPath / "mimetype")), mode="w")
        mimetype.write("application/epub+zip")
        mimetype.close()

        container = ET.ElementTree()
        cRoot = ET.Element("container")
        cRoot.set("version", "1.0")
        cRoot.set("xmlns", "urn:oasis:names:tc:opendocument:xmlns:container")
        container._setroot(cRoot)
        rootFiles = ET.Element("rootfiles")
        rootfile = ET.Element("rootfile")
        rootfile.set("full-path", "OEBPS/content.opf")
        rootfile.set("media-type", "application/oebps-package+xml")
        rootFiles.append(rootfile)
        cRoot.append(rootFiles)
        container.write(str(Path(metaInf / "container.xml")), encoding="utf-8", xml_declaration=True)

        # copyimages to images
        pagesList = []
        if "EPUB" in self.pagesLocationList.keys():
            coverNumber = self.configDictionary["pages"].index(self.configDictionary["cover"])
            for p in self.pagesLocationList["EPUB"]:
                if os.path.exists(p):
                    shutil.copy2(p, str(imagePath))
                    pagesList.append(str(Path(imagePath / os.path.basename(p))))
            if len(self.pagesLocationList["EPUB"]) >= coverNumber:
                coverpageurl = pagesList[coverNumber]
        else:
            print("CPMT: Couldn't find the location for the epub files.")
            return False

        # for each image, make an xml file
        htmlFiles = []
        for i in range(len(pagesList)):
            pageName = "Page" + str(i) + ".xhtml"
            doc = ET.ElementTree()
            html = ET.Element("html")
            doc._setroot(html)
            html.set("xmlns", "http://www.w3.org/1999/xhtml")
            html.set("xmlns:epub", "http://www.idpf.org/2007/ops")

            head = ET.Element("head")
            html.append(head)

            body = ET.Element("body")

            img = ET.Element("img")
            img.set("src", os.path.relpath(pagesList[i], str(textPath)))
            body.append(img)

            if pagesList[i] != coverpageurl:
                pagenumber = ET.Element("p")
                pagenumber.text = "Page " + str(i)
                body.append(pagenumber)
            html.append(body)

            filename = str(Path(textPath / pageName))
            doc.write(filename, encoding="utf-8", xml_declaration=True)
            if pagesList[i] == coverpageurl:
                coverpagehtml = os.path.relpath(filename, str(oebps))
            htmlFiles.append(filename)

        # opf file
        opfFile = ET.ElementTree()
        opfRoot = ET.Element("package")
        opfRoot.set("version", "3.0")
        opfRoot.set("unique-identifier", "BookId")
        opfRoot.set("xmlns", "http://www.idpf.org/2007/opf")
        opfFile._setroot(opfRoot)

        # metadata
        opfMeta = ET.Element("metadata")
        opfMeta.set("xmlns:dc", "http://purl.org/dc/elements/1.1/")

        if "language" in self.configDictionary.keys():
            bookLang = ET.Element("dc:language")
            bookLang.text = self.configDictionary["language"]
            opfMeta.append(bookLang)
        bookTitle = ET.Element("dc:title")
        if "title" in self.configDictionary.keys():
            bookTitle.text = str(self.configDictionary["title"])
        else:
            bookTitle.text = "Comic with no Name"
        opfMeta.append(bookTitle)
        if "authorList" in self.configDictionary.keys():
            for authorE in range(len(self.configDictionary["authorList"])):
                authorDict = self.configDictionary["authorList"][authorE]
                authorType = "dc:creator"
                if "role" in authorDict.keys():
                    if str(authorDict["role"]).lower() in ["editor", "assistant editor", "proofreader", "beta"]:
                        authorType = "dc:contributor"
                author = ET.Element(authorType)
                authorName = []
                if "last-name" in authorDict.keys():
                    authorName.append(authorDict["last-name"])
                if "first-name" in authorDict.keys():
                    authorName.append(authorDict["first-name"])
                if "initials" in authorDict.keys():
                    authorName.append(authorDict["initials"])
                if "nickname" in authorDict.keys():
                    authorName.append("(" + authorDict["nickname"] + ")")
                author.text = ", ".join(authorName)
                opfMeta.append(author)
                if "role" in authorDict.keys():
                    author.set("id", "cre" + str(authorE))
                    role = ET.Element("meta")
                    role.set("refines", "cre" + str(authorE))
                    role.set("scheme", "marc:relators")
                    role.set("property", "role")
                    role.text = str(authorDict["role"])
                    opfMeta.append(role)

        if "publishingDate" in self.configDictionary.keys():
            date = ET.Element("dc:date")
            date.text = self.configDictionary["publishingDate"]
            opfMeta.append(date)
        description = ET.Element("dc:description")
        if "summary" in self.configDictionary.keys():
            description.text = self.configDictionary["summary"]
        else:
            description.text = "There was no summary upon generation of this file."
        opfMeta.append(description)

        type = ET.Element("dc:type")
        type.text = "Comic"
        opfMeta.append(type)
        if "publisherName" in self.configDictionary.keys():
            publisher = ET.Element("dc:publisher")
            publisher.text = self.configDictionary["publisherName"]
            opfMeta.append(publisher)
        if "isbn-number" in self.configDictionary.keys():
            publishISBN = ET.Element("dc:identifier")
            publishISBN.text = str("urn:isbn:") + self.configDictionary["isbn-number"]
            opfMeta.append(publishISBN)
        if "license" in self.configDictionary.keys():
            rights = ET.Element("dc:rights")
            rights.text = self.configDictionary["license"]
            opfMeta.append(rights)

        if "genre" in self.configDictionary.keys():
            for g in self.configDictionary["genre"]:
                subject = ET.Element("dc:subject")
                subject.text = g
                opfMeta.append(subject)
        if "characters" in self.configDictionary.keys():
            for name in self.configDictionary["characters"]:
                char = ET.Element("dc:subject")
                char.text = name
                opfMeta.append(char)
        if "format" in self.configDictionary.keys():
            for format in self.configDictionary["format"]:
                f = ET.Element("dc:subject")
                f.text = format
                opfMeta.append(f)
        if "otherKeywords" in self.configDictionary.keys():
            for key in self.configDictionary["otherKeywords"]:
                word = ET.Element("dc:subject")
                word.text = key
                opfMeta.append(word)

        opfRoot.append(opfMeta)

        opfManifest = ET.Element("manifest")
        toc = ET.Element("item")
        toc.set("id", "ncx")
        toc.set("href", "toc.ncx")
        toc.set("media-type", "application/x-dtbncx+xml")
        opfManifest.append(toc)
        for p in htmlFiles:
            item = ET.Element("item")
            item.set("id", os.path.basename(p))
            item.set("href", os.path.relpath(p, str(oebps)))
            item.set("media-type", "application/xhtml+xml")
            opfManifest.append(item)
        for p in pagesList:
            item = ET.Element("item")
            item.set("id", os.path.basename(p))
            item.set("href", os.path.relpath(p, str(oebps)))
            item.set("media-type", "image/png")
            if os.path.basename(p) == os.path.basename(coverpageurl):
                item.set("properties", "cover-image")
            opfManifest.append(item)

        opfRoot.append(opfManifest)

        opfSpine = ET.Element("spine")
        opfSpine.set("toc", "ncx")
        for p in htmlFiles:
            item = ET.Element("itemref")
            item.set("idref", os.path.basename(p))
            opfSpine.append(item)
        opfRoot.append(opfSpine)

        opfGuide = ET.Element("guide")
        if coverpagehtml is not None and coverpagehtml.isspace() is False and len(coverpagehtml) > 0:
            item = ET.Element("reference")
            item.set("type", "cover")
            item.set("title", "Cover")
            item.set("href", coverpagehtml)
        opfRoot.append(opfGuide)

        opfFile.write(str(Path(oebps / "content.opf")), encoding="utf-8", xml_declaration=True)
        # toc
        tocDoc = ET.ElementTree()
        ncx = ET.Element("ncx")
        ncx.set("version", "2005-1")
        ncx.set("xmlns", "http://www.daisy.org/z3986/2005/ncx/")
        tocDoc._setroot(ncx)

        tocHead = ET.Element("head")
        metaID = ET.Element("meta")
        metaID.set("content", "ID_UNKNOWN")
        metaID.set("name", "dtb:uid")
        tocHead.append(metaID)
        metaDepth = ET.Element("meta")
        metaDepth.set("content", str(0))
        metaDepth.set("name", "dtb:depth")
        tocHead.append(metaDepth)
        metaTotal = ET.Element("meta")
        metaTotal.set("content", str(0))
        metaTotal.set("name", "dtb:totalPageCount")
        tocHead.append(metaTotal)
        metaMax = ET.Element("meta")
        metaMax.set("content", str(0))
        metaMax.set("name", "dtb:maxPageNumber")
        tocHead.append(metaDepth)
        ncx.append(tocHead)

        docTitle = ET.Element("docTitle")
        text = ET.Element("text")
        if "title" in self.configDictionary.keys():
            text.text = str(self.configDictionary["title"])
        else:
            text.text = "Comic with no Name"
        docTitle.append(text)
        ncx.append(docTitle)

        navmap = ET.Element("navMap")
        navPoint = ET.Element("navPoint")
        navPoint.set("id", "navPoint-1")
        navPoint.set("playOrder", "1")
        navLabel = ET.Element("navLabel")
        navLabelText = ET.Element("text")
        navLabelText.text = "Start"
        navLabel.append(navLabelText)
        navContent = ET.Element("content")
        navContent.set("src", os.path.relpath(htmlFiles[0], str(oebps)))
        navPoint.append(navLabel)
        navPoint.append(navContent)
        navmap.append(navPoint)
        ncx.append(navmap)

        tocDoc.write(str(Path(oebps / "toc.ncx")), encoding="utf-8", xml_declaration=True)

        self.package_epub()
        return True

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
                url = os.path.join(self.projectURL, pagesList[p])
                page = Application.openDocument(url)
                page.waitForDone()

                # remove layers and flatten.
                labelList = self.configDictionary["labelsToRemove"]
                panelsAndText = []

                # These three lines are what is causing the page not to close.
                root = page.rootNode()
                self.removeLayers(labelList, root)
                self.getPanelsAndText(root, panelsAndText)
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
                    fn = os.path.join(str(Path(exportPath / folderName)), "page_" + format(p, "03d") + "_" + str(listScales[0]) + "x" + str(listScales[1]) + "." + w["FileType"])
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
            textRoot = ET.fromstring(shape.toSvg())
            paragraph = ET.Element("p")
            if (len(textRoot) > 0):
                self.parseTextChildren(textRoot, paragraph)
            shapeDesc["text"] = ET.tostring(paragraph, "unicode")
        list.append(shapeDesc)

    """
    Function to parse svg text to acbf ready text
    TODO: Move to a new file.
    """

    def parseTextChildren(self, elRead, elWrite):

        if elRead.text is not None:
            if len(elWrite) > 0:
                if (elWrite[-1].tail is None):
                    elWrite[-1].tail = str()
                elWrite[-1].tail = " ".join([elWrite[-1].tail, elRead.text])
            else:
                if elWrite.text is None:
                    elWrite.text = elRead.text
                else:
                    elWrite.text = " ".join([elWrite.text, elRead.text])

        for childNode in elRead:
            fontWeight = childNode.get("font-weight")
            fontItalic = childNode.get("font-style")
            fontStrikeThrough = childNode.get("text-decoration")
            fontBaseLine = childNode.get("baseline-shift")
            newElementMade = False
            if fontItalic is not None:
                if (fontItalic == "italic"):
                    newElement = ET.Element("Emphasis")
                    newElementMade = True
            elif fontWeight is not None:
                if (fontWeight == "bold" or int(fontWeight) > 400):
                    newElement = ET.Element("Strong")
                    newElementMade = True
            elif fontStrikeThrough is not None:
                if (fontStrikeThrough == "line-through"):
                    newElement = ET.Element("Strikethrough")
                    newElementMade = True
            elif fontBaseLine is not None:
                if (fontBaseLine == "super"):
                    newElement = ET.Element("Sup")
                    newElementMade = True
                elif (fontBaseLine == "sub"):
                    newElement = ET.Element("Sub")
                    newElementMade = True

            if newElementMade is True:
                if (len(childNode) > 0):
                    self.parseTextChildren(childNode, newElement)
                else:
                    newElement.text = childNode.text
                elWrite.append(newElement)
            else:
                if (len(childNode) > 0):
                    self.parseTextChildren(childNode, elWrite)
                else:
                    if len(elWrite) > 0:
                        if (elWrite[-1].tail is None):
                            elWrite[-1].tail = str()
                        elWrite[-1].tail = " ".join([elWrite[-1].tail, childNode.text])
                    else:
                        if elWrite.text is None:
                            elWrite.text = str()
                        elWrite.text = " ".join([elWrite.text, childNode.text])

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
    Write the Advanced Comic Book Data xml file.
    
    http://acbf.wikia.com/wiki/ACBF_Specifications
    
    """

    def write_acbf_meta_data(self):
        acbfGenreList = ["science_fiction", "fantasy", "adventure", "horror", "mystery", "crime", "military", "real_life", "superhero", "humor", "western", "manga", "politics", "caricature", "sports", "history", "biography", "education", "computer", "religion", "romance", "children", "non-fiction", "adult", "alternative", "other", "artbook"]
        acbfAuthorRolesList = ["Writer", "Adapter", "Artist", "Penciller", "Inker", "Colorist", "Letterer", "Cover Artist", "Photographer", "Editor", "Assistant Editor", "Translator", "Other", "Designer"]
        title = self.configDictionary["projectName"]
        if "title" in self.configDictionary.keys():
            title = self.configDictionary["title"]
        location = str(os.path.join(self.projectURL, self.configDictionary["exportLocation"], "metadata", title + ".acbf"))
        document = minidom.Document()
        root = document.createElement("ACBF")
        root.setAttribute("xmlns", "http://www.fictionbook-lib.org/xml/acbf/1.0")
        document.appendChild(root)

        meta = document.createElement("meta-data")

        bookInfo = document.createElement("book-info")
        if "authorList" in self.configDictionary.keys():
            for authorE in range(len(self.configDictionary["authorList"])):
                author = document.createElement("author")
                authorDict = self.configDictionary["authorList"][authorE]
                if "first-name" in authorDict.keys():
                    authorN = document.createElement("first-name")
                    authorN.appendChild(document.createTextNode(str(authorDict["first-name"])))
                    author.appendChild(authorN)
                if "last-name" in authorDict.keys():
                    authorN = document.createElement("last-name")
                    authorN.appendChild(document.createTextNode(str(authorDict["last-name"])))
                    author.appendChild(authorN)
                if "initials" in authorDict.keys():
                    authorN = document.createElement("middle-name")
                    authorN.appendChild(document.createTextNode(str(authorDict["initials"])))
                    author.appendChild(authorN)
                if "nickname" in authorDict.keys():
                    authorN = document.createElement("nickname")
                    authorN.appendChild(document.createTextNode(str(authorDict["nickname"])))
                    author.appendChild(authorN)
                if "homepage" in authorDict.keys():
                    authorN = document.createElement("home-page")
                    authorN.appendChild(document.createTextNode(str(authorDict["homepage"])))
                    author.appendChild(authorN)
                if "email" in authorDict.keys():
                    authorN = document.createElement("email")
                    authorN.appendChild(document.createTextNode(str(authorDict["email"])))
                    author.appendChild(authorN)
                if "role" in authorDict.keys():
                    if str(authorDict["role"]).title() in acbfAuthorRolesList:
                        author.setAttribute("activity", str(authorDict["role"]))
                if "language" in authorDict.keys():
                    author.setAttribute("lang", str(authorDict["language"]))
                bookInfo.appendChild(author)
        bookTitle = document.createElement("book-title")
        if "title" in self.configDictionary.keys():
            bookTitle.appendChild(document.createTextNode(str(self.configDictionary["title"])))
        else:
            bookTitle.appendChild(document.createTextNode(str("Comic with no Name")))
        bookInfo.appendChild(bookTitle)
        extraGenres = []

        if "genre" in self.configDictionary.keys():
            for genre in self.configDictionary["genre"]:
                genreModified = str(genre).lower()
                genreModified.replace(" ", "_")
                if genreModified in acbfGenreList:
                    bookGenre = document.createElement("genre")
                    bookGenre.appendChild(document.createTextNode(str(genreModified)))
                    bookInfo.appendChild(bookGenre)
                else:
                    extraGenres.appendChild(genre)
        annotation = document.createElement("annotation")
        if "summary" in self.configDictionary.keys():
            paragraphList = str(self.configDictionary["summary"]).split("\n")
            for para in paragraphList:
                p = document.createElement("p")
                p.appendChild(document.createTextNode(str(para)))
                annotation.appendChild(p)
        else:
            p = document.createElement("p")
            p.appendChild(document.createTextNode(str("There was no summary upon generation of this file.")))
            annotation.appendChild(p)
        bookInfo.appendChild(annotation)

        if "characters" in self.configDictionary.keys():
            character = document.createElement("characters")
            for name in self.configDictionary["characters"]:
                char = document.createElement("name")
                char.appendChild(document.createTextNode(str(name)))
                character.appendChild(char)
            bookInfo.appendChild(character)

        keywords = document.createElement("keywords")
        stringKeywordsList = []
        for key in extraGenres:
            stringKeywordsList.append(str(key))
        if "otherKeywords" in self.configDictionary.keys():
            for key in self.configDictionary["otherKeywords"]:
                stringKeywordsList.append(str(key))
        if "format" in self.configDictionary.keys():
            for key in self.configDictionary["format"]:
                stringKeywordsList.append(str(key))
        keywords.appendChild(document.createTextNode(", ".join(stringKeywordsList)))
        bookInfo.appendChild(keywords)

        coverpageurl = ""
        coverpage = document.createElement("coverpage")
        if "pages" in self.configDictionary.keys():
            if "cover" in self.configDictionary.keys():
                pageList = []
                pageList = self.configDictionary["pages"]
                coverNumber = max([pageList.index(self.configDictionary["cover"]), 0])
                image = document.createElement("image")
                if len(self.pagesLocationList["CBZ"]) >= coverNumber:
                    coverpageurl = self.pagesLocationList["CBZ"][coverNumber]
                    image.setAttribute("href", os.path.basename(coverpageurl))
                coverpage.appendChild(image)
        bookInfo.appendChild(coverpage)

        if "language" in self.configDictionary.keys():
            language = document.createElement("languages")
            textlayer = document.createElement("text-layer")
            textlayer.setAttribute("lang", self.configDictionary["language"])
            textlayer.setAttribute("show", "False")
            textlayerNative = document.createElement("text-layer")
            textlayerNative.setAttribute("lang", self.configDictionary["language"])
            textlayerNative.setAttribute("show", "True")
            language.appendChild(textlayer)
            language.appendChild(textlayerNative)
            bookInfo.appendChild(language)

            bookTitle.setAttribute("lang", self.configDictionary["language"])
            annotation.setAttribute("lang", self.configDictionary["language"])
            keywords.setAttribute("lang", self.configDictionary["language"])
        #database = document.createElement("databaseref")
        # bookInfo.appendChild(database)

        if "seriesName" in self.configDictionary.keys():
            sequence = document.createElement("sequence")
            sequence.setAttribute("title", self.configDictionary["seriesName"])
            if "seriesVolume" in self.configDictionary.keys():
                sequence.setAttribute("volume", str(self.configDictionary["seriesVolume"]))
            if "seriesNumber" in self.configDictionary.keys():
                sequence.appendChild(document.createTextNode(str(self.configDictionary["seriesNumber"])))
            else:
                sequence.appendChild(document.createTextNode(str(0)))
            bookInfo.appendChild(sequence)
        contentrating = document.createElement("content-rating")

        if "rating" in self.configDictionary.keys():
            contentrating.appendChild(document.createTextNode(str(self.configDictionary["rating"])))
        else:
            contentrating.appendChild(document.createTextNode(str("Unrated.")))
        if "ratingSystem" in self.configDictionary.keys():
            contentrating.setAttribute("type", self.configDictionary["ratingSystem"])
        bookInfo.appendChild(contentrating)

        if "readingDirection" in self.configDictionary.keys():
            readingDirection = document.createElement("reading-direction")
            if self.configDictionary["readingDirection"] is "rightToLeft":
                readingDirection.appendChild(document.createTextNode(str("RTL")))
            else:
                readingDirection.appendChild(document.createTextNode(str("LTR")))
            bookInfo.appendChild(readingDirection)
        meta.appendChild(bookInfo)

        publisherInfo = document.createElement("publish-info")
        if "publisherName" in self.configDictionary.keys():
            publisherName = document.createElement("publisher")
            publisherName.appendChild(document.createTextNode(str(self.configDictionary["publisherName"])))
            publisherInfo.appendChild(publisherName)
        if "publishingDate" in self.configDictionary.keys():
            publishingDate = document.createElement("publish-date")
            publishingDate.setAttribute("value", self.configDictionary["publishingDate"])
            publishingDate.appendChild(document.createTextNode(QDate.fromString(self.configDictionary["publishingDate"], Qt.ISODate).toString(Qt.SystemLocaleLongDate)))
            publisherInfo.appendChild(publishingDate)
        if "publisherCity" in self.configDictionary.keys():
            publishCity = document.createElement("city")
            publishCity.appendChild(document.createTextNode(str(self.configDictionary["publisherCity"])))
            publisherInfo.appendChild(publishCity)
        if "isbn-number" in self.configDictionary.keys():
            publishISBN = document.createElement("isbn")
            publishISBN.appendChild(document.createTextNode(str(self.configDictionary["isbn-number"])))
            publisherInfo.appendChild(publishISBN)
        if "license" in self.configDictionary.keys():
            license = self.configDictionary["license"]
            if license.isspace() is False and len(license) > 0:
                publishLicense = document.createElement("license")
                publishLicense.appendChild(document.createTextNode(str(self.configDictionary["license"])))
                publisherInfo.appendChild(publishLicense)

        meta.appendChild(publisherInfo)

        documentInfo = document.createElement("document-info")
        # TODO: ACBF apparantly uses first/middle/last/nick/email/homepage for the document auhtor too...
        #      The following code compensates for me not understanding this initially. This still needs
        #      adjustments in the gui.
        if "acbfAuthor" in self.configDictionary.keys():
            if isinstance(self.configDictionary["acbfAuthor"], list):
                for e in self.configDictionary["acbfAuthor"]:
                    acbfAuthor = document.createElement("author")
                    authorDict = self.configDictionary["acbfAuthor"][e]
                    if "first-name" in authorDict.keys():
                        authorN = document.createElement("first-name")
                        authorN.appendChild(document.createTextNode(str(authorDict["first-name"])))
                        acbfAuthor.appendChild(authorN)
                    if "last-name" in authorDict.keys():
                        authorN = document.createElement("last-name")
                        authorN.appendChild(document.createTextNode(str(authorDict["last-name"])))
                        acbfAuthor.appendChild(authorN)
                    if "initials" in authorDict.keys():
                        authorN = document.createElement("middle-name")
                        authorN.appendChild(document.createTextNode(str(authorDict["initials"])))
                        acbfAuthor.appendChild(authorN)
                    if "nickname" in authorDict.keys():
                        authorN = document.createElement("nickname")
                        authorN.appendChild(document.createTextNode(str(authorDict["nickname"])))
                        acbfAuthor.appendChild(authorN)
                    if "homepage" in authorDict.keys():
                        authorN = document.createElement("homepage")
                        authorN.appendChild(document.createTextNode(str(authorDict["home-page"])))
                        acbfAuthor.appendChild(authorN)
                    if "email" in authorDict.keys():
                        authorN = document.createElement("email")
                        authorN.appendChild(document.createTextNode(str(authorDict["email"])))
                        acbfAuthor.appendChild(authorN)
                    if "language" in authorDict.keys():
                        acbfAuthor.setAttribute("lang", str(authorDict["language"]))
                    documentInfo.appendChild(acbfAuthor)
            else:
                acbfAuthor = document.createElement("author")
                acbfAuthorNick = document.createElement("nickname")
                acbfAuthorNick.appendChild(document.createTextNode(str(self.configDictionary["acbfAuthor"])))
                acbfAuthor.appendChild(acbfAuthorNick)
                documentInfo.appendChild(acbfAuthor)
        else:
            acbfAuthor = document.createElement("author")
            acbfAuthorNick = document.createElement("nickname")
            acbfAuthorNick.appendChild(document.createTextNode(str("Anon")))
            acbfAuthor.appendChild(acbfAuthorNick)
            documentInfo.appendChild(acbfAuthor)

        acbfDate = document.createElement("creation-date")
        now = QDate.currentDate()
        acbfDate.setAttribute("value", now.toString(Qt.ISODate))
        acbfDate.appendChild(document.createTextNode(str(now.toString(Qt.SystemLocaleLongDate))))
        documentInfo.appendChild(acbfDate)

        if "acbfSource" in self.configDictionary.keys():
            acbfSource = document.createElement("source")
            acbfSourceP = document.createElement("p")
            acbfSourceP.appendChild(document.createTextNode(str(self.configDictionary["acbfSource"])))
            acbfSource.appendChild(acbfSourceP)
            documentInfo.appendChild(acbfSource)

        if "acbfID" in self.configDictionary.keys():
            acbfID = document.createElement("id")
            acbfID.appendChild(document.createTextNode(str(self.configDictionary["acbfID"])))
            documentInfo.appendChild(acbfID)

        if "acbfVersion" in self.configDictionary.keys():
            acbfVersion = document.createElement("version")
            acbfVersion.appendChild(document.createTextNode(str(self.configDictionary["acbfVersion"])))
            documentInfo.appendChild(acbfVersion)

        if "acbfHistory" in self.configDictionary.keys():
            acbfHistory = document.createElement("history")
            for h in self.configDictionary["acbfHistory"]:
                p = document.createElement("p")
                p.appendChild(document.createTextNode(str(h)))
                acbfHistory.appendChild(p)
            documentInfo.appendChild(acbfHistory)
        meta.appendChild(documentInfo)

        root.appendChild(meta)

        body = document.createElement("body")

        for p in range(0, len(self.pagesLocationList["CBZ"])):
            page = self.pagesLocationList["CBZ"][p]
            language = "en"
            if "language" in self.configDictionary.keys():
                language = self.configDictionary["language"]
            textLayer = document.createElement("text-layer")
            textLayer.setAttribute("lang", language)
            pageData = self.acbfPageData[p]
            transform = pageData["transform"]
            frameList = []
            for v in pageData["vector"]:
                boundingBoxText = []
                for point in v["boundingBox"]:
                    offset = QPointF(transform["offsetX"], transform["offsetY"])
                    pixelPoint = QPointF(point.x() * transform["resDiff"], point.y() * transform["resDiff"])
                    newPoint = pixelPoint - offset
                    x = int(newPoint.x() * transform["scaleWidth"])
                    y = int(newPoint.y() * transform["scaleHeight"])
                    pointText = str(x) + "," + str(y)
                    boundingBoxText.append(pointText)

                if "text" in v.keys():
                    textArea = document.createElement("text-area")
                    textArea.setAttribute("points", " ".join(boundingBoxText))
                    # TODO: Rotate will require proper global transform api as transform info is not written intotext.                        #textArea.setAttribute("text-rotation", str(v["rotate"]))
                    paragraph = minidom.parseString(v["text"])
                    textArea.appendChild(paragraph.documentElement)
                    textLayer.appendChild(textArea)
                else:
                    f = {}
                    f["points"] = " ".join(boundingBoxText)
                    frameList.append(f)
            if page is not coverpageurl:
                pg = document.createElement("page")
                image = document.createElement("image")
                image.setAttribute("href", os.path.basename(page))
                pg.appendChild(image)
                if "acbf_title" in pageData["keys"]:
                    title = document.createElement("title")
                    title.setAttribute("lang", language)
                    title.appendChild(document.createTextNode(str(pageData["title"])))
                    pg.appendChild(title)
                if "acbf_none" in pageData["keys"]:
                    pg.setAttribute("transition", "none")
                if "acbf_blend" in pageData["keys"]:
                    pg.setAttribute("transition", "blend")
                if "acbf_fade" in pageData["keys"]:
                    pg.setAttribute("transition", "fade")
                if "acbf_horizontal" in pageData["keys"]:
                    pg.setAttribute("transition", "scroll_right")
                if "acbf_vertical" in pageData["keys"]:
                    pg.setAttribute("transition", "scroll_down")
                for f in frameList:
                    frame = document.createElement("frame")
                    frame.setAttribute("points", f["points"])
                    pg.appendChild(frame)
                pg.appendChild(textLayer)
                body.appendChild(pg)
            else:
                for f in frameList:
                    frame = document.createElement("frame")
                    frame.setAttribute("points", f["points"])
                    coverpage.appendChild(frame)
                coverpage.appendChild(textLayer)

        root.appendChild(body)

        f = open(location, 'w', newline="", encoding="utf-8")
        f.write(document.toprettyxml(indent="  "))
        f.close()
        self.acbfLocation = location
        success = True
        success = self.createStandAloneACBF(document)
        return success

    def createStandAloneACBF(self, document):
        title = self.configDictionary["projectName"]
        if "title" in self.configDictionary.keys():
            title = self.configDictionary["title"]
        location = str(os.path.join(self.projectURL, self.configDictionary["exportLocation"], title + ".acbf"))
        root = document.getElementsByTagName("ACBF")[0]
        meta = root.getElementsByTagName("meta-data")[0]
        bookInfo = meta.getElementsByTagName("book-info")[0]
        cover = bookInfo.getElementsByTagName("coverpage")[0]

        body = root.getElementsByTagName("body")[0]
        pages = body.getElementsByTagName("page")
        if (cover):
            pages.append(cover)

        data = document.createElement("data")

        # Covert pages to base64 strings.
        for i in range(0, len(pages)):
            image = pages[i].getElementsByTagName("image")[0]
            href = image.getAttribute("href")
            for p in self.pagesLocationList["CBZ"]:
                if href in p:
                    binary = document.createElement("binary")
                    binary.setAttribute("id", href)
                    imageFile = QImage()
                    imageFile.load(p)
                    imageData = QByteArray()
                    buffer = QBuffer(imageData)
                    imageFile.save(buffer, "PNG")
                    # For now always embed as png.
                    contentType = "image/png"
                    binary.setAttribute("content-type", contentType)
                    binary.appendChild(document.createTextNode(str(bytearray(imageData.toBase64()).decode("ascii"))))

                    image.setAttribute("href", "#" + href)
                    data.appendChild(binary)

        root.appendChild(data)

        f = open(location, 'w', newline="", encoding="utf-8")
        f.write(document.toprettyxml(indent="  "))
        f.close()
        return True

    """
    Write a CoMet xml file to url
    """

    def write_comet_meta_data(self):
        title = self.configDictionary["projectName"]
        if "title" in self.configDictionary.keys():
            title = self.configDictionary["title"]
        location = str(os.path.join(self.projectURL, self.configDictionary["exportLocation"], "metadata", title + " CoMet.xml"))
        document = minidom.Document()
        root = document.createElement("comet")
        root.setAttribute("xmlns:comet", "http://www.denvog.com/comet/")
        root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")
        root.setAttribute("xsi:schemaLocation", "http://www.denvog.com http://www.denvog.com/comet/comet.xsd")
        document.appendChild(root)

        title = document.createElement("title")
        if "title" in self.configDictionary.keys():
            title.appendChild(document.createTextNode(str(self.configDictionary["title"])))
        else:
            title.appendChild(document.createTextNode(str("Untitled Comic")))
        root.appendChild(title)
        description = document.createElement("description")
        if "summary" in self.configDictionary.keys():
            description.appendChild(document.createTextNode(str(self.configDictionary["summary"])))
        else:
            description.appendChild(document.createTextNode(str("There was no summary upon generation of this file.")))
        root.appendChild(description)
        if "seriesName" in self.configDictionary.keys():
            series = document.createElement("series")
            series.appendChild(document.createTextNode(str(self.configDictionary["seriesName"])))
            root.appendChild(series)
            if "seriesNumber" in self.configDictionary.keys():
                issue = document.createElement("issue")
                issue.appendChild(document.createTextNode(str(self.configDictionary["seriesNumber"])))
                root.appendChild(issue)
            if "seriesVolume" in self.configDictionary.keys():
                volume = document.createElement("volume")
                volume.appendChild(document.createTextNode(str(self.configDictionary["seriesVolume"])))
                root.appendChild(volume)

        if "publisherName" in self.configDictionary.keys():
            publisher = document.createElement("publisher")
            publisher.appendChild(document.createTextNode(str(self.configDictionary["publisherName"])))
            root.appendChild(publisher)

        if "publishingDate" in self.configDictionary.keys():
            date = document.createElement("date")
            date.appendChild(document.createTextNode(str(self.configDictionary["publishingDate"])))
            root.appendChild(date)

        if "genre" in self.configDictionary.keys():
            for genreE in self.configDictionary["genre"]:
                genre = document.createElement("genre")
                genre.appendChild(document.createTextNode(str(genreE)))
                root.appendChild(genre)

        if "characters" in self.configDictionary.keys():
            for char in self.configDictionary["characters"]:
                character = document.createElement("character")
                character.appendChild(document.createTextNode(str(char)))
                root.appendChild(character)

        if "format" in self.configDictionary.keys():
            format = document.createElement("format")
            format.appendChild(document.createTextNode(str(",".join(self.configDictionary["format"]))))
            root.appendChild(format)

        if "language" in self.configDictionary.keys():
            language = document.createElement("language")
            language.appendChild(document.createTextNode(str(self.configDictionary["language"])))
            root.appendChild(language)
        if "rating" in self.configDictionary.keys():
            rating = document.createElement("rating")
            rating.appendChild(document.createTextNode(str(self.configDictionary["rating"])))
            root.appendChild(rating)
        #rights = document.createElement("rights")
        if "pages" in self.configDictionary.keys():
            pages = document.createElement("pages")
            pages.appendChild(document.createTextNode(str(len(self.configDictionary["pages"]))))
            root.appendChild(pages)

        if "isbn-number" in self.configDictionary.keys():
            identifier = document.createElement("identifier")
            identifier.appendChild(document.createTextNode(str(self.configDictionary["isbn-number"])))
            root.appendChild(identifier)

        if "authorList" in self.configDictionary.keys():
            for authorE in range(len(self.configDictionary["authorList"])):
                author = document.createElement("creator")
                authorDict = self.configDictionary["authorList"][authorE]
                if "role" in authorDict.keys():
                    if str(authorDict["role"]).lower() in ["writer", "penciller", "editor", "assistant editor", "cover artist", "letterer", "inker", "colorist"]:
                        if str(authorDict["role"]).lower() is "cover artist":
                            author = document.createElement("coverDesigner")
                        elif str(authorDict["role"]).lower() is "assistant editor":
                            author = document.createElement("editor")
                        else:
                            author = document.createElement(str(authorDict["role"]).lower())
                stringName = []
                if "last-name" in authorDict.keys():
                    stringName.append(authorDict["last-name"])
                if "first-name" in authorDict.keys():
                    stringName.append(authorDict["first-name"])
                if "nickname" in authorDict.keys():
                    stringName.append("(" + authorDict["nickname"] + ")")
                author.appendChild(document.createTextNode(str(",".join(stringName))))
                root.appendChild(author)

        if "pages" in self.configDictionary.keys():
            if "cover" in self.configDictionary.keys():
                pageList = []
                pageList = self.configDictionary["pages"]
                coverNumber = pageList.index(self.configDictionary["cover"])
                if len(self.pagesLocationList["CBZ"]) >= coverNumber:
                    coverImage = document.createElement("coverImage")
                    coverImage.appendChild(document.createTextNode(str(os.path.basename(self.pagesLocationList["CBZ"][coverNumber]))))
                    root.appendChild(coverImage)
        readingDirection = document.createElement("readingDirection")
        readingDirection.appendChild(document.createTextNode(str("ltr")))
        if "readingDirection" in self.configDictionary.keys():
            if self.configDictionary["readingDirection"] is "rightToLeft":
                readingDirection.appendChild(document.createTextNode(str("rtl")))
        root.appendChild(readingDirection)

        f = open(location, 'w', newline="", encoding="utf-8")
        f.write(document.toprettyxml(indent="  "))
        f.close()
        self.cometLocation = location
        return True
    """
    The comicrack information is sorta... incomplete, so no idea if the following is right...
    I can't check in any case: It is a windows application.
    """

    def write_comic_rack_info(self):
        location = str(os.path.join(self.projectURL, self.configDictionary["exportLocation"], "metadata", "ComicInfo.xml"))
        document = minidom.Document()
        root = document.createElement("ComicInfo")
        root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")
        root.setAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema")

        title = document.createElement("Title")
        if "title" in self.configDictionary.keys():
            title.appendChild(document.createTextNode(str(self.configDictionary["title"])))
        else:
            title.appendChild(document.createTextNode(str("Untitled Comic")))
        root.appendChild(title)
        description = document.createElement("Summary")
        if "summary" in self.configDictionary.keys():
            description.appendChild(document.createTextNode(str(self.configDictionary["summary"])))
        else:
            description.appendChild(document.createTextNode(str("There was no summary upon generation of this file.")))
        root.appendChild(description)
        if "seriesNumber" in self.configDictionary.keys():
            number = document.createElement("Number")
            number.appendChild(document.createTextNode(str(self.configDictionary["seriesNumber"])))
            root.appendChild(number)

        if "publishingDate" in self.configDictionary.keys():
            date = QDate.fromString(self.configDictionary["publishingDate"], Qt.ISODate)
            publishYear = document.createElement("Year")
            publishYear.appendChild(document.createTextNode(str(date.year())))
            publishMonth = document.createElement("Month")
            publishMonth.appendChild(document.createTextNode(str(date.month())))
            root.appendChild(publishYear)
            root.appendChild(publishMonth)

        if "format" in self.configDictionary.keys():
            for form in self.configDictionary["format"]:
                formattag = document.createElement("Format")
                formattag.appendChild(document.createTextNode(str(form)))
                root.appendChild(formattag)
        if "otherKeywords" in self.configDictionary.keys():
            tags = document.createElement("Tags")
            tags.appendChild(document.createTextNode(str(", ".join(self.configDictionary["otherKeywords"]))))
            root.appendChild(tags)

        if "authorList" in self.configDictionary.keys():
            for authorE in range(len(self.configDictionary["authorList"])):
                author = document.createElement("Writer")
                authorDict = self.configDictionary["authorList"][authorE]
                if "role" in authorDict.keys():
                    if str(authorDict["role"]).lower() in ["writer", "penciller", "editor", "assistant editor", "cover artist", "letterer", "inker", "colorist"]:
                        if str(authorDict["role"]).lower() is "cover artist":
                            author = document.createElement("CoverArtist")
                        elif str(authorDict["role"]).lower() is "assistant editor":
                            author = document.createElement("Editor")
                        else:
                            author = document.createElement(str(authorDict["role"]).title())
                stringName = []
                if "last-name" in authorDict.keys():
                    stringName.append(authorDict["last-name"])
                if "first-name" in authorDict.keys():
                    stringName.append(authorDict["first-name"])
                if "nickname" in authorDict.keys():
                    stringName.append("(" + authorDict["nickname"] + ")")
                author.appendChild(document.createTextNode(str(",".join(stringName))))
                root.appendChild(author)
        if "publisherName" in self.configDictionary.keys():
            publisher = document.createElement("Publisher")
            publisher.appendChild(document.createTextNode(str(self.configDictionary["publisherName"])))
            root.appendChild(publisher)

        if "genre" in self.configDictionary.keys():
            for genreE in self.configDictionary["genre"]:
                genre = document.createElement("Genre")
                genre.appendChild(document.createTextNode(str(genreE)))
                root.appendChild(genre)
        blackAndWhite = document.createElement("BlackAndWhite")
        blackAndWhite.appendChild(document.createTextNode(str("No")))
        root.appendChild(blackAndWhite)
        readingDirection = document.createElement("Manga")
        readingDirection.appendChild(document.createTextNode(str("No")))
        if "readingDirection" in self.configDictionary.keys():
            if self.configDictionary["readingDirection"] is "rightToLeft":
                readingDirection.appendChild(document.createTextNode(str("Yes")))
        root.appendChild(readingDirection)

        if "characters" in self.configDictionary.keys():
            for char in self.configDictionary["characters"]:
                character = document.createElement("Character")
                character.appendChild(document.createTextNode(str(char)))
                root.appendChild(character)
        if "pages" in self.configDictionary.keys():
            pagecount = document.createElement("PageCount")
            pagecount.appendChild(document.createTextNode(str(len(self.configDictionary["pages"]))))
            root.appendChild(pagecount)
        pages = document.createElement("Pages")
        covernumber = 0
        if "pages" in self.configDictionary.keys() and "cover" in self.configDictionary.keys():
            covernumber = self.configDictionary["pages"].index(self.configDictionary["cover"])
        for i in range(len(self.pagesLocationList["CBZ"])):
            page = document.createElement("Page")
            page.setAttribute("Image", str(i))
            if i is covernumber:
                page.setAttribute("Type", "FrontCover")
            pages.appendChild(page)
        root.appendChild(pages)
        document.appendChild(root)
        f = open(location, 'w', newline="", encoding="utf-8")
        f.write(document.toprettyxml(indent="  "))
        f.close()
        self.comicRackInfo = location
        return True
    """
    Another metadata format but then a json dump stored into the zipfile comment.
    Doesn't seem to be supported much. :/
    https://code.google.com/archive/p/comicbookinfo/wikis/Example.wiki
    """

    def write_comic_book_info_json(self):
        self.comic_book_info_json_dump = str()

        basedata = {}
        metadata = {}
        authorList = []
        taglist = []

        if "authorList" in self.configDictionary.keys():
            for authorE in range(len(self.configDictionary["authorList"])):
                author = {}

                authorDict = self.configDictionary["authorList"][authorE]
                stringName = []
                if "last-name" in authorDict.keys():
                    stringName.append(authorDict["last-name"])
                if "first-name" in authorDict.keys():
                    stringName.append(authorDict["first-name"])
                if "nickname" in authorDict.keys():
                    stringName.append("(" + authorDict["nickname"] + ")")
                author["person"] = ",".join(stringName)
                if "role" in authorDict.keys():
                    author["role"] = str(authorDict["role"]).title()
                authorList.append(author)
        if "characters" in self.configDictionary.keys():
            for character in self.configDictionary["characters"]:
                taglist.append(character)
        if "format" in self.configDictionary.keys():
            for item in self.configDictionary["format"]:
                taglist.append(item)
        if "otherKeywords" in self.configDictionary.keys():
            for item in self.configDictionary["otherKeywords"]:
                taglist.append(item)

        if "seriesName" in self.configDictionary.keys():
            metadata["series"] = self.configDictionary["seriesName"]
        if "title" in self.configDictionary.keys():
            metadata["title"] = self.configDictionary["title"]
        else:
            metadata["title"] = "Unnamed comic"
        if "publisherName" in self.configDictionary.keys():
            metadata["publisher"] = self.configDictionary["publisherName"]
        if "publishingDate" in self.configDictionary.keys():
            date = QDate.fromString(self.configDictionary["publishingDate"], Qt.ISODate)
            metadata["publicationMonth"] = date.month()
            metadata["publicationYear"] = date.year()
        if "seriesNumber" in self.configDictionary.keys():
            metadata["issue"] = self.configDictionary["seriesNumber"]
        if "seriesVolume" in self.configDictionary.keys():
            metadata["volume"] = self.configDictionary["seriesVolume"]
        if "genre" in self.configDictionary.keys():
            metadata["genre"] = self.configDictionary["genre"]
        if "language" in self.configDictionary.keys():
            metadata["language"] = QLocale.languageToString(QLocale(self.configDictionary["language"]).language())

        metadata["credits"] = authorList

        metadata["tags"] = taglist
        if "summary" in self.configDictionary.keys():
            metadata["comments"] = self.configDictionary["summary"]
        else:
            metadata["comments"] = "File generated without summary"

        basedata["appID"] = "Krita"
        basedata["lastModified"] = QDateTime.currentDateTimeUtc().toString(Qt.ISODate)
        basedata["ComicBookInfo/1.0"] = metadata

        self.comic_book_info_json_dump = json.dumps(basedata)
        return True

    """
    package cbz puts all the meta-data and relevant files into an zip file ending with ".cbz"
    """

    def package_cbz(self):

        # Use the project name if there's no title to avoid sillyness with unnamed zipfiles.
        title = self.configDictionary["projectName"]
        if "title" in self.configDictionary.keys():
            title = self.configDictionary["title"]

        # Get the appropriate path.
        url = os.path.join(self.projectURL, self.configDictionary["exportLocation"], title + ".cbz")

        # Create a zip file.
        cbzArchive = zipfile.ZipFile(url, mode="w", compression=zipfile.ZIP_STORED)

        # Add all the meta data files.
        cbzArchive.write(self.acbfLocation, os.path.basename(self.acbfLocation))
        cbzArchive.write(self.cometLocation, os.path.basename(self.cometLocation))
        cbzArchive.write(self.comicRackInfo, os.path.basename(self.comicRackInfo))
        cbzArchive.comment = self.comic_book_info_json_dump.encode("utf-8")

        # Add the pages.
        if "CBZ" in self.pagesLocationList.keys():
            for page in self.pagesLocationList["CBZ"]:
                if (os.path.exists(page)):
                    cbzArchive.write(page, os.path.basename(page))

        # Close the zip file when done.
        cbzArchive.close()

    """
    package epub packages the whole epub folder and renames the zip file to .epub.
    """

    def package_epub(self):

        # Use the project name if there's no title to avoid sillyness with unnamed zipfiles.
        title = self.configDictionary["projectName"]
        if "title" in self.configDictionary.keys():
            title = self.configDictionary["title"]

        # Get the appropriate paths.
        url = os.path.join(self.projectURL, self.configDictionary["exportLocation"], title)
        epub = os.path.join(self.projectURL, self.configDictionary["exportLocation"], "EPUB-files")

        # Make the archive.
        shutil.make_archive(base_name=url, format="zip", root_dir=epub)

        # Rename the archive to epub.
        shutil.move(src=str(url + ".zip"), dst=str(url + ".epub"))
