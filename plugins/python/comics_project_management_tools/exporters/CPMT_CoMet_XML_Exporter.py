"""
SPDX-FileCopyrightText: 2018 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

This file is part of the Comics Project Management Tools(CPMT).

SPDX-License-Identifier: GPL-3.0-or-later
"""

"""
Write a CoMet xml file to url
"""

import os
from xml.dom import minidom

def write_xml(configDictionary = {}, pagesLocationList = [], location = str()):
    document = minidom.Document()
    root = document.createElement("comet")
    root.setAttribute("xmlns:comet", "http://www.denvog.com/comet/")
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")
    root.setAttribute("xsi:schemaLocation", "http://www.denvog.com http://www.denvog.com/comet/comet.xsd")
    document.appendChild(root)

    title = document.createElement("title")
    if "title" in configDictionary.keys():
        title.appendChild(document.createTextNode(str(configDictionary["title"])))
    else:
        title.appendChild(document.createTextNode(str("Untitled Comic")))
    root.appendChild(title)
    description = document.createElement("description")
    if "summary" in configDictionary.keys():
        description.appendChild(document.createTextNode(str(configDictionary["summary"])))
    else:
        description.appendChild(document.createTextNode(str("There was no summary upon generation of this file.")))
    root.appendChild(description)
    if "seriesName" in configDictionary.keys():
        series = document.createElement("series")
        series.appendChild(document.createTextNode(str(configDictionary["seriesName"])))
        root.appendChild(series)
        if "seriesNumber" in configDictionary.keys():
            issue = document.createElement("issue")
            issue.appendChild(document.createTextNode(str(configDictionary["seriesNumber"])))
            root.appendChild(issue)
        if "seriesVolume" in configDictionary.keys():
            volume = document.createElement("volume")
            volume.appendChild(document.createTextNode(str(configDictionary["seriesVolume"])))
            root.appendChild(volume)

    if "publisherName" in configDictionary.keys():
        publisher = document.createElement("publisher")
        publisher.appendChild(document.createTextNode(str(configDictionary["publisherName"])))
        root.appendChild(publisher)

    if "publishingDate" in configDictionary.keys():
        date = document.createElement("date")
        date.appendChild(document.createTextNode(str(configDictionary["publishingDate"])))
        root.appendChild(date)

    if "genre" in configDictionary.keys():
        genreListConf = configDictionary["genre"]
        if isinstance(configDictionary["genre"], dict):
            genreListConf = configDictionary["genre"].keys()
        for genreE in genreListConf:
            genre = document.createElement("genre")
            genre.appendChild(document.createTextNode(str(genreE)))
            root.appendChild(genre)

    if "characters" in configDictionary.keys():
        for char in configDictionary["characters"]:
            character = document.createElement("character")
            character.appendChild(document.createTextNode(str(char)))
            root.appendChild(character)

    if "format" in configDictionary.keys():
        format = document.createElement("format")
        format.appendChild(document.createTextNode(str(",".join(configDictionary["format"]))))
        root.appendChild(format)

    if "language" in configDictionary.keys():
        language = document.createElement("language")
        language.appendChild(document.createTextNode(str(configDictionary["language"])))
        root.appendChild(language)
    if "rating" in configDictionary.keys():
        rating = document.createElement("rating")
        rating.appendChild(document.createTextNode(str(configDictionary["rating"])))
        root.appendChild(rating)
    #rights = document.createElement("rights")
    if "pages" in configDictionary.keys():
        pages = document.createElement("pages")
        pages.appendChild(document.createTextNode(str(len(configDictionary["pages"]))))
        root.appendChild(pages)

    if "isbn-number" in configDictionary.keys():
        identifier = document.createElement("identifier")
        identifier.appendChild(document.createTextNode(str(configDictionary["isbn-number"])))
        root.appendChild(identifier)

    if "authorList" in configDictionary.keys():
        for authorE in range(len(configDictionary["authorList"])):
            author = document.createElement("creator")
            authorDict = configDictionary["authorList"][authorE]
            if "role" in authorDict.keys():
                if str(authorDict["role"]).lower() in ["writer", "penciller", "editor", "assistant editor", "cover artist", "letterer", "inker", "colorist"]:
                    if str(authorDict["role"]).lower() == "cover artist":
                        author = document.createElement("coverDesigner")
                    elif str(authorDict["role"]).lower() == "assistant editor":
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

    if "pages" in configDictionary.keys():
        if "cover" in configDictionary.keys():
            pageList = []
            pageList = configDictionary["pages"]
            coverNumber = pageList.index(configDictionary["cover"])
            if len(pagesLocationList) >= coverNumber:
                coverImage = document.createElement("coverImage")
                coverImage.appendChild(document.createTextNode(str(os.path.basename(pagesLocationList[coverNumber]))))
                root.appendChild(coverImage)
    readingDirection = document.createElement("readingDirection")
    readingDirection.appendChild(document.createTextNode(str("ltr")))
    if "readingDirection" in configDictionary.keys():
        if configDictionary["readingDirection"] == "rightToLeft":
            readingDirection.appendChild(document.createTextNode(str("rtl")))
    root.appendChild(readingDirection)

    f = open(location, 'w', newline="", encoding="utf-8")
    f.write(document.toprettyxml(indent="  "))
    f.close()
    return True
