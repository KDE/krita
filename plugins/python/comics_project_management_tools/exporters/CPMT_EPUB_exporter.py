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
from PyQt5.QtCore import Qt, QDateTime
from PyQt5.QtGui import QImage

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
        viewport = doc.createElement("meta")
        viewport.setAttribute("name", "viewport")

        img = QImage()
        img.load(pagesLocationList[i])
        w = img.width()
        h = img.height()
        
        widthHeight = "width="+str(w)+", height="+str(h)
        
        viewport.setAttribute("content", widthHeight)
        head.appendChild(viewport)
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
    
    # marc relators
    # This has several entries removed to reduce it to the most relevant entries.
    marcRelators = {"abr":i18n("Abridger"), "acp":i18n("Art copyist"), "act":i18n("Actor"), "adi":i18n("Art director"), "adp":i18n("Adapter"), "ann":i18n("Annotator"), "ant":i18n("Bibliographic antecedent"), "arc":i18n("Architect"), "ard":i18n("Artistic director"), "art":i18n("Artist"), "asn":i18n("Associated name"), "ato":i18n("Autographer"), "att":i18n("Attributed name"), "aud":i18n("Author of dialog"), "aut":i18n("Author"), "bdd":i18n("Binding designer"), "bjd":i18n("Bookjacket designer"), "bkd":i18n("Book designer"), "bkp":i18n("Book producer"), "blw":i18n("Blurb writer"), "bnd":i18n("Binder"), "bpd":i18n("Bookplate designer"), "bsl":i18n("Bookseller"), "cll":i18n("Calligrapher"), "clr":i18n("Colorist"), "cns":i18n("Censor"), "cov":i18n("Cover designer"), "cph":i18n("Copyright holder"), "cre":i18n("Creator"), "ctb":i18n("Contributor"), "cur":i18n("Curator"), "cwt":i18n("Commentator for written text"), "drm":i18n("Draftsman"), "dsr":i18n("Designer"), "dub":i18n("Dubious author"), "edt":i18n("Editor"), "etr":i18n("Etcher"), "exp":i18n("Expert"), "fnd":i18n("Funder"), "ill":i18n("Illustrator"), "ilu":i18n("Illuminator"), "ins":i18n("Inscriber"), "lse":i18n("Licensee"), "lso":i18n("Licensor"), "ltg":i18n("Lithographer"), "mdc":i18n("Metadata contact"), "oth":i18n("Other"), "own":i18n("Owner"), "pat":i18n("Patron"), "pbd":i18n("Publishing director"), "pbl":i18n("Publisher"), "prt":i18n("Printer"), "sce":i18n("Scenarist"), "scr":i18n("Scribe"), "spn":i18n("Sponsor"), "stl":i18n("Storyteller"), "trc":i18n("Transcriber"), "trl":i18n("Translator"), "tyd":i18n("Type designer"), "tyg":i18n("Typographer"), "wac":i18n("Writer of added commentary"), "wal":i18n("Writer of added lyrics"), "wam":i18n("Writer of accompanying material"), "wat":i18n("Writer of added text"), "win":i18n("Writer of introduction"), "wpr":i18n("Writer of preface"), "wst":i18n("Writer of supplementary textual content")}
    
    # opf file
    opfFile = QDomDocument()
    opfRoot = opfFile.createElement("package")
    opfRoot.setAttribute("version", "3.0")
    opfRoot.setAttribute("unique-identifier", "BookId")
    opfRoot.setAttribute("xmlns", "http://www.idpf.org/2007/opf")
    opfRoot.setAttribute("prefix", "rendition: http://www.idpf.org/vocab/rendition/#")
    opfFile.appendChild(opfRoot)
    
    opfMeta = opfFile.createElement("metadata")
    opfMeta.setAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/")
    opfMeta.setAttribute("xmlns:dcterms", "http://purl.org/dc/terms/")

    # EPUB metadata requires a title, language and uuid

    langString = "en-US"
    if "language" in configDictionary.keys():
        langString = str(configDictionary["language"]).replace("_", "-")
    
    bookLang = opfFile.createElement("dc:language")
    bookLang.appendChild(opfFile.createTextNode(langString))
    opfMeta.appendChild(bookLang)

    bookTitle = opfFile.createElement("dc:title")
    if "title" in configDictionary.keys():
        bookTitle.appendChild(opfFile.createTextNode(str(configDictionary["title"])))
    else:
        bookTitle.appendChild(opfFile.createTextNode("Comic with no Name"))
    opfMeta.appendChild(bookTitle)
    
    # Generate series title and the like here too.
    if "seriesName" in configDictionary.keys():
        bookTitle.setAttribute("id", "main")

        refine = opfFile.createElement("meta")
        refine.setAttribute("refines", "#main")
        refine.setAttribute("property", "title-type")
        refine.appendChild(opfFile.createTextNode("main"))
        opfMeta.appendChild(refine)

        refine2 = opfFile.createElement("meta")
        refine2.setAttribute("refines", "#main")
        refine2.setAttribute("property", "display-seq")
        refine2.appendChild(opfFile.createTextNode("1"))
        opfMeta.appendChild(refine2)

        seriesTitle = opfFile.createElement("dc:title")
        seriesTitle.appendChild(opfFile.createTextNode(str(configDictionary["seriesName"])))
        seriesTitle.setAttribute("id", "series")
        opfMeta.appendChild(seriesTitle)

        refineS = opfFile.createElement("meta")
        refineS.setAttribute("refines", "#series")
        refineS.setAttribute("property", "title-type")
        refineS.appendChild(opfFile.createTextNode("collection"))
        opfMeta.appendChild(refineS)

        refineS2 = opfFile.createElement("meta")
        refineS2.setAttribute("refines", "#series")
        refineS2.setAttribute("property", "display-seq")
        refineS2.appendChild(opfFile.createTextNode("2"))
        opfMeta.appendChild(refineS2)

        if "seriesNumber" in configDictionary.keys():
            refineS3 = opfFile.createElement("meta")
            refineS3.setAttribute("refines", "#series")
            refineS3.setAttribute("property", "group-position")
            refineS3.appendChild(opfFile.createTextNode(str(configDictionary["seriesNumber"])))
            opfMeta.appendChild(refineS3)

    uuid = str(configDictionary["uuid"])
    uuid = uuid.strip("{")
    uuid = uuid.strip("}")

    # Append the id, and assign it as the bookID.
    uniqueID = opfFile.createElement("dc:identifier")
    uniqueID.appendChild(opfFile.createTextNode("urn:uuid:"+uuid))
    uniqueID.setAttribute("id", "BookId")
    opfMeta.appendChild(uniqueID)

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
                roleString = str(authorDict["role"])
                if roleString in marcRelators.values():
                    i = list(marcRelators.values()).index(roleString)
                    roleString = list(marcRelators.keys())[i]
                else:
                    roleString = "oth"
                role.appendChild(opfFile.createTextNode(roleString))
                opfMeta.appendChild(role)

    if "publishingDate" in configDictionary.keys():
        date = opfFile.createElement("dc:date")
        date.appendChild(opfFile.createTextNode(configDictionary["publishingDate"]))
        opfMeta.appendChild(date)
    
    #Creation date
    modified = opfFile.createElement("meta")
    modified.setAttribute("property", "dcterms:modified")
    modified.appendChild(opfFile.createTextNode(QDateTime.currentDateTimeUtc().toString(Qt.ISODate)))
    opfMeta.appendChild(modified)
    
    if "source" in configDictionary.keys():
        if len(configDictionary["source"])>0:
            source = opfFile.createElement("dc:source")
            source.appendChild(opfFile.createTextNode(configDictionary["source"]))
            opfMeta.appendChild(source)
    
    description = opfFile.createElement("dc:description")
    if "summary" in configDictionary.keys():
        description.appendChild(opfFile.createTextNode(configDictionary["summary"]))
    else:
        description.appendChild(opfFile.createTextNode("There was no summary upon generation of this file."))
    opfMeta.appendChild(description)

    # Type can be dictionary or index, or one of those edupub thingies. Not necessary for comics.
    # typeE = opfFile.createElement("dc:type")
    # opfMeta.appendChild(typeE)
    if "publisherName" in configDictionary.keys():
        publisher = opfFile.createElement("dc:publisher")
        publisher.appendChild(opfFile.createTextNode(configDictionary["publisherName"]))
        opfMeta.appendChild(publisher)
    
    
    if "isbn-number" in configDictionary.keys():
        publishISBN = opfFile.createElement("dc:identifier")
        publishISBN.appendChild(opfFile.createTextNode(str("urn:isbn:") + configDictionary["isbn-number"]))
        opfMeta.appendChild(publishISBN)
    if "license" in configDictionary.keys():
        if len(configDictionary["license"])>0:
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

    # Pre-pagination and layout
    # Comic are always prepaginated.
    
    elLayout = opfFile.createElement("meta")
    elLayout.setAttribute("property", "rendition:layout")
    elLayout.appendChild(opfFile.createTextNode("pre-paginated"))
    opfMeta.appendChild(elLayout)
    
    # We should figure out if the pages are portrait or not...
    elOrientation = opfFile.createElement("meta")
    elOrientation.setAttribute("property", "rendition:orientation")
    elOrientation.appendChild(opfFile.createTextNode("portrait"))
    opfMeta.appendChild(elOrientation)
    
    elSpread = opfFile.createElement("meta")
    elSpread.setAttribute("property", "rendition:spread")
    elSpread.appendChild(opfFile.createTextNode("landscape"))
    opfMeta.appendChild(elSpread)
    
    opfRoot.appendChild(opfMeta)
    
    # Manifest

    opfManifest = opfFile.createElement("manifest")
    toc = opfFile.createElement("item")
    toc.setAttribute("id", "ncx")
    toc.setAttribute("href", "toc.ncx")
    toc.setAttribute("media-type", "application/x-dtbncx+xml")
    toc.setAttribute("properties", "nav") # Set the propernavmap to use this later)
    opfManifest.appendChild(toc)
    
    ids = 0
    for p in pagesList:
        item = opfFile.createElement("item")
        item.setAttribute("id", "img"+str(ids))
        ids +=1
        item.setAttribute("href", os.path.relpath(p, str(path)))
        item.setAttribute("media-type", "image/png")
        if os.path.basename(p) == os.path.basename(coverpageurl):
            item.setAttribute("properties", "cover-image")
        opfManifest.appendChild(item)


    ids = 0
    for p in htmlFiles:
        item = opfFile.createElement("item")
        item.setAttribute("id", "p"+str(ids))
        ids +=1
        item.setAttribute("href", os.path.relpath(p, str(path)))
        item.setAttribute("media-type", "application/xhtml+xml")
        opfManifest.appendChild(item)
    

    opfRoot.appendChild(opfManifest)
    
    # Spine

    opfSpine = opfFile.createElement("spine")
    # this sets the table of contents to use the ncx file
    opfSpine.setAttribute("toc", "ncx")
    # Reading Direction:

    spreadRight = True
    direction = 0
    if "readingDirection" in configDictionary.keys():
        if configDictionary["readingDirection"] is "rightToLeft":
            opfSpine.setAttribute("page-progression-direction", "rtl")
            spreadRight = False
            direction = 1
        else:
            opfSpine.setAttribute("page-progression-direction", "ltr")

    # Here we'd need to switch between the two and if spread keywrod use neither but combine with spread-none
    # TODO implement spread keyword.
    spread = False
    
    ids = 0
    for p in htmlFiles:
        item = opfFile.createElement("itemref")
        item.setAttribute("idref", "p"+str(ids))
        ids +=1
        props = []
        if spread:
            # Don't do a spread for this one.
            props.append("spread-none")
            
            # Reset the spread boolean.
            # It needs to point at the first side after the spread.
            # So ltr -> spread-left, rtl->spread-right
            if direction == 0:
                spreadRight = False
            else:
                spreadRight = True
        else:
            if spreadRight:
                props.append("page-spread-right")
                spreadRight = False
            else:
                props.append("page-spread-left")
                spreadRight = True
        item.setAttribute("properties", " ".join(props))
        opfSpine.appendChild(item)
    opfRoot.appendChild(opfSpine)

    # Guide
    
    opfGuide = opfFile.createElement("guide")
    if coverpagehtml is not None and coverpagehtml.isspace() is False and len(coverpagehtml) > 0:
        item = opfFile.createElement("reference")
        item.setAttribute("type", "cover")
        item.setAttribute("title", "Cover")
        item.setAttribute("href", coverpagehtml)
        opfGuide.appendChild(item)
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
