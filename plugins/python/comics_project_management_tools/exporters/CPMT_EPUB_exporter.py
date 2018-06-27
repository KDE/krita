"""
Copyright (c) 2018 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

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
Create an epub folder, finally, package to a epubzip.
"""

import shutil
import os
from pathlib import Path
import xml.etree.ElementTree as ET

def export(configDictionary = {}, projectURL = str(), pagesLocationList = []):
    path = Path(os.path.join(projectURL, configDictionary["exportLocation"]))
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
    if len(pagesLocationList)>0:
        if "cover" in configDictionary.keys():
            coverNumber = configDictionary["pages"].index(configDictionary["cover"])
        else:
            coverNumber = 0
        for p in pagesLocationList:
            if os.path.exists(p):
                shutil.copy2(p, str(imagePath))
                pagesList.append(str(Path(imagePath / os.path.basename(p))))
        if len(pagesLocationList) >= coverNumber:
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

    if "language" in configDictionary.keys():
        bookLang = ET.Element("dc:language")
        bookLang.text = configDictionary["language"]
        opfMeta.append(bookLang)
    bookTitle = ET.Element("dc:title")
    if "title" in configDictionary.keys():
        bookTitle.text = str(configDictionary["title"])
    else:
        bookTitle.text = "Comic with no Name"
    opfMeta.append(bookTitle)
    if "authorList" in configDictionary.keys():
        for authorE in range(len(configDictionary["authorList"])):
            authorDict = configDictionary["authorList"][authorE]
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

    if "publishingDate" in configDictionary.keys():
        date = ET.Element("dc:date")
        date.text = configDictionary["publishingDate"]
        opfMeta.append(date)
    description = ET.Element("dc:description")
    if "summary" in configDictionary.keys():
        description.text = configDictionary["summary"]
    else:
        description.text = "There was no summary upon generation of this file."
    opfMeta.append(description)

    type = ET.Element("dc:type")
    type.text = "Comic"
    opfMeta.append(type)
    if "publisherName" in configDictionary.keys():
        publisher = ET.Element("dc:publisher")
        publisher.text = configDictionary["publisherName"]
        opfMeta.append(publisher)
    if "isbn-number" in configDictionary.keys():
        publishISBN = ET.Element("dc:identifier")
        publishISBN.text = str("urn:isbn:") + configDictionary["isbn-number"]
        opfMeta.append(publishISBN)
    if "license" in configDictionary.keys():
        rights = ET.Element("dc:rights")
        rights.text = configDictionary["license"]
        opfMeta.append(rights)

    if "genre" in configDictionary.keys():
        genreListConf = configDictionary["genre"]
        if isinstance(configDictionary["genre"], dict):
            genreListConf = configDictionary["genre"].keys()
        for g in genreListConf:
            subject = ET.Element("dc:subject")
            subject.text = g
            opfMeta.append(subject)
    if "characters" in configDictionary.keys():
        for name in configDictionary["characters"]:
            char = ET.Element("dc:subject")
            char.text = name
            opfMeta.append(char)
    if "format" in configDictionary.keys():
        for format in configDictionary["format"]:
            f = ET.Element("dc:subject")
            f.text = format
            opfMeta.append(f)
    if "otherKeywords" in configDictionary.keys():
        for key in configDictionary["otherKeywords"]:
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
    if "title" in configDictionary.keys():
        text.text = str(configDictionary["title"])
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

    package_epub(configDictionary, projectURL)
    return True
"""
package epub packages the whole epub folder and renames the zip file to .epub.
"""

def package_epub(configDictionary = {}, projectURL = str()):

    # Use the project name if there's no title to avoid sillyness with unnamed zipfiles.
    title = configDictionary["projectName"]
    if "title" in configDictionary.keys():
        title = str(configDictionary["title"]).replace(" ", "_")

    # Get the appropriate paths.
    url = os.path.join(projectURL, configDictionary["exportLocation"], title)
    epub = os.path.join(projectURL, configDictionary["exportLocation"], "EPUB-files")

    # Make the archive.
    shutil.make_archive(base_name=url, format="zip", root_dir=epub)

    # Rename the archive to epub.
    shutil.move(src=str(url + ".zip"), dst=str(url + ".epub"))
