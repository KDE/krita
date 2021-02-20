"""
SPDX-FileCopyrightText: 2018 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

This file is part of the Comics Project Management Tools(CPMT).

SPDX-License-Identifier: GPL-3.0-or-later
"""

"""
Write the Advanced Comic Book Data xml file.

http://acbf.wikia.com/wiki/ACBF_Specifications

"""

import os
import re
from PyQt5.QtCore import QDate, Qt, QPointF, QByteArray, QBuffer
from PyQt5.QtGui import QImage, QColor, QFont, QRawFont
from PyQt5.QtXml import QDomDocument, QDomElement, QDomText, QDomNodeList
from . import CPMT_po_parser as po_parser

def write_xml(configDictionary = {}, pageData = [],  pagesLocationList = [], locationBasic = str(), locationStandAlone = str(), projectUrl = str()):
    acbfGenreList = ["science_fiction", "fantasy", "adventure", "horror", "mystery", "crime", "military", "real_life", "superhero", "humor", "western", "manga", "politics", "caricature", "sports", "history", "biography", "education", "computer", "religion", "romance", "children", "non-fiction", "adult", "alternative", "other", "artbook"]
    acbfAuthorRolesList = ["Writer", "Adapter", "Artist", "Penciller", "Inker", "Colorist", "Letterer", "Cover Artist", "Photographer", "Editor", "Assistant Editor", "Translator", "Other", "Designer"]
    document = QDomDocument()
    root = document.createElement("ACBF")
    root.setAttribute("xmlns", "http://www.acbf.info/xml/acbf/1.1")
    document.appendChild(root)
    
    emphasisStyle = {}
    strongStyle = {}
    if "acbfStyles" in configDictionary.keys():
        stylesDictionary = configDictionary.get("acbfStyles", {})
        emphasisStyle = stylesDictionary.get("emphasis", {})
        strongStyle = stylesDictionary.get("strong", {})
        styleString = "\n"
        tabs = "    "
        for key in sorted(stylesDictionary.keys()):
            style = stylesDictionary.get(key, {})
            if key == "emphasis" or key == "strong":
                styleClass = key+" {\n"
            elif key == "speech":
                styleClass = "text-area {\n"
            elif key == "general":
                styleClass = "* {\n"
            elif key == "inverted":
                styleClass = "text-area[inverted=\"true\"] {\n"
            else:
                styleClass = "text-area[type=\""+key+"\"] {\n"
            styleString += tabs+styleClass
            if "color" in style.keys():
                styleString += tabs+tabs+"color:"+style["color"]+";\n"
            if "font" in style.keys():
                fonts = style["font"]
                genericfont = style.get("genericfont", "sans-serif")
                if isinstance(fonts, list):
                    styleString += tabs+tabs+"font-family:\""+str("\", \"").join(fonts)+"\", "+genericfont+";\n"
                else:
                    styleString += tabs+tabs+"font-family:\""+fonts+"\", "+genericfont+";\n"
            if "bold" in style.keys():
                if style["bold"]:
                    styleString += tabs+tabs+"font-weight: bold;\n"
            if "ital" in style.keys():
                if style["ital"]:
                    styleString += tabs+tabs+"font-style: italic;\n"
                else:
                    styleString += tabs+tabs+"font-style: normal;\n"
            styleString += tabs+"}\n"
        style = document.createElement("style")
        style.setAttribute("type", "text/css")
        style.appendChild(document.createTextNode(styleString))
        root.appendChild(style)
        

    meta = document.createElement("meta-data")
    
    translationFolder = configDictionary.get("translationLocation", "translations")
    fullTranslationPath = os.path.join(projectUrl, translationFolder)
    poParser = po_parser.po_file_parser(fullTranslationPath, True)

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
                author.setAttribute("lang", str(authorDict["language"]).replace("_", "-"))
            bookInfo.appendChild(author)
    bookTitle = document.createElement("book-title")
    if "title" in configDictionary.keys():
        bookTitle.appendChild(document.createTextNode(str(configDictionary["title"])))
    else:
        bookTitle.appendChild(document.createTextNode(str("Comic with no Name")))
    bookInfo.appendChild(bookTitle)
    extraGenres = []

    if "genre" in configDictionary.keys():
        genreListConf = configDictionary["genre"]
        if isinstance(configDictionary["genre"], dict):
            genreListConf = configDictionary["genre"].keys()
        for genre in genreListConf:
            genreModified = str(genre).lower()
            genreModified.replace(" ", "_")
            if genreModified in acbfGenreList:
                bookGenre = document.createElement("genre")
                bookGenre.appendChild(document.createTextNode(str(genreModified)))
                if isinstance(configDictionary["genre"], dict):
                    genreMatch = configDictionary["genre"][genreModified]
                    if genreMatch>0:
                        bookGenre.setAttribute("match", str(genreMatch))
                bookInfo.appendChild(bookGenre)
            else:
                extraGenres.append(genre)

    if "characters" in configDictionary.keys():
        character = document.createElement("characters")
        for name in configDictionary["characters"]:
            char = document.createElement("name")
            char.appendChild(document.createTextNode(str(name)))
            character.appendChild(char)
        bookInfo.appendChild(character)

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
        textlayer.setAttribute("lang", str(configDictionary["language"]).replace("_", "-"))
        textlayer.setAttribute("show", "false")
        textlayerNative = document.createElement("text-layer")
        textlayerNative.setAttribute("lang", str(configDictionary["language"]).replace("_", "-"))
        textlayerNative.setAttribute("show", "true")
        language.appendChild(textlayer)
        language.appendChild(textlayerNative)
        translationComments = {}
        for lang in poParser.get_translation_list():
            textlayer = document.createElement("text-layer")
            textlayer.setAttribute("lang", lang)
            textlayer.setAttribute("show", "true")
            language.appendChild(textlayer)
            translationComments[lang] = []
            translation = poParser.get_entry_for_key("@meta-title "+configDictionary["title"], lang).get("trans", None)
            if translation is not None:
                bookTitleTr = document.createElement("book-title")
                bookTitleTr.setAttribute("lang", lang)
                bookTitleTr.appendChild(document.createTextNode(translation))
                bookInfo.insertAfter(bookTitleTr, bookTitle)
            translation = poParser.get_entry_for_key("@meta-summary "+configDictionary["summary"], lang).get("trans", None)
            if translation is not None:
                annotationTr = document.createElement("annotation")
                annotationTr.setAttribute("lang", lang)
                paragraph = document.createElement("p")
                paragraph.appendChild(document.createTextNode(translation))
                annotationTr.appendChild(paragraph)
                bookInfo.insertAfter(annotationTr, annotation)
            translation = poParser.get_entry_for_key("@meta-keywords "+", ".join(configDictionary["otherKeywords"]), lang).get("trans", None)
            if translation is not None:
                keywordsTr = document.createElement("keywords")
                keywordsTr.setAttribute("lang", lang)
                keywordsTr.appendChild(document.createTextNode(translation))
                bookInfo.insertAfter(keywordsTr, keywords)
        bookInfo.appendChild(language)

        bookTitle.setAttribute("lang", str(configDictionary["language"]).replace("_", "-"))
        annotation.setAttribute("lang", str(configDictionary["language"]).replace("_", "-"))
        keywords.setAttribute("lang", str(configDictionary["language"]).replace("_", "-"))
        
    if "databaseReference" in configDictionary.keys():
        database = document.createElement("databaseref")
        dbRef = configDictionary["databaseReference"]
        database.setAttribute("dbname", dbRef.get("name", ""))
        if "type" in dbRef.keys():
            database.setAttribute("type", dbRef["type"])
        database.appendChild(document.createTextNode(dbRef.get("entry", "")))
        bookInfo.appendChild(database)

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
        if configDictionary["readingDirection"] == "rightToLeft":
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
    license = str(configDictionary.get("license", ""))
    if license.isspace() is False and len(license) > 0:
        publishLicense = document.createElement("license")
        publishLicense.appendChild(document.createTextNode(license))
        publisherInfo.appendChild(publishLicense)

    meta.appendChild(publisherInfo)

    documentInfo = document.createElement("document-info")
    # TODO: ACBF apparently uses first/middle/last/nick/email/homepage for the document author too...
    #      The following code compensates for me not understanding this initially.
    if "acbfAuthor" in configDictionary.keys():
        if isinstance(configDictionary["acbfAuthor"], list):
            for e in configDictionary["acbfAuthor"]:
                acbfAuthor = document.createElement("author")
                authorDict = e
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
                    authorN = document.createElement("home-page")
                    authorN.appendChild(document.createTextNode(str(authorDict["homepage"])))
                    acbfAuthor.appendChild(authorN)
                if "email" in authorDict.keys():
                    authorN = document.createElement("email")
                    authorN.appendChild(document.createTextNode(str(authorDict["email"])))
                    acbfAuthor.appendChild(authorN)
                if "language" in authorDict.keys():
                    acbfAuthor.setAttribute("lang", str(authorDict["language"]).replace("_", "-"))
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
        if len(configDictionary["acbfHistory"])>0:
            acbfHistory = document.createElement("history")
            for h in configDictionary["acbfHistory"]:
                p = document.createElement("p")
                p.appendChild(document.createTextNode(str(h)))
                acbfHistory.appendChild(p)
            documentInfo.appendChild(acbfHistory)
    meta.appendChild(documentInfo)

    root.appendChild(meta)

    body = document.createElement("body")
    
    references = document.createElement("references")
    
    def figure_out_type(svg = QDomElement()):
        type = None
        skipList = ["speech", "emphasis", "strong", "inverted", "general"]
        if svg.attribute("text-anchor") == "middle" or svg.attribute("text-align") == "center":
            if "acbfStyles" in configDictionary.keys():
                stylesDictionary = configDictionary.get("acbfStyles", {})
                for key in stylesDictionary.keys():
                    if key not in skipList:
                        style = stylesDictionary.get(key, {})
                        font = style.get("font", "")
                        if isinstance(fonts, list):
                            if svg.attribute("family") in font:
                                type = key
                        elif svg.attribute("family") == font:
                            type = key
            else:
                type = None
        elif svg.attribute("text-align") == "justified":
            type = "formal"
        else:
            type = "commentary"
        inverted = None
        #Figure out whether this is inverted colored text.
        if svg.hasAttribute("fill"):
            stylesDictionary = configDictionary.get("acbfStyles", {})
            key = stylesDictionary.get("general", {})
            regular = QColor(key.get("color", "#000000"))
            key = stylesDictionary.get("inverted", {})
            invertedColor = QColor(key.get("color", "#FFFFFF"))
            textColor = QColor(svg.attribute("fill"))
            # Proceed to get luma for the three colors.
            lightnessR = (0.21 * regular.redF()) + (0.72 * regular.greenF()) + (0.07 * regular.blueF())
            lightnessI = (0.21 * invertedColor.redF()) + (0.72 * invertedColor.greenF()) + (0.07 * invertedColor.blueF())
            lightnessT = (0.21 * textColor.redF()) + (0.72 * textColor.greenF()) + (0.07 * textColor.blueF())
            if lightnessI > lightnessR:
                if lightnessT > (lightnessI+lightnessR)*0.5:
                    inverted = "true"
            else:
                if lightnessT < (lightnessI+lightnessR)*0.5:
                    inverted = "true"
        return [type, inverted]
    
    listOfPageColors = []
    
    for p in range(0, len(pagesLocationList)):
        page = pagesLocationList[p]
        imageFile = QImage()
        imageFile.load(page)
        imageRect = imageFile.rect().adjusted(0, 0, -1, -1)
        pageColor = findDominantColor([imageFile.pixelColor(imageRect.topLeft()), imageFile.pixelColor(imageRect.topRight()), imageFile.pixelColor(imageRect.bottomRight()), imageFile.pixelColor(imageRect.bottomLeft())])
        listOfPageColors.append(pageColor)
        language = "en"
        if "language" in configDictionary.keys():
            language = str(configDictionary["language"]).replace("_", "-")
        textLayer = document.createElement("text-layer")
        textLayer.setAttribute("lang", language)
        data = pageData[p]
        transform = data["transform"]
        frameList = []
        listOfTextColors = []
        for v in data["vector"]:
            boundingBoxText = []
            listOfBoundaryColors = []
            for point in v["boundingBox"]:
                offset = QPointF(transform["offsetX"], transform["offsetY"])
                pixelPoint = QPointF(point.x() * transform["resDiff"], point.y() * transform["resDiff"])
                newPoint = pixelPoint - offset
                x = max(0, min(imageRect.width(), int(newPoint.x() * transform["scaleWidth"])))
                y = max(0, min(imageRect.height(), int(newPoint.y() * transform["scaleHeight"])))
                listOfBoundaryColors.append(imageFile.pixelColor(x, y))
                pointText = str(x) + "," + str(y)
                boundingBoxText.append(pointText)
            mainColor = findDominantColor(listOfBoundaryColors)

            if "text" in v.keys():
                textArea = document.createElement("text-area")
                textArea.setAttribute("points", " ".join(boundingBoxText))
                # TODO: Rotate will require proper global transform api as transform info is not written intotext.                        #textArea.setAttribute("text-rotation", str(v["rotate"]))
                svg = QDomDocument()
                svg.setContent(v["text"])
                figureOut = figure_out_type(svg.documentElement())
                type = figureOut[0]
                inverted = figureOut[1]
                paragraph = QDomDocument()
                paragraph.appendChild(paragraph.createElement("p"))
                parseTextChildren(paragraph, svg.documentElement(), paragraph.documentElement(), emphasisStyle, strongStyle) 
                textArea.appendChild(paragraph.documentElement())
                textArea.setAttribute("bgcolor", mainColor.name())
                if type is not None:
                    textArea.setAttribute("type", type)
                if inverted is not None:
                    textArea.setAttribute("inverted", inverted)
                textLayer.appendChild(textArea)
            else:
                f = {}
                f["points"] = " ".join(boundingBoxText)
                frameList.append(f)
            listOfTextColors.append(mainColor)
        textLayer.setAttribute("bgcolor", findDominantColor(listOfTextColors).name())
        textLayerList = document.createElement("trlist")
        for lang in poParser.get_translation_list():
            textLayerTr = document.createElement("text-layer")
            textLayerTr.setAttribute("lang", lang)
            for i in range(len(data["vector"])):
                d = data["vector"]
                v = d[i]
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
                    svg = QDomDocument()
                    svg.setContent(v["text"])
                    figureOut = figure_out_type(svg.documentElement())
                    type = figureOut[0]
                    inverted = figureOut[1]
                    string = re.sub("\<\/*?text.*?\>",'', str(v["text"]))
                    string = re.sub("\s+?", " ", string)
                    translationEntry = poParser.get_entry_for_key(string, lang)
                    string = translationEntry.get("trans", string)
                    svg.setContent("<text>"+string+"</text>")
                    paragraph = QDomDocument()
                    paragraph.appendChild(paragraph.createElement("p"))
                    parseTextChildren(paragraph, svg.documentElement(), paragraph.documentElement(), emphasisStyle, strongStyle)
                    if "translComment" in translationEntry.keys():
                        key = translationEntry["translComment"]
                        listOfComments = []
                        listOfComments = translationComments[lang]
                        index = 0
                        if key in listOfComments:
                            index = listOfComments.index(key)+1
                        else:
                            listOfComments.append(key)
                            index = len(listOfComments)
                            translationComments[lang] = listOfComments
                        refID = "-".join(["tn", lang, str(index)])
                        anchor = document.createElement("a")
                        anchor.setAttribute("href", "#"+refID)
                        anchor.appendChild(document.createTextNode("*"))
                        paragraph.documentElement().appendChild(anchor)
                    textArea.appendChild(paragraph.documentElement())
                    textLayerTr.appendChild(textArea)
                    if type is not None:
                        textArea.setAttribute("type", type)
                    if inverted is not None:
                        textArea.setAttribute("inverted", inverted)
                    textArea.setAttribute("bgcolor", listOfTextColors[i].name())
                if textLayerTr.hasChildNodes():
                    textLayerTr.setAttribute("bgcolor", findDominantColor(listOfTextColors).name())
                    textLayerList.appendChild(textLayerTr)
                    
        

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
                for lang in poParser.get_translation_list():
                    titleTrans = " "
                    titlekey = "@page-title "+str(data["title"])
                    translationEntry = poParser.get_entry_for_key(titlekey, lang)
                    titleTrans = translationEntry.get("trans", titleTrans)
                    if titleTrans.isspace() is False:
                        titleT = document.createElement("title")
                        titleT.setAttribute("lang", lang)
                        titleT.appendChild(document.createTextNode(titleTrans))
                        pg.appendChild(titleT)
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
            if textLayer.hasChildNodes():
                pg.appendChild(textLayer)
            pg.setAttribute("bgcolor", pageColor.name())
            for n in range(0, textLayerList.childNodes().size()):
                node = textLayerList.childNodes().at(n)
                pg.appendChild(node)
            for f in frameList:
                frame = document.createElement("frame")
                frame.setAttribute("points", f["points"])
                pg.appendChild(frame)
            body.appendChild(pg)
        else:
            for f in frameList:
                frame = document.createElement("frame")
                frame.setAttribute("points", f["points"])
                coverpage.appendChild(frame)
            coverpage.appendChild(textLayer)
            for n in range(0, textLayerList.childNodes().size()):
                node = textLayerList.childNodes().at(n)
                coverpage.appendChild(node)
    bodyColor = findDominantColor(listOfPageColors)
    body.setAttribute("bgcolor", bodyColor.name())

    if configDictionary.get("includeTranslComment", False):
        for lang in translationComments.keys():
            for key in translationComments[lang]:
                index = translationComments[lang].index(key)+1
                refID = "-".join(["tn", lang, str(index)])
                ref = document.createElement("reference")
                ref.setAttribute("lang", lang)
                ref.setAttribute("id", refID)
                transHeaderStr = configDictionary.get("translatorHeader", "Translator's Notes")
                transHeaderStr = poParser.get_entry_for_key("@meta-translator "+transHeaderStr, lang).get("trans", transHeaderStr)
                translatorHeader = document.createElement("p")
                translatorHeader.appendChild(document.createTextNode(transHeaderStr+":"))
                ref.appendChild(translatorHeader)
                refPara = document.createElement("p")
                refPara.appendChild(document.createTextNode(key))
                ref.appendChild(refPara)
                references.appendChild(ref)

    root.appendChild(body)
    if references.childNodes().size():
        root.appendChild(references)

    f = open(locationBasic, 'w', newline="", encoding="utf-8")
    f.write(document.toString(indent=2))
    f.close()
    success = True
    success = createStandAloneACBF(configDictionary, document, locationStandAlone, pagesLocationList)
    return success

