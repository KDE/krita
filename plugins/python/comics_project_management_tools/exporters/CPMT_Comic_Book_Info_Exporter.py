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
Another metadata format but then a json dump stored into the zipfile comment.
The logic here being that the zipfile information can be read quicker.
Doesn't seem to be supported much. :/ (except by comicbooklovers, which is dead...
https://code.google.com/archive/p/comicbookinfo/wikis/Example.wiki

https://docs.google.com/document/pub?id=1Tu9eoPWc_8SPgxx5J4-6mEaaRWLLv-bEA8i_jcIe3IE

Missing:

numberOfIssues
numberOfVolumes
rating (1-5)
country
"""

import json
from PyQt5.QtCore import QDateTime, QDate, Qt, QLocale

def writeJson(configDictionary = {}):
    basedata = {}
    metadata = {}
    authorList = []
    taglist = []
    listOfRoles = ["Writer", "Inker", "Creator", "Editor", "Cartoonist", "Colorist", "Letterer", "Penciller", "Painter", "Cover", "Artist"]

    if "authorList" in configDictionary.keys():
        for authorE in range(len(configDictionary["authorList"])):
            author = {}

            authorDict = configDictionary["authorList"][authorE]
            stringName = []
            if "last-name" in authorDict.keys():
                stringName.append(authorDict["last-name"])
            if "first-name" in authorDict.keys():
                stringName.append(authorDict["first-name"])
            if "nickname" in authorDict.keys():
                stringName.append("(" + authorDict["nickname"] + ")")
            author["person"] = ",".join(stringName)
            if "role" in authorDict.keys():
                role = str(authorDict["role"]).title()
                if "editor" in role.lower():
                    role = "Editor"
                if "cover" in role.lower():
                    role = "Cover"
                if role in listOfRoles:
                    author["role"] = role
            authorList.append(author)

    if "characters" in configDictionary.keys():
        for character in configDictionary["characters"]:
            taglist.append(character)
    if "format" in configDictionary.keys():
        for item in configDictionary["format"]:
            taglist.append(item)
    if "otherKeywords" in configDictionary.keys():
        for item in configDictionary["otherKeywords"]:
            taglist.append(item)

    if "seriesName" in configDictionary.keys():
        metadata["series"] = configDictionary["seriesName"]
    if "title" in configDictionary.keys():
        metadata["title"] = configDictionary["title"]
    else:
        metadata["title"] = "Unnamed comic"
    if "publisherName" in configDictionary.keys():
        metadata["publisher"] = configDictionary["publisherName"]
    if "publishingDate" in configDictionary.keys():
        date = QDate.fromString(configDictionary["publishingDate"], Qt.ISODate)
        metadata["publicationMonth"] = date.month()
        metadata["publicationYear"] = date.year()
    if "seriesNumber" in configDictionary.keys():
        metadata["issue"] = configDictionary["seriesNumber"]
    if "seriesVolume" in configDictionary.keys():
        metadata["volume"] = configDictionary["seriesVolume"]
    if "genre" in configDictionary.keys():
        if isinstance(configDictionary["genre"], dict):
            listKeys = []
            for key in configDictionary["genre"].keys():
                listKeys.append(key)
            metadata["genre"] = listKeys
        else:
            metadata["genre"] = configDictionary["genre"]
    if "language" in configDictionary.keys():
        metadata["language"] = QLocale.languageToString(QLocale(configDictionary["language"]).language())

    metadata["credits"] = authorList

    metadata["tags"] = taglist
    if "summary" in configDictionary.keys():
        metadata["comments"] = configDictionary["summary"]
    else:
        metadata["comments"] = "File generated without summary"

    # 

    basedata["appID"] = "Krita"
    basedata["lastModified"] = QDateTime.currentDateTimeUtc().toString(Qt.ISODate)
    basedata["ComicBookInfo/1.0"] = metadata
    

    return json.dumps(basedata)
