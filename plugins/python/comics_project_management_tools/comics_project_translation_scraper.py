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
A class for getting translatable strings out.

This class does several things:
1) It can parse through kra files' document.xml, and then through the svgs that file is pointing at.
2) It can parse a preexisting POT file to ensure it isn't making duplicates.
3) It can write a POT file.
4) Writing to a csv file was considered until the realisaton hit that comic dialog itself contains commas.
"""

import sys
import os
import csv
import zipfile
import types
from xml.dom import minidom 

class translation_scraper():
    projectURL = str()
    translation_folder = str()
    textLayerNameList = []
    translationDict = {}
    projectName = str()
    languageKey = "AA_language"

    def __init__(self, projectURL = str(), translation_folder = str(), textLayerNameList = [], projectName = str()):
        self.projectURL = projectURL
        self.projectName = projectName
        self.translation_folder = translation_folder
        self.textLayerNameList = textLayerNameList
        self.translationDict = {}
        
        # Check for a preexisting translation file and parse that.
        for entry in os.scandir(os.path.join(self.projectURL, self.translation_folder)):
            if entry.name.endswith(projectName+'.pot') and entry.is_file():
                self.parse_pot(entry.name)
                break

    def start(self, pagesList, language):
        if self.languageKey not in self.translationDict.keys():
            self.translationDict[self.languageKey] = language
        for p in pagesList:
            self.get_svg_layers(os.path.join(self.projectURL, p))
        self.write_pot()

    def parse_pot(self, location):
        if (os.path.exists(location)):
            file = open(location, "r", newline="", encoding="utf8")
            potEntry = False
            key = ""
            entry = {}
            for line in file:
                if line.isspace():
                    if entry.keys()>0:
                        if key.isspace():
                            key = entry["text"]
                        self.translationDict[key] = entry
                    entry = {}
                if line.startswith("msgid "):
                    string = line.rstrip("msgid \"")
                    string = string[:-1]
                    string = string.replace("\\\"", "\"")
                    string = string.replace("\\\'", "\'")
                    string = string.replace("\\#", "#")
                    entry["text"] = string
                if line.startswith("# "):
                    #Translator comment
                    entry["translator"] = line
                if line.startswith("#. "):
                    entry["extract"] = line
                if line.startswith("msgctxt "):
                    key = line.strip("msgctxt ")
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
                    if childNode.tagName == "layer" and childNode.getAttribute("nodetype")=="shapelayer":
                        isTextLayer = False
                        for t in self.textLayerNameList:
                            if t in childNode.getAttribute("name"):
                                isTextLayer = True
                        if isTextLayer:
                            filename = childNode.getAttribute("filename")
                            for c in candidates:
                                if str(filename+".shapelayer/content.svg") in c:
                                    self.get_txt(page.read(c))
                    if childNode.childNodes:
                        parseThroughChildNodes(childNode)
                    
        parseThroughChildNodes(doc)
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
                    elif childNode.childNodes:
                        parseThroughChildNodes(childNode)
                    
        parseThroughChildNodes(svg.documentElement)

    def write_pot(self):
        quote = "\""
        newLine = "\n"
        location = os.path.join(self.projectURL, self.translation_folder, self.projectName+".pot")
        file = open(location, "w", newline="", encoding="utf8")
        for key in self.translationDict.keys():
            if key != self.languageKey:
                file.write(newLine)
                if "translator" in self.translationDict[key].keys():
                    file.write("# "+self.translationDict[key]["translator"]+newLine)
                if "extract" in self.translationDict[key].keys():
                    file.write("#. "+self.translationDict[key]["extract"]+newLine)
                string = self.translationDict[key]["text"]
                uniqueContext = False
                if string != key:
                    uniqueContext = True
                string = string.replace(quote, "\\\"")
                string = string.replace("\'", "\\\'")
                string = string.replace("#", "\\#")
                if uniqueContext:
                    file.write("msgctxt "+quote+key+quote+newLine)
                file.write("msgid "+quote+string+quote+newLine)
                file.write("msgstr "+quote+quote+newLine)
        file.close()
