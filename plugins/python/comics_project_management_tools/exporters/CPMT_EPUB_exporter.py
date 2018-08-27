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
from PyQt5.QtXml import QDomDocument, QDomElement, QDomText, QDomNodeList

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

    container = QDomDocument()
    cRoot = container.createElement("container")
    cRoot.setAttribute("version", "1.0")
    cRoot.setAttribute("xmlns", "urn:oasis:names:tc:opendocument:xmlns:container")
    container.appendChild(cRoot)
    rootFiles = container.createElement("rootfiles")
    rootfile = container.createElement("rootfile")
    rootfile.setAttribute("full-path", "OEBPS/content.opf")
    rootfile.setAttribute("media-type", "application/oebps-package+xml")
    rootFiles.appendChild(rootfile)
    cRoot.appendChild(rootFiles)

    containerFile = open(str(Path(metaInf / "container.xml")), 'w', newline="", encoding="utf-8")
    containerFile.write(container.toString(indent=2))
    containerFile.close()

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
        print("CPMT: Couldn't find the location for the epub images.")
        return False

    # for each image, make an xml file
    htmlFiles = []
    for i in range(len(pagesList)):
        pageName = "Page" + str(i) + ".xhtml"
        doc = QDomDocument()
        html = doc.createElement("html")
        doc.appendChild(html)
        html.setAttribute("xmlns", "http://www.w3.org/1999/xhtml")
        html.setAttribute("xmlns:epub", "http://www.idpf.org/2007/ops")

        head = doc.createElement("head")
        html.appendChild(head)

        body = doc.createElement("body")

        img = doc.createElement("img")
        img.setAttribute("src", os.path.relpath(pagesList[i], str(textPath)))
        body.appendChild(img)

        html.appendChild(body)

        filename = str(Path(textPath / pageName))
        docFile = open(filename, 'w', newline="", encoding="utf-8")
        docFile.write(doc.toString(indent=2))
        docFile.close()

        if pagesList[i] == coverpageurl:
            coverpagehtml = os.path.relpath(filename, str(oebps))
        htmlFiles.append(filename)

    # metadata
    
    write_opf_file(oebps, configDictionary, htmlFiles, pagesList, coverpageurl, coverpagehtml)
    
    # toc
    write_ncx_file(oebps, configDictionary, htmlFiles)
    

    package_epub(configDictionary, projectURL)
    return True

"""
Write OPF metadata file
"""


