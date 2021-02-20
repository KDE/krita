"""
SPDX-FileCopyrightText: 2018 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

This file is part of the Comics Project Management Tools(CPMT).

SPDX-License-Identifier: GPL-3.0-or-later
"""

"""
The comicrack information is sorta... incomplete, so no idea if the following is right...
I can't check in any case: It is a windows application.

Based off:

https://github.com/dickloraine/EmbedComicMetadata/blob/master/comicinfoxml.py

ComicRack is also a dead application.

Missing:

Count (issues)
AlternateSeries
AlternateNumber
StoryArc
SeriesGroup
AlternateCount
Notes
Imprint
Locations
ScanInformation
AgeRating - Not sure if this should be added or not...
Teams
Web

"""

from xml.dom import minidom
from PyQt5.QtCore import QDate, Qt

def write_xml(configDictionary = {}, pagesLocationList = [],  location = str()):
    document = minidom.Document()
    root = document.createElement("ComicInfo")
    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance")
    root.setAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema")

    title = document.createElement("Title")
    if "title" in configDictionary.keys():
        title.appendChild(document.createTextNode(str(configDictionary["title"])))
    else:
        title.appendChild(document.createTextNode(str("Untitled Comic")))
    root.appendChild(title)
    description = document.createElement("Summary")
    if "summary" in configDictionary.keys():
        description.appendChild(document.createTextNode(str(configDictionary["summary"])))
    else:
        description.appendChild(document.createTextNode(str("There was no summary upon generation of this file.")))
    root.appendChild(description)
    
    if "seriesNumber" in configDictionary.keys():
        number = document.createElement("Number")
        number.appendChild(document.createTextNode(str(configDictionary["seriesNumber"])))
        root.appendChild(number)
    if "seriesName" in configDictionary.keys():
        seriesname = document.createElement("Series")
        seriesname.appendChild(document.createTextNode(str(configDictionary["seriesName"])))
        root.appendChild(seriesname)

    if "publishingDate" in configDictionary.keys():
        date = QDate.fromString(configDictionary["publishingDate"], Qt.ISODate)
        publishYear = document.createElement("Year")
        publishYear.appendChild(document.createTextNode(str(date.year())))
        publishMonth = document.createElement("Month")
        publishMonth.appendChild(document.createTextNode(str(date.month())))
        publishDay = document.createElement("Day")
        publishDay.appendChild(document.createTextNode(str(date.day())))
        root.appendChild(publishYear)
        root.appendChild(publishMonth)
        root.appendChild(publishDay)

    if "format" in configDictionary.keys():
        for form in configDictionary["format"]:
            formattag = document.createElement("Format")
            formattag.appendChild(document.createTextNode(str(form)))
            root.appendChild(formattag)
    if "otherKeywords" in configDictionary.keys():
        tags = document.createElement("Tags")
        tags.appendChild(document.createTextNode(str(", ".join(configDictionary["otherKeywords"]))))
        root.appendChild(tags)

    if "authorList" in configDictionary.keys():
        for authorE in range(len(configDictionary["authorList"])):
            author = document.createElement("Writer")
            authorDict = configDictionary["authorList"][authorE]
            if "role" in authorDict.keys():
                if str(authorDict["role"]).lower() in ["writer", "penciller", "editor", "assistant editor", "cover artist", "letterer", "inker", "colorist"]:
                    if str(authorDict["role"]).lower() == "cover artist":
                        author = document.createElement("CoverArtist")
                    elif str(authorDict["role"]).lower() == "assistant editor":
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
    if "publisherName" in configDictionary.keys():
        publisher = document.createElement("Publisher")
        publisher.appendChild(document.createTextNode(str(configDictionary["publisherName"])))
        root.appendChild(publisher)

    if "genre" in configDictionary.keys():
        genreListConf = configDictionary["genre"]
        if isinstance(configDictionary["genre"], dict):
            genreListConf = configDictionary["genre"].keys()
        for genreE in genreListConf:
            genre = document.createElement("Genre")
            genre.appendChild(document.createTextNode(str(genreE)))
            root.appendChild(genre)
    blackAndWhite = document.createElement("BlackAndWhite")
    blackAndWhite.appendChild(document.createTextNode(str("No")))
    root.appendChild(blackAndWhite)
    readingDirection = document.createElement("Manga")
    readingDirection.appendChild(document.createTextNode(str("No")))
    if "readingDirection" in configDictionary.keys():
        if configDictionary["readingDirection"] == "rightToLeft":
            readingDirection.appendChild(document.createTextNode(str("YesAndRightToLeft")))
    root.appendChild(readingDirection)

    if "characters" in configDictionary.keys():
        for char in configDictionary["characters"]:
            character = document.createElement("Character")
            character.appendChild(document.createTextNode(str(char)))
            root.appendChild(character)
    if "pages" in configDictionary.keys():
        pagecount = document.createElement("PageCount")
        pagecount.appendChild(document.createTextNode(str(len(configDictionary["pages"]))))
        root.appendChild(pagecount)
    pages = document.createElement("Pages")
    covernumber = 0
    if "pages" in configDictionary.keys() and "cover" in configDictionary.keys():
        covernumber = configDictionary["pages"].index(configDictionary["cover"])
    for i in range(len(pagesLocationList)):
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
    return True
