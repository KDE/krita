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
Write the Advanced Comic Book Data xml file.

http://acbf.wikia.com/wiki/ACBF_Specifications

"""

import os
from xml.dom import minidom
from PyQt5.QtCore import QDate, Qt, QPointF, QByteArray, QBuffer
from PyQt5.QtGui import QImage

def write_xml(configDictionary = {}, pageData = [],  pagesLocationList = [], locationBasic = str(), locationStandAlone = str()):
    acbfGenreList = ["science_fiction", "fantasy", "adventure", "horror", "mystery", "crime", "military", "real_life", "superhero", "humor", "western", "manga", "politics", "caricature", "sports", "history", "biography", "education", "computer", "religion", "romance", "children", "non-fiction", "adult", "alternative", "other", "artbook"]
    acbfAuthorRolesList = ["Writer", "Adapter", "Artist", "Penciller", "Inker", "Colorist", "Letterer", "Cover Artist", "Photographer", "Editor", "Assistant Editor", "Translator", "Other", "Designer"]
    document = minidom.Document()
    root = document.createElement("ACBF")
    root.setAttribute("xmlns", "http://www.fictionbook-lib.org/xml/acbf/1.0")
    document.appendChild(root)

    meta = document.createElement("meta-data")

    bookInfo = document.createElement("book-info")
    if "authorList" in configDictionary.keys():
        for authorE in range(len(configDictionary["authorList"])):
            author = document.createElement("author")
            authorDict = configDictionary["authorList"][authorE]
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
    if "title" in configDictionary.keys():
        bookTitle.appendChild(document.createTextNode(str(configDictionary["title"])))
    else:
        bookTitle.appendChild(document.createTextNode(str("Comic with no Name")))
    bookInfo.appendChild(bookTitle)
    extraGenres = []

    if "genre" in configDictionary.keys():
        for genre in configDictionary["genre"]:
            genreModified = str(genre).lower()
            genreModified.replace(" ", "_")
            if genreModified in acbfGenreList:
                bookGenre = document.createElement("genre")
                bookGenre.appendChild(document.createTextNode(str(genreModified)))
                bookInfo.appendChild(bookGenre)
            else:
                extraGenres.appendChild(genre)
    annotation = document.createElement("annotation")
    if "summary" in configDictionary.keys():
        paragraphList = str(configDictionary["summary"]).split("\n")
        for para in paragraphList:
            p = document.createElement("p")
            p.appendChild(document.createTextNode(str(para)))
            annotation.appendChild(p)
    else:
        p = document.createElement("p")
        p.appendChild(document.createTextNode(str("There was no summary upon generation of this file.")))
        annotation.appendChild(p)
    bookInfo.appendChild(annotation)

    if "characters" in configDictionary.keys():
        character = document.createElement("characters")
        for name in configDictionary["characters"]:
            char = document.createElement("name")
            char.appendChild(document.createTextNode(str(name)))
            character.appendChild(char)
        bookInfo.appendChild(character)

    keywords = document.createElement("keywords")
    stringKeywordsList = []
    for key in extraGenres:
        stringKeywordsList.append(str(key))
    if "otherKeywords" in configDictionary.keys():
        for key in configDictionary["otherKeywords"]:
            stringKeywordsList.append(str(key))
    if "format" in configDictionary.keys():
        for key in configDictionary["format"]:
            stringKeywordsList.append(str(key))
    keywords.appendChild(document.createTextNode(", ".join(stringKeywordsList)))
    bookInfo.appendChild(keywords)

    coverpageurl = ""
    coverpage = document.createElement("coverpage")
    if "pages" in configDictionary.keys():
        if "cover" in configDictionary.keys():
            pageList = []
            pageList = configDictionary["pages"]
            coverNumber = max([pageList.index(configDictionary["cover"]), 0])
            image = document.createElement("image")
            if len(pagesLocationList) >= coverNumber:
                coverpageurl = pagesLocationList[coverNumber]
                image.setAttribute("href", os.path.basename(coverpageurl))
            coverpage.appendChild(image)
    bookInfo.appendChild(coverpage)

    if "language" in configDictionary.keys():
        language = document.createElement("languages")
        textlayer = document.createElement("text-layer")
        textlayer.setAttribute("lang", configDictionary["language"])
        textlayer.setAttribute("show", "False")
        textlayerNative = document.createElement("text-layer")
        textlayerNative.setAttribute("lang", configDictionary["language"])
        textlayerNative.setAttribute("show", "True")
        language.appendChild(textlayer)
        language.appendChild(textlayerNative)
        bookInfo.appendChild(language)

        bookTitle.setAttribute("lang", configDictionary["language"])
        annotation.setAttribute("lang", configDictionary["language"])
        keywords.setAttribute("lang", configDictionary["language"])
    #database = document.createElement("databaseref")
    # bookInfo.appendChild(database)

    if "seriesName" in configDictionary.keys():
        sequence = document.createElement("sequence")
        sequence.setAttribute("title", configDictionary["seriesName"])
        if "seriesVolume" in configDictionary.keys():
            sequence.setAttribute("volume", str(configDictionary["seriesVolume"]))
        if "seriesNumber" in configDictionary.keys():
            sequence.appendChild(document.createTextNode(str(configDictionary["seriesNumber"])))
        else:
            sequence.appendChild(document.createTextNode(str(0)))
        bookInfo.appendChild(sequence)
    contentrating = document.createElement("content-rating")

    if "rating" in configDictionary.keys():
        contentrating.appendChild(document.createTextNode(str(configDictionary["rating"])))
    else:
        contentrating.appendChild(document.createTextNode(str("Unrated.")))
    if "ratingSystem" in configDictionary.keys():
        contentrating.setAttribute("type", configDictionary["ratingSystem"])
    bookInfo.appendChild(contentrating)

    if "readingDirection" in configDictionary.keys():
        readingDirection = document.createElement("reading-direction")
        if configDictionary["readingDirection"] is "rightToLeft":
            readingDirection.appendChild(document.createTextNode(str("RTL")))
        else:
            readingDirection.appendChild(document.createTextNode(str("LTR")))
        bookInfo.appendChild(readingDirection)
    meta.appendChild(bookInfo)

    publisherInfo = document.createElement("publish-info")
    if "publisherName" in configDictionary.keys():
        publisherName = document.createElement("publisher")
        publisherName.appendChild(document.createTextNode(str(configDictionary["publisherName"])))
        publisherInfo.appendChild(publisherName)
    if "publishingDate" in configDictionary.keys():
        publishingDate = document.createElement("publish-date")
        publishingDate.setAttribute("value", configDictionary["publishingDate"])
        publishingDate.appendChild(document.createTextNode(QDate.fromString(configDictionary["publishingDate"], Qt.ISODate).toString(Qt.SystemLocaleLongDate)))
        publisherInfo.appendChild(publishingDate)
    if "publisherCity" in configDictionary.keys():
        publishCity = document.createElement("city")
        publishCity.appendChild(document.createTextNode(str(configDictionary["publisherCity"])))
        publisherInfo.appendChild(publishCity)
    if "isbn-number" in configDictionary.keys():
        publishISBN = document.createElement("isbn")
        publishISBN.appendChild(document.createTextNode(str(configDictionary["isbn-number"])))
        publisherInfo.appendChild(publishISBN)
    if "license" in configDictionary.keys():
        license = configDictionary["license"]
        if license.isspace() is False and len(license) > 0:
            publishLicense = document.createElement("license")
            publishLicense.appendChild(document.createTextNode(str(configDictionary["license"])))
            publisherInfo.appendChild(publishLicense)

    meta.appendChild(publisherInfo)

    documentInfo = document.createElement("document-info")
    # TODO: ACBF apparantly uses first/middle/last/nick/email/homepage for the document auhtor too...
    #      The following code compensates for me not understanding this initially. This still needs
    #      adjustments in the gui.
    if "acbfAuthor" in configDictionary.keys():
        if isinstance(configDictionary["acbfAuthor"], list):
            for e in configDictionary["acbfAuthor"]:
                acbfAuthor = document.createElement("author")
                authorDict = configDictionary["acbfAuthor"][e]
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
            acbfAuthorNick.appendChild(document.createTextNode(str(configDictionary["acbfAuthor"])))
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

    if "acbfSource" in configDictionary.keys():
        acbfSource = document.createElement("source")
        acbfSourceP = document.createElement("p")
        acbfSourceP.appendChild(document.createTextNode(str(configDictionary["acbfSource"])))
        acbfSource.appendChild(acbfSourceP)
        documentInfo.appendChild(acbfSource)

    if "acbfID" in configDictionary.keys():
        acbfID = document.createElement("id")
        acbfID.appendChild(document.createTextNode(str(configDictionary["acbfID"])))
        documentInfo.appendChild(acbfID)

    if "acbfVersion" in configDictionary.keys():
        acbfVersion = document.createElement("version")
        acbfVersion.appendChild(document.createTextNode(str(configDictionary["acbfVersion"])))
        documentInfo.appendChild(acbfVersion)

    if "acbfHistory" in configDictionary.keys():
        acbfHistory = document.createElement("history")
        for h in configDictionary["acbfHistory"]:
            p = document.createElement("p")
            p.appendChild(document.createTextNode(str(h)))
            acbfHistory.appendChild(p)
        documentInfo.appendChild(acbfHistory)
    meta.appendChild(documentInfo)

    root.appendChild(meta)

    body = document.createElement("body")

    for p in range(0, len(pagesLocationList)):
        page = pagesLocationList[p]
        language = "en"
        if "language" in configDictionary.keys():
            language = configDictionary["language"]
        textLayer = document.createElement("text-layer")
        textLayer.setAttribute("lang", language)
        data = pageData[p]
        transform = data["transform"]
        frameList = []
        for v in data["vector"]:
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
            if "acbf_title" in data["keys"]:
                title = document.createElement("title")
                title.setAttribute("lang", language)
                title.appendChild(document.createTextNode(str(data["title"])))
                pg.appendChild(title)
            if "acbf_none" in data["keys"]:
                pg.setAttribute("transition", "none")
            if "acbf_blend" in data["keys"]:
                pg.setAttribute("transition", "blend")
            if "acbf_fade" in data["keys"]:
                pg.setAttribute("transition", "fade")
            if "acbf_horizontal" in data["keys"]:
                pg.setAttribute("transition", "scroll_right")
            if "acbf_vertical" in data["keys"]:
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

    f = open(locationBasic, 'w', newline="", encoding="utf-8")
    f.write(document.toprettyxml(indent="  "))
    f.close()
    success = True
    success = createStandAloneACBF(configDictionary, document, locationStandAlone, pagesLocationList)
    return success

def createStandAloneACBF(configDictionary, document, location, pagesLocationList = []):
    title = configDictionary["projectName"]
    if "title" in configDictionary.keys():
        title = configDictionary["title"]
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
        for p in pagesLocationList:
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