def write_opf_file(path, configDictionary, htmlFiles, pagesList, coverpageurl, coverpagehtml):
    
    # opf file
    opfFile = QDomDocument()
    opfRoot = opfFile.createElement("package")
    opfRoot.setAttribute("version", "3.0")
    opfRoot.setAttribute("unique-identifier", "BookId")
    opfRoot.setAttribute("xmlns", "http://www.idpf.org/2007/opf")
    opfFile.appendChild(opfRoot)
    
    opfMeta = opfFile.createElement("metadata")
    opfMeta.setAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/")

    if "language" in configDictionary.keys():
        bookLang = opfFile.createElement("dc:language")
        bookLang.appendChild(opfFile.createTextNode(configDictionary["language"]))
        opfMeta.appendChild(bookLang)
    bookTitle = opfFile.createElement("dc:title")
    if "title" in configDictionary.keys():
        bookTitle.appendChild(opfFile.createTextNode(str(configDictionary["title"])))
    else:
        bookTitle.appendChild(opfFile.createTextNode("Comic with no Name"))
    opfMeta.appendChild(bookTitle)


    if "authorList" in configDictionary.keys():
        for authorE in range(len(configDictionary["authorList"])):
            authorDict = configDictionary["authorList"][authorE]
            authorType = "dc:creator"
            if "role" in authorDict.keys():
                if str(authorDict["role"]).lower() in ["editor", "assistant editor", "proofreader", "beta"]:
                    authorType = "dc:contributor"
            author = opfFile.createElement(authorType)
            authorName = []
            if "last-name" in authorDict.keys():
                authorName.append(authorDict["last-name"])
            if "first-name" in authorDict.keys():
                authorName.append(authorDict["first-name"])
            if "initials" in authorDict.keys():
                authorName.append(authorDict["initials"])
            if "nickname" in authorDict.keys():
                authorName.append("(" + authorDict["nickname"] + ")")
            author.appendChild(opfFile.createTextNode(", ".join(authorName)))
            opfMeta.appendChild(author)
            if "role" in authorDict.keys():
                author.setAttribute("id", "cre" + str(authorE))
                role = opfFile.createElement("meta")
                role.setAttribute("refines", "cre" + str(authorE))
                role.setAttribute("scheme", "marc:relators")
                role.setAttribute("property", "role")
                role.appendChild(opfFile.createTextNode(str(authorDict["role"])))
                opfMeta.appendChild(role)

    if "publishingDate" in configDictionary.keys():
        date = opfFile.createElement("dc:date")
        date.appendChild(opfFile.createTextNode(configDictionary["publishingDate"]))
        opfMeta.appendChild(date)
    description = opfFile.createElement("dc:description")
    if "summary" in configDictionary.keys():
        description.appendChild(opfFile.createTextNode(configDictionary["summary"]))
    else:
        description.appendChild(opfFile.createTextNode("There was no summary upon generation of this file."))
    opfMeta.appendChild(description)

    typeE = opfFile.createElement("dc:type")
    typeE.appendChild(opfFile.createTextNode("Comic"))
    opfMeta.appendChild(typeE)
    if "publisherName" in configDictionary.keys():
        publisher = opfFile.createElement("dc:publisher")
        publisher.appendChild(opfFile.createTextNode(configDictionary["publisherName"]))
        opfMeta.appendChild(publisher)
    if "isbn-number" in configDictionary.keys():
        publishISBN = opfFile.createElement("dc:identifier")
        publishISBN.appendChild(opfFile.createTextNode(str("urn:isbn:") + configDictionary["isbn-number"]))
        opfMeta.appendChild(publishISBN)
    if "license" in configDictionary.keys():
        rights = opfFile.createElement("dc:rights")
        rights.appendChild(opfFile.createTextNode(configDictionary["license"]))
        opfMeta.appendChild(rights)

    if "genre" in configDictionary.keys():
        genreListConf = configDictionary["genre"]
        if isinstance(configDictionary["genre"], dict):
            genreListConf = configDictionary["genre"].keys()
        for g in genreListConf:
            subject = opfFile.createElement("dc:subject")
            subject.appendChild(opfFile.createTextNode(g))
            opfMeta.appendChild(subject)
    if "characters" in configDictionary.keys():
        for name in configDictionary["characters"]:
            char = opfFile.createElement("dc:subject")
            char.appendChild(opfFile.createTextNode(name))
            opfMeta.appendChild(char)
    if "format" in configDictionary.keys():
        for formatF in configDictionary["format"]:
            f = opfFile.createElement("dc:subject")
            f.appendChild(opfFile.createTextNode(formatF))
            opfMeta.appendChild(f)
    if "otherKeywords" in configDictionary.keys():
        for key in configDictionary["otherKeywords"]:
            word = opfFile.createElement("dc:subject")
            word.appendChild(opfFile.createTextNode(key))
            opfMeta.appendChild(word)

    opfRoot.appendChild(opfMeta)

    opfManifest = opfFile.createElement("manifest")
    toc = opfFile.createElement("item")
    toc.setAttribute("id", "ncx")
    toc.setAttribute("href", "toc.ncx")
    toc.setAttribute("media-type", "application/x-dtbncx+xml")
    opfManifest.appendChild(toc)
    for p in htmlFiles:
        item = opfFile.createElement("item")
        item.setAttribute("id", os.path.basename(p))
        item.setAttribute("href", os.path.relpath(p, str(path)))
        item.setAttribute("media-type", "application/xhtml+xml")
        opfManifest.appendChild(item)
    for p in pagesList:
        item = opfFile.createElement("item")
        item.setAttribute("id", os.path.basename(p))
        item.setAttribute("href", os.path.relpath(p, str(path)))
        item.setAttribute("media-type", "image/png")
        if os.path.basename(p) == os.path.basename(coverpageurl):
            item.setAttribute("properties", "cover-image")
        opfManifest.appendChild(item)

    opfRoot.appendChild(opfManifest)

    opfSpine = opfFile.createElement("spine")
    # this sets the table of contents to use the ncx file
    opfSpine.setAttribute("toc", "ncx")
    for p in htmlFiles:
        item = opfFile.createElement("itemref")
        item.setAttribute("idref", os.path.basename(p))
        opfSpine.appendChild(item)
    opfRoot.appendChild(opfSpine)

    opfGuide = opfFile.createElement("guide")
    if coverpagehtml is not None and coverpagehtml.isspace() is False and len(coverpagehtml) > 0:
        item = opfFile.createElement("reference")
        item.setAttribute("type", "cover")
        item.setAttribute("title", "Cover")
        item.setAttribute("href", coverpagehtml)
    opfRoot.appendChild(opfGuide)

    docFile = open(str(Path(path / "content.opf")), 'w', newline="", encoding="utf-8")
    docFile.write(opfFile.toString(indent=2))
    docFile.close()
    return True