def createStandAloneACBF(configDictionary, document = QDomDocument(), location = str(), pagesLocationList = []):
    title = configDictionary["projectName"]
    if "title" in configDictionary.keys():
        title = configDictionary["title"]
    root = document.firstChildElement("ACBF")
    meta = root.firstChildElement("meta-data")
    bookInfo = meta.firstChildElement("book-info")
    cover = bookInfo.firstChildElement("coverpage")

    body = root.firstChildElement("body")
    pages = []
    for p in range(0, len(body.elementsByTagName("page"))):
        pages.append(body.elementsByTagName("page").item(p).toElement())
    if (cover):
        pages.append(cover)

    data = document.createElement("data")
    root.appendChild(data)

    # Convert pages to base64 strings.
    for i in range(0, len(pages)):
        image = pages[i].firstChildElement("image")
        href = image.attribute("href")
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

    f = open(location, 'w', newline="", encoding="utf-8")
    f.write(document.toString(indent=2))
    f.close()
    return True

"""
Function to parse svg text to acbf ready text
"""

def parseTextChildren(document = QDomDocument(), elRead = QDomElement(), elWrite = QDomElement(), emphasisStyle = {}, strongStyle = {}):
    for n in range(0, elRead.childNodes().size()):
        childNode = elRead.childNodes().item(n)
        if childNode.isText():
            if elWrite.hasChildNodes() and str(childNode.nodeValue()).startswith(" ") is False:
                elWrite.appendChild(document.createTextNode(" "))
            elWrite.appendChild(document.createTextNode(str(childNode.nodeValue())))
        elif childNode.hasChildNodes():
            childNode = childNode.toElement()
            fontFamily = str(childNode.attribute("font-family"))
            fontWeight = str(childNode.attribute("font-weight", "400"))
            fontItalic = str(childNode.attribute("font-style"))
            fontStrikeThrough = str(childNode.attribute("text-decoration"))
            fontBaseLine = str(childNode.attribute("baseline-shift"))
            newElementMade = False

            emphasis = False
            strong = False
            if len(emphasisStyle.keys()) > 0:
                emphasis = compare_styles(emphasisStyle, fontFamily, fontWeight, fontItalic)
            else:
                if fontItalic == "italic":
                    emphasis = True
            if len(strongStyle.keys()) > 0:
                strong = compare_styles(strongStyle, fontFamily, fontWeight, fontItalic)
            else:
                if fontWeight == "bold" or int(fontWeight) > 400:
                    strong = True

            if strong:
                    newElement = document.createElement("strong")
                    newElementMade = True
            elif emphasis:
                    newElement = document.createElement("emphasis")
                    newElementMade = True
            elif fontStrikeThrough == "line-through":
                    newElement = document.createElement("strikethrough")
                    newElementMade = True
            elif fontBaseLine.isalnum():
                if (fontBaseLine == "super"):
                    newElement = document.createElement("sup")
                    newElementMade = True
                elif (fontBaseLine == "sub"):
                    newElement = document.createElement("sub")
                    newElementMade = True

            if newElementMade is True:
                parseTextChildren(document, childNode, newElement, emphasisStyle, strongStyle)
                elWrite.appendChild(newElement)
            else:
                parseTextChildren(document, childNode, elWrite, emphasisStyle, strongStyle)

        # If it is not a text node, nor does it have children(which could be textnodes),
        # we should assume it's empty and ignore it.
    elWrite.normalize()
    for e in range(0, elWrite.childNodes().size()):
        el = elWrite.childNodes().item(e)
        if el.isText():
            eb = el.nodeValue()
            el.setNodeValue(eb.replace("  ", " "))
            
