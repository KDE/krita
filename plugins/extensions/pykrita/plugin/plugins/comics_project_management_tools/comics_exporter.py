"""
Part of the comics project management tools (CPMT).

An exporter that take the comicsConfig and uses it to generate several files.
"""
import sys
import os
from pathlib import Path
import json
import zipfile
import xml.etree.ElementTree as ET
import shutil
from PyQt5.QtWidgets import QLabel, QProgressDialog  # For the progress dialog.
from PyQt5.QtCore import QElapsedTimer, QDateTime, QLocale, Qt
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
    cometLocation = str()
    comicRackInfo = str()
    comic_book_info_json_dump = str()
    pagesLocationList = {}

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

            for p in range(0, len(pagesList)):

                # Update the label in the progress dialog.
                progress.setValue(p)
                timePassed = timer.elapsed()
                if (p > 0):
                    timeEstimated = (len(pagesList) - p) * (timePassed / p)
                    passedString = str(int(timePassed / 60000)) + ":" + format(int(timePassed / 1000), "02d") + ":" + format(timePassed % 1000, "03d")
                    estimatedString = str(int(timeEstimated / 60000)) + ":" + format(int(timeEstimated / 1000), "02d") + ":" + format(int(timeEstimated % 1000), "03d")
                    progress.setLabelText(str(i18n("{pages} of {pagesTotal} done. \nTime passed: {passedString}:\n Estimated:{estimated}")).format(pages=p, pagesTotal=len(pagesList), passedString=passedString, estimated=estimatedString))

                # Get the appropriate url and open the page.
                url = os.path.join(self.projectURL, pagesList[p])
                page = Application.openDocument(url)

                # remove layers and flatten.
                labelList = self.configDictionary["labelsToRemove"]
                root = page.rootNode()
                self.removeLayers(labelList, node=root)
                page.refreshProjection()
                page.flatten()
                while page.isIdle() is False:
                    page.waitForDone()

                # Start making the format specific copy.
                if page.isIdle():
                    for key in sizesList.keys():
                        w = sizesList[key]
                        # copy over data
                        projection = Application.createDocument(page.width(), page.height(), page.name(), page.colorModel(), page.colorDepth(), page.colorProfile())
                        batchsave = Application.batchmode()
                        Application.setBatchmode(True)
                        projection.activeNode().setPixelData(page.pixelData(0, 0, page.width(), page.height()), 0, 0, page.width(), page.height())

                        # Crop. Cropping per guide only happens if said guides have been found.
                        if w["Crop"] is True:
                            listHGuides = page.horizontalGuides()
                            listHGuides.sort()
                            listVGuides = page.verticalGuides()
                            listVGuides.sort()
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

                        # resize appropriately
                        res = page.resolution()
                        listScales = [projection.width(), projection.height(), res, res]
                        sizesCalc = sizesCalculator()
                        listScales = sizesCalc.get_scale_from_resize_config(config=w, listSizes=listScales)
                        projection.scaleImage(listScales[0], listScales[1], listScales[2], listScales[3], "bicubic")
                        projection.waitForDone()

                        # png, gif and other webformats should probably be in 8bit srgb at maximum.
                        if key is not "TIFF":
                            if projection.colorModel() is not "RGBA" or projection.colorModel() is not "GRAYA" or projection.colorDepth() is not "U8":
                                projection.setColorSpace("RGBA", "U8", "sRGB built-in")
                                projection.refreshProjection()
                        else:
                            # Tiff on the other hand can handle all the colormodels, but can only handle integer bit depths.
                            # Tiff is intended for print output, and 16 bit integer will be sufficient.
                            if projection.colorDepth() is not "U8" or projection.colorDepth() is not "U16":
                                projection.setColorSpace(page.colorModel(), "U16", page.colorProfile())
                                projection.refreshProjection()

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
                        if projection.isIdle():
                            projection.activeNode().save(fn, projection.resolution(), projection.resolution())
                            projection.waitForDone()
                        self.pagesLocationList[key].append(fn)

                        # close
                        Application.setBatchmode(batchsave)
                        projection.close()
                    page.close()
            progress.setValue(len(pagesList))
            # TODO: Check what or whether memory leaks are still caused and otherwise remove the entry below.
            print("CPMT: Export has finished. There are memory leaks, but the source is not obvious due wild times on git master. Last attempt to fix was august 2017")
            return True
        print("CPMT: Export not happening because there aren't any pages.")
        return False

    """
    Function to remove layers when they have the given labels.

    If not, but the node does have children, check those too.
    """

    def removeLayers(self, labels, node):
        if node.colorLabel() in labels:
            node.remove()
        else:
            if len(node.childNodes()) > 0:
                for child in node.childNodes():
                    self.removeLayers(labels, node=child)

    """
    Write the Advanced Comic Book Data xml file.
    
    http://acbf.wikia.com/wiki/ACBF_Specifications
    
    """

    def write_acbf_meta_data(self):
        acbfGenreList = ["science_fiction", "fantasy", "adventure", "horror", "mystery", "crime", "military", "real_life", "superhero", "humor", "western", "manga", "politics", "caricature", "sports", "history", "biography", "education", "computer", "religion", "romance", "children", "non-fiction", "adult", "alternative", "other"]
        acbfAuthorRolesList = ["Writer", "Adapter", "Artist", "Penciller", "Inker", "Colorist", "Letterer", "Cover Artist", "Photographer", "Editor", "Assistant Editor", "Translator", "Other"]
        title = self.configDictionary["projectName"]
        if "title" in self.configDictionary.keys():
            title = self.configDictionary["title"]
        location = str(os.path.join(self.projectURL, self.configDictionary["exportLocation"], "metadata", title + ".acbf"))
        document = ET.ElementTree()
        root = ET.Element("ACBF")
        root.set("xmlns", "http://www.fictionbook-lib.org/xml/acbf/1.0")
        document._setroot(root)

        meta = ET.Element("meta-data")

        bookInfo = ET.Element("book-info")
        if "authorList" in self.configDictionary.keys():
            for authorE in range(len(self.configDictionary["authorList"])):
                author = ET.Element("author")
                authorDict = self.configDictionary["authorList"][authorE]
                if "first-name" in authorDict.keys():
                    authorN = ET.Element("first-name")
                    authorN.text = str(authorDict["first-name"])
                    author.append(authorN)
                if "last-name" in authorDict.keys():
                    authorN = ET.Element("last-name")
                    authorN.text = str(authorDict["last-name"])
                    author.append(authorN)
                if "initials" in authorDict.keys():
                    authorN = ET.Element("middle-name")
                    authorN.text = str(authorDict["initials"])
                    author.append(authorN)
                if "nickname" in authorDict.keys():
                    authorN = ET.Element("nickname")
                    authorN.text = str(authorDict["nickname"])
                    author.append(authorN)
                if "homepage" in authorDict.keys():
                    authorN = ET.Element("homepage")
                    authorN.text = str(authorDict["homepage"])
                    author.append(authorN)
                if "email" in authorDict.keys():
                    authorN = ET.Element("email")
                    authorN.text = str(authorDict["email"])
                    author.append(authorN)
                if "role" in authorDict.keys():
                    if str(authorDict["role"]).title() in acbfAuthorRolesList:
                        author.set("activity", str(authorDict["role"]))
                if "language" in authorDict.keys():
                    author.set("lang", str(authorDict["language"]))
                bookInfo.append(author)
        bookTitle = ET.Element("book-title")
        if "title" in self.configDictionary.keys():
            bookTitle.text = str(self.configDictionary["title"])
        else:
            bookTitle.text = "Comic with no Name"
        bookInfo.append(bookTitle)
        extraGenres = []
        if "genre" in self.configDictionary.keys():
            for genre in self.configDictionary["genre"]:
                genreModified = str(genre).lower()
                genreModified.replace(" ", "_")
                if genreModified in acbfGenreList:
                    bookGenre = ET.Element("genre")
                    bookGenre.text = genreModified
                    bookInfo.append(bookGenre)
                else:
                    extraGenres.append(genre)
        annotation = ET.Element("annotation")
        if "summary" in self.configDictionary.keys():
            paragraphList = str(self.configDictionary["summary"]).split("\n")
            for para in paragraphList:
                p = ET.Element("p")
                p.text = para
                annotation.append(p)
        else:
            p = ET.Element("p")
            p.text = "There was no summary upon generation of this file."
            annotation.append(p)
        bookInfo.append(annotation)

        if "characters" in self.configDictionary.keys():
            character = ET.Element("characters")
            for name in self.configDictionary["characters"]:
                char = ET.Element("name")
                char.text = name
                character.append(char)
            bookInfo.append(character)

        keywords = ET.Element("keywords")
        stringKeywordsList = []
        for key in extraGenres:
            stringKeywordsList.append(str(key))
        if "otherKeywords" in self.configDictionary.keys():
            for key in self.configDictionary["otherKeywords"]:
                stringKeywordsList.append(str(key))
        if "format" in self.configDictionary.keys():
            for key in self.configDictionary["format"]:
                stringKeywordsList.append(str(key))
        keywords.text = ", ".join(stringKeywordsList)
        bookInfo.append(keywords)

        coverpageurl = ""
        coverpage = ET.Element("coverpage")
        if "pages" in self.configDictionary.keys():
            if "cover" in self.configDictionary.keys():
                pageList = []
                pageList = self.configDictionary["pages"]
                coverNumber = pageList.index(self.configDictionary["cover"])
                image = ET.Element("image")
                if len(self.pagesLocationList["CBZ"]) >= coverNumber:
                    coverpageurl = self.pagesLocationList["CBZ"][coverNumber]
                    image.set("href", os.path.basename(coverpageurl))
                coverpage.append(image)
        bookInfo.append(coverpage)

        if "language" in self.configDictionary.keys():
            language = ET.Element("languages")
            textlayer = ET.Element("text-layer")
            textlayer.set("lang", self.configDictionary["language"])
            textlayer.set("show", "False")
            language.append(textlayer)
            bookInfo.append(language)
        #database = ET.Element("databaseref")
        # bookInfo.append(database)

        if "seriesName" in self.configDictionary.keys():
            sequence = ET.Element("sequence")
            sequence.set("title", self.configDictionary["seriesName"])
            if "seriesVolume" in self.configDictionary.keys():
                sequence.set("volume", str(self.configDictionary["seriesVolume"]))
            if "seriesNumber" in self.configDictionary.keys():
                sequence.text = str(self.configDictionary["seriesNumber"])
            else:
                sequence.text = 0
            bookInfo.append(sequence)
        contentrating = ET.Element("content-rating")

        if "rating" in self.configDictionary.keys():
            contentrating.text = self.configDictionary["rating"]
        else:
            contentrating.text = "Unrated."
        if "ratingSystem" in self.configDictionary.keys():
            contentrating.set("type", self.configDictionary["ratingSystem"])
        bookInfo.append(contentrating)
        meta.append(bookInfo)

        publisherInfo = ET.Element("publish-info")
        if "publisherName" in self.configDictionary.keys():
            publisherName = ET.Element("publisher")
            publisherName.text = self.configDictionary["publisherName"]
            publisherInfo.append(publisherName)
        if "publishingDate" in self.configDictionary.keys():
            publishingDate = ET.Element("publish-date")
            publishingDate.set("value", self.configDictionary["publishingDate"])
            publishingDate.text = QDate.fromString(self.configDictionary["publishingDate"], Qt.ISODate).toString(Qt.SystemLocaleLongDate)
            publisherInfo.append(publishingDate)
        if "publisherCity" in self.configDictionary.keys():
            publishCity = ET.Element("city")
            publishCity.text = self.configDictionary["publisherCity"]
            publisherInfo.append(publishCity)
        if "isbn-number" in self.configDictionary.keys():
            publishISBN = ET.Element("isbn")
            publishISBN.text = self.configDictionary["isbn-number"]
            publisherInfo.append(publishISBN)
        if "license" in self.configDictionary.keys():
            license = self.configDictionary["license"]
            if license.isspace() is False and len(license) > 0:
                publishLicense = ET.Element("license")
                publishLicense.text = self.configDictionary["license"]
                publisherInfo.append(publishLicense)

        meta.append(publisherInfo)

        documentInfo = ET.Element("document-info")
        acbfAuthor = ET.Element("author")
        if "acbfAuthor" in self.configDictionary.keys():
            acbfAuthor.text = self.configDictionary["acbfAuthor"]
        else:
            acbfAuthor.text = "Anon"
        documentInfo.append(acbfAuthor)

        acbfDate = ET.Element("creation-date")
        now = QDate.currentDate()
        acbfDate.set("value", now.toString(Qt.ISODate))
        acbfDate.text = now.toString(Qt.SystemLocaleLongDate)
        documentInfo.append(acbfDate)

        acbfSource = ET.Element("source")
        if "acbfSource" in self.configDictionary.keys():
            acbfSource.text = self.configDictionary["acbfSource"]
        documentInfo.append(acbfSource)

        acbfID = ET.Element("id")
        if "acbfID" in self.configDictionary.keys():
            acbfID.text = self.configDictionary["acbfID"]
        documentInfo.append(acbfID)

        acbfVersion = ET.Element("version")
        if "acbfVersion" in self.configDictionary.keys():
            acbfVersion.text = str(self.configDictionary["acbfVersion"])
        documentInfo.append(acbfVersion)

        acbfHistory = ET.Element("history")
        if "acbfHistory" in self.configDictionary.keys():
            for h in self.configDictionary["acbfHistory"]:
                p = ET.Element("p")
                p.text = h
                acbfHistory.append(p)
        documentInfo.append(acbfHistory)
        meta.append(documentInfo)

        root.append(meta)

        body = ET.Element("body")

        for page in self.pagesLocationList["CBZ"]:
            if page is not coverpageurl:
                pg = ET.Element("page")
                image = ET.Element("image")
                image.set("href", os.path.basename(page))
                pg.append(image)
                body.append(pg)

        root.append(body)

        document.write(location, encoding="UTF-8", xml_declaration=True)
        self.acbfLocation = location
        return True

    """
    Write a CoMet xml file to url
    """

    def write_comet_meta_data(self):
        title = self.configDictionary["projectName"]
        if "title" in self.configDictionary.keys():
            title = self.configDictionary["title"]
        location = str(os.path.join(self.projectURL, self.configDictionary["exportLocation"], "metadata", title + " CoMet.xml"))
        document = ET.ElementTree()
        root = ET.Element("comet")
        root.set("xmlns:comet", "http://www.denvog.com/comet/")
        root.set("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")
        root.set("xsi:schemaLocation", "http://www.denvog.com http://www.denvog.com/comet/comet.xsd")
        document._setroot(root)

        title = ET.Element("title")
        if "title" in self.configDictionary.keys():
            title.text = self.configDictionary["title"]
        else:
            title.text = "Untitled Comic"
        root.append(title)
        description = ET.Element("description")
        if "summary" in self.configDictionary.keys():
            description.text = self.configDictionary["summary"]
        else:
            description.text = "There was no summary upon generation of this file."
        root.append(description)
        if "seriesName" in self.configDictionary.keys():
            series = ET.Element("series")
            series.text = self.configDictionary["seriesName"]
            root.append(series)
            if "seriesNumber" in self.configDictionary.keys():
                issue = ET.Element("issue")
                issue.text = str(self.configDictionary["seriesName"])
                root.append(issue)
            if "seriesVolume" in self.configDictionary.keys():
                volume = ET.Element("volume")
                volume.text = str(self.configDictionary["seriesVolume"])
                root.append(volume)

        if "publisherName" in self.configDictionary.keys():
            pubisher = ET.Element("publisher")
            pubisher.text = self.configDictionary["publisherName"]
            root.append(pubisher)

        if "publishingDate" in self.configDictionary.keys():
            date = ET.Element("date")
            date.text = self.configDictionary["publishingDate"]
            root.append(date)

        if "genre" in self.configDictionary.keys():
            for genreE in self.configDictionary["genre"]:
                genre = ET.Element("genre")
                genre.text = genreE
                root.append(genre)

        if "characters" in self.configDictionary.keys():
            for char in self.configDictionary["characters"]:
                character = ET.Element("character")
                character.text = char
                root.append(character)

        if "format" in self.configDictionary.keys():
            format = ET.Element("format")
            format.text = ",".join(self.configDictionary["format"])
            root.append(format)

        if "language" in self.configDictionary.keys():
            language = ET.Element("language")
            language.text = self.configDictionary["language"]
            root.append(language)
        if "rating" in self.configDictionary.keys():
            rating = ET.Element("rating")
            rating.text = self.configDictionary["rating"]
            root.append(rating)
        #rights = ET.Element("rights")
        if "pages" in self.configDictionary.keys():
            pages = ET.Element("pages")
            pages.text = str(len(self.configDictionary["pages"]))
            root.append(pages)

        if "isbn-number" in self.configDictionary.keys():
            identifier = ET.Element("identifier")
            identifier.text = self.configDictionary["isbn-number"]
            root.append(identifier)

        if "authorList" in self.configDictionary.keys():
            for authorE in range(len(self.configDictionary["authorList"])):
                author = ET.Element("creator")
                authorDict = self.configDictionary["authorList"][authorE]
                if "role" in authorDict.keys():
                    if str(authorDict["role"]).lower() in ["writer", "penciller", "editor", "assistant editor", "cover artist", "letterer", "inker", "colorist"]:
                        if str(authorDict["role"]).lower() is "cover artist":
                            author = ET.Element("coverDesigner")
                        elif str(authorDict["role"]).lower() is "assistant editor":
                            author = ET.Element("editor")
                        else:
                            author = ET.Element(str(authorDict["role"]).lower())
                stringName = []
                if "last-name" in authorDict.keys():
                    stringName.append(authorDict["last-name"])
                if "first-name" in authorDict.keys():
                    stringName.append(authorDict["first-name"])
                if "last-name" in authorDict.keys():
                    stringName.append("(" + authorDict["nickname"] + ")")
                author.text = ",".join(stringName)
                root.append(author)

        if "pages" in self.configDictionary.keys():
            if "cover" in self.configDictionary.keys():
                pageList = []
                pageList = self.configDictionary["pages"]
                coverNumber = pageList.index(self.configDictionary["cover"])
                if len(self.pagesLocationList["CBZ"]) >= coverNumber:
                    coverImage = ET.Element("coverImage")
                    coverImage.text = os.path.basename(self.pagesLocationList["CBZ"][coverNumber])
                    root.append(coverImage)
        readingDirection = ET.Element("readingDirection")
        readingDirection.text = "ltr"
        if "readingDirection" in self.configDictionary.keys():
            if self.configDictionary["readingDirection"] is "rightToLeft":
                readingDirection.text = "rtl"
        root.append(readingDirection)

        document.write(location, encoding="UTF-8", xml_declaration=True)
        self.cometLocation = location
        return True
    """
    The comicrack information is sorta... incomplete, so no idea if the following is right...
    I can't check in any case: It is a windows application.
    """

    def write_comic_rack_info(self):
        location = str(os.path.join(self.projectURL, self.configDictionary["exportLocation"], "metadata", "ComicInfo.xml"))
        document = ET.ElementTree()
        root = ET.Element("ComicInfo")
        root.set("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")
        root.set("xmlns:xsd", "http://www.w3.org/2001/XMLSchema")

        title = ET.Element("Title")
        if "title" in self.configDictionary.keys():
            title.text = self.configDictionary["title"]
        else:
            title.text = "Untitled Comic"
        root.append(title)
        description = ET.Element("Summary")
        if "summary" in self.configDictionary.keys():
            description.text = self.configDictionary["summary"]
        else:
            description.text = "There was no summary upon generation of this file."
        root.append(description)
        if "seriesNumber" in self.configDictionary.keys():
            number = ET.Element("Number")
            number.text = str(self.configDictionary["seriesNumber"])
            root.append(number)

        if "publishingDate" in self.configDictionary.keys():
            date = QDate.fromString(self.configDictionary["publishingDate"], Qt.ISODate)
            publishYear = ET.Element("Year")
            publishYear.text = str(date.year())
            publishMonth = ET.Element("Month")
            publishMonth.text = str(date.month())
            root.append(publishYear)
            root.append(publishMonth)

        if "format" in self.configDictionary.keys():
            for form in self.configDictionary["format"]:
                formattag = ET.Element("Format")
                formattag.text = str(form)
                root.append(formattag)
        if "otherKeywords" in self.configDictionary.keys():
            tags = ET.Element("Tags")
            tags.text = ", ".join(self.configDictionary["otherKeywords"])
            root.append(tags)

        if "authorList" in self.configDictionary.keys():
            for authorE in range(len(self.configDictionary["authorList"])):
                author = ET.Element("Writer")
                authorDict = self.configDictionary["authorList"][authorE]
                if "role" in authorDict.keys():
                    if str(authorDict["role"]).lower() in ["writer", "penciller", "editor", "assistant editor", "cover artist", "letterer", "inker", "colorist"]:
                        if str(authorDict["role"]).lower() is "cover artist":
                            author = ET.Element("CoverArtist")
                        elif str(authorDict["role"]).lower() is "assistant editor":
                            author = ET.Element("Editor")
                        else:
                            author = ET.Element(str(authorDict["role"]).title())
                stringName = []
                if "last-name" in authorDict.keys():
                    stringName.append(authorDict["last-name"])
                if "first-name" in authorDict.keys():
                    stringName.append(authorDict["first-name"])
                if "last-name" in authorDict.keys():
                    stringName.append("(" + authorDict["nickname"] + ")")
                author.text = ",".join(stringName)
                root.append(author)
        if "publisherName" in self.configDictionary.keys():
            publisher = ET.Element("Publisher")
            publisher.text = self.configDictionary["publisherName"]
            root.append(publisher)

        if "genre" in self.configDictionary.keys():
            for genreE in self.configDictionary["genre"]:
                genre = ET.Element("Genre")
                genre.text = genreE
                root.append(genre)
        blackAndWhite = ET.Element("BlackAndWhite")
        blackAndWhite.text = "No"
        root.append(blackAndWhite)
        readingDirection = ET.Element("Manga")
        readingDirection.text = "No"
        if "readingDirection" in self.configDictionary.keys():
            if self.configDictionary["readingDirection"] is "rightToLeft":
                readingDirection.text = "Yes"
        root.append(readingDirection)

        if "characters" in self.configDictionary.keys():
            for char in self.configDictionary["characters"]:
                character = ET.Element("Character")
                character.text = char
                root.append(character)
        if "pages" in self.configDictionary.keys():
            pagecount = ET.Element("PageCount")
            pagecount.text = str(len(self.configDictionary["pages"]))
            root.append(pagecount)
        pages = ET.Element("Pages")
        covernumber = 0
        if "pages" in self.configDictionary.keys() and "cover" in self.configDictionary.keys():
            covernumber = self.configDictionary["pages"].index(self.configDictionary["cover"])
        for i in range(len(self.pagesLocationList["CBZ"])):
            page = ET.Element("Page")
            page.set("Image", str(i))
            if i is covernumber:
                page.set("Type", "FrontCover")
            pages.append(page)
        root.append(pages)
        document._setroot(root)
        document.write(location, encoding="UTF-8", xml_declaration=True)
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