"""
Write a NCX file.
"""

def write_ncx_file(path, configDictionary, htmlFiles):
    tocDoc = QDomDocument()
    ncx = tocDoc.createElement("ncx")
    ncx.setAttribute("version", "2005-1")
    ncx.setAttribute("xmlns", "http://www.daisy.org/z3986/2005/ncx/")
    tocDoc.appendChild(ncx)

    tocHead = tocDoc.createElement("head")
    metaID = tocDoc.createElement("meta")
    metaID.setAttribute("content", "ID_UNKNOWN")
    metaID.setAttribute("name", "dtb:uid")
    tocHead.appendChild(metaID)
    metaDepth = tocDoc.createElement("meta")
    metaDepth.setAttribute("content", str(0))
    metaDepth.setAttribute("name", "dtb:depth")
    tocHead.appendChild(metaDepth)
    metaTotal = tocDoc.createElement("meta")
    metaTotal.setAttribute("content", str(0))
    metaTotal.setAttribute("name", "dtb:totalPageCount")
    tocHead.appendChild(metaTotal)
    metaMax = tocDoc.createElement("meta")
    metaMax.setAttribute("content", str(0))
    metaMax.setAttribute("name", "dtb:maxPageNumber")
    tocHead.appendChild(metaDepth)
    ncx.appendChild(tocHead)

    docTitle = tocDoc.createElement("docTitle")
    text = tocDoc.createElement("text")
    if "title" in configDictionary.keys():
        text.appendChild(tocDoc.createTextNode(str(configDictionary["title"])))
    else:
        text.appendChild(tocDoc.createTextNode("Comic with no Name"))
    docTitle.appendChild(text)
    ncx.appendChild(docTitle)

    navmap = tocDoc.createElement("navMap")
    navPoint = tocDoc.createElement("navPoint")
    navPoint.setAttribute("id", "navPoint-1")
    navPoint.setAttribute("playOrder", "1")
    navLabel = tocDoc.createElement("navLabel")
    navLabelText = tocDoc.createElement("text")
    navLabelText.appendChild(tocDoc.createTextNode("Start"))
    navLabel.appendChild(navLabelText)
    navContent = tocDoc.createElement("content")
    navContent.setAttribute("src", os.path.relpath(htmlFiles[0], str(path)))
    navPoint.appendChild(navLabel)
    navPoint.appendChild(navContent)
    navmap.appendChild(navPoint)
    ncx.appendChild(navmap)

    docFile = open(str(Path(path / "toc.ncx")), 'w', newline="", encoding="utf-8")
    docFile.write(tocDoc.toString(indent=2))
    docFile.close()
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