def compare_styles(style = {}, fontFamily = str(), fontWeight = str(), fontStyle = str()):
    compare = []
    if "font" in style.keys():
        font = style.get("font")
        if isinstance(font, list):
            compare.append(fontFamily in font)
        else:
            compare.append((fontFamily == font))
    if "bold" in style.keys():
        compare.append(fontWeight == "bold" or int(fontWeight) > 400)
    if "ital" in style.keys():
        compare.append(fontStyle == "italic")
    countTrue = 0
    for i in compare:
        if i is True:
            countTrue +=1
    if countTrue > 1:
        return True
    else:
        return False
"""
This function tries to determine if there's a dominant color,
and if not, it'll mix all of them.
"""

def findDominantColor(listOfColors = [QColor()]):
    dominantColor = QColor()
    listOfColorNames = {}
    
    for color in listOfColors:
        count = listOfColorNames.get(color.name(), 0)
        listOfColorNames[color.name()] = count+1
    
    # Check if there's a sense of dominant color:
    clear_dominant = False
    
    if len(listOfColorNames) == 2 and len(listOfColors) == 1:
        clear_dominant = True
    elif len(listOfColorNames) == 3 and len(listOfColors) == 2:
        clear_dominant = True
    elif len(listOfColorNames.keys()) < (len(listOfColors)*0.5):
        clear_dominant = True
    
    if clear_dominant:
        namesSorted = sorted(listOfColorNames, key=listOfColorNames.get, reverse=True)
        dominantColor = QColor(namesSorted[0])
    else:
        for color in listOfColors:
            dominantColor.setRedF(0.5*(dominantColor.redF()+color.redF()))
            dominantColor.setGreenF(0.5*(dominantColor.greenF()+color.greenF()))
            dominantColor.setBlueF(0.5*(dominantColor.blueF()+color.blueF()))
    
    return dominantColor
