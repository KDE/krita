"""
SPDX-FileCopyrightText: 2018 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

This file is part of the Comics Project Management Tools(CPMT).

SPDX-License-Identifier: GPL-3.0-or-later
"""

"""
A class for getting translatable strings out.

This class does several things:
1) It can parse through kra files' document.xml, and then through the svgs that file is pointing at.
2) It can parse a preexisting POT file to ensure it isn't making duplicates.
3) It can write a POT file.
4) Writing to a csv file was considered until the realisation hit that comic dialog itself contains commas.
"""

import sys
import os
import csv
import zipfile
import types
from xml.dom import minidom
from PyQt5.QtCore import QDateTime, Qt


class translation_scraper():
    projectURL = str()
    translation_folder = str()
    textLayerNameList = []
    translationDict = {}
    translationKeys = []  # separate so that the keys will be somewhat according to the order of appearance.
    pageTitleKeys= []
    projectName = str()
    languageKey = "AA_language"

    def __init__(self, projectURL=str(), translation_folder=str(), textLayerNameList=[], projectName=str()):
        self.projectURL = projectURL
        self.projectName = projectName
        self.translation_folder = translation_folder
        self.textLayerNameList = textLayerNameList
        self.translationDict = {}
        self.pageTitleKeys = []

        # Check for a preexisting translation file and parse that.
        for entry in os.scandir(os.path.join(self.projectURL, self.translation_folder)):
            if entry.name.endswith(projectName + '.pot') and entry.is_file():
                self.parse_pot(os.path.join(self.projectURL, self.translation_folder, entry.name))
                break

    def start(self, pagesList, language, metaData={}):
        if self.languageKey not in self.translationDict.keys():
            self.translationDict[self.languageKey] = language
        for p in pagesList:
            self.get_svg_layers(os.path.join(self.projectURL, p))
        self.write_pot(metaData)

    def parse_pot(self, location):
        if (os.path.exists(location)):
            file = open(location, "r", newline="", encoding="utf8")
            multiLine = ""
            key = None
            entry = {}

            def addEntryToTranslationDict(key, entry):
                if len(entry.keys()) > 0:
                    if key is None:
                        key = entry.get("text", None)
                    if key is not None:
                        if len(key) > 0:
                            self.translationDict[key] = entry

            for line in file or len(line) < 1:
                if line.isspace():
                    addEntryToTranslationDict(key, entry)
                    entry = {}
                    key = None
                    multiLine = ""
                if line.startswith("msgid "):
                    string = line.strip("msgid \"")
                    string = string[:-len('"\n')]
                    string = string.replace("\\\"", "\"")
                    string = string.replace("\\\'", "\'")
                    string = string.replace("\\#", "#")
                    entry["text"] = string
                    multiLine = "text"
                if line.startswith("msgstr "):
                    string = line.strip("msgstr \"")
                    string = string[:-len('"\n')]
                    string = string.replace("\\\"", "\"")
                    string = string.replace("\\\'", "\'")
                    string = string.replace("\\#", "#")
                    entry["trans"] = string
                    multiLine = "trans"
                if line.startswith("# "):
                    # Translator comment
                    entry["translator"] = line
                if line.startswith("#. "):
                    entry["extract"] = line
                if line.startswith("msgctxt "):
                    string = line.strip("msgctxt \"")
                    string = string[:-len('"\n')]
                    string = string.replace("\\\"", "\"")
                    string = string.replace("\\\'", "\'")
                    string = string.replace("\\#", "#")
                    key = string
                if line.startswith("\"") and len(multiLine) > 0:
                    string = line[1:]
                    string = string[:-len('"\n')]
                    string = string.replace("\\\"", "\"")
                    string = string.replace("\\\'", "\'")
                    string = string.replace("\\#", "#")
                    entry[multiLine] += string
            addEntryToTranslationDict(key, entry)
            file.close()

    def get_svg_layers(self, location):
        page = zipfile.ZipFile(location, "a")
        xmlroot = minidom.parseString(page.read("maindoc.xml"))
        doc = xmlroot.documentElement

        candidates = []

        for member in page.namelist():
            info = page.getinfo(member)
            if info.filename.endswith('svg'):
                candidates.append(info.filename)

        def parseThroughChildNodes(node):
            for childNode in node.childNodes:
                if childNode.nodeType != minidom.Node.TEXT_NODE:
                    if childNode.tagName == "layer" and childNode.getAttribute("nodetype") == "shapelayer":
                        isTextLayer = False
                        for t in self.textLayerNameList:
                            if t in childNode.getAttribute("name"):
                                isTextLayer = True
                        if isTextLayer:
                            filename = childNode.getAttribute("filename")
                            for c in candidates:
                                if str(filename + ".shapelayer/content.svg") in c:
                                    self.get_txt(page.read(c))
                    if childNode.childNodes:
                        parseThroughChildNodes(childNode)

        parseThroughChildNodes(doc)
        
        # Get page title if the keywords contain acbf_title
        xmlroot = minidom.parseString(page.read("documentinfo.xml"))
        dict = {}
        def parseThroughDocumentInfo(node, dict):
            for childNode in node.childNodes:
                if childNode.nodeType != minidom.Node.TEXT_NODE and childNode.nodeType != minidom.Node.CDATA_SECTION_NODE:
                    if childNode.tagName == "title":
                        title = ""
                        for text in childNode.childNodes:
                            title += text.data
                        dict["title"] = title
                    elif childNode.tagName == "keyword":
                        k = ""
                        for text in childNode.childNodes:
                            k += text.data
                        keywords = k.split(",")
                        for i in range(len(keywords)):
                            keywords[i] = str(keywords[i]).strip()
                        dict["key"] = keywords
                    if childNode.childNodes:
                        parseThroughDocumentInfo(childNode, dict)
                        
        parseThroughDocumentInfo(xmlroot.documentElement, dict)
        keywords = dict["key"]
        if "acbf_title" in keywords:
            self.pageTitleKeys.append(dict["title"])
                    
        page.close()

    def get_txt(self, string):
        svg = minidom.parseString(string)
        # parse through string as if svg.

        def parseThroughChildNodes(node):
            for childNode in node.childNodes:
                if childNode.nodeType != minidom.Node.TEXT_NODE:
                    if childNode.tagName == "text":
                        text = ""
                        for c in childNode.childNodes:
                            text += c.toxml()
                        if text not in self.translationDict.keys():
                            entry = {}
                            entry["text"] = text
                            self.translationDict[text] = entry
                        if text not in self.translationKeys:
                            self.translationKeys.append(text)
                    elif childNode.childNodes:
                        parseThroughChildNodes(childNode)

        parseThroughChildNodes(svg.documentElement)

    def write_pot(self, metaData):
        quote = "\""
        newLine = "\n"
        location = os.path.join(self.projectURL, self.translation_folder, self.projectName + ".pot")
        file = open(location, "w", newline="", encoding="utf8")

        file.write("msgid " + quote + quote + newLine)
        file.write("msgstr " + quote + quote + newLine)
        date = QDateTime.currentDateTimeUtc().toString(Qt.ISODate)
        file.write(quote + "POT-Creation-Date:" + date + "\\n" + quote + newLine)
        file.write(quote + "Content-Type: text/plain; charset=UTF-8\\n" + quote + newLine)
        file.write(quote + "Content-Transfer-Encoding: 8bit\\n" + quote + newLine)
        file.write(quote + "X-Generator: Krita Comics Project Manager Tools Plugin\\n" + quote + newLine)

        file.write(newLine)
        file.write("#. Title of the work" + newLine)
        file.write("msgctxt \"@meta-title\"" + newLine)
        file.write("msgid " + quote + metaData.get("title", "") + quote + newLine)
        file.write("msgstr " + quote + quote + newLine)
        file.write(newLine)

        file.write("#. The summary" + newLine)
        file.write("msgctxt \"@meta-summary\"" + newLine)
        file.write("msgid " + quote + metaData.get("summary", "") + quote + newLine)
        file.write("msgstr " + quote + quote + newLine)
        file.write(newLine)

        file.write("#. The keywords, these need to be comma separated." + newLine)
        file.write("msgctxt \"@meta-keywords\"" + newLine)
        file.write("msgid " + quote + metaData.get("keywords", "") + quote + newLine)
        file.write("msgstr " + quote + quote + newLine)
        file.write(newLine)

        file.write("#. The header that will prepend translator's notes" + newLine)
        file.write("msgctxt \"@meta-translator\"" + newLine)
        file.write("msgid " + quote + metaData.get("transnotes", "") + quote + newLine)
        file.write("msgstr " + quote + quote + newLine)
        
        for i in range(len(self.pageTitleKeys)):
            title = self.pageTitleKeys[i]
            file.write(newLine)
            file.write("msgctxt " + quote + "@page-title" + quote + newLine)
            file.write("msgid " + quote + title + quote + newLine)
            file.write("msgstr " + quote + quote + newLine)

        for key in self.translationKeys:
            if key != self.languageKey:
                file.write(newLine)
                if "translComment" in self.translationDict[key].keys():
                    file.write("# " + self.translationDict[key]["translator"] + newLine)
                if "extract" in self.translationDict[key].keys():
                    file.write("#. " + self.translationDict[key]["extract"] + newLine)
                string = self.translationDict[key]["text"]
                uniqueContext = False
                if string != key:
                    uniqueContext = True
                string = string.replace(quote, "\\\"")
                string = string.replace("\'", "\\\'")
                string = string.replace("#", "\\#")
                if uniqueContext:
                    file.write("msgctxt " + quote + key + quote + newLine)
                file.write("msgid " + quote + string + quote + newLine)
                file.write("msgstr " + quote + quote + newLine)
        file.close()
        print("CPMT: Translations have been written to:", location)
