"""
SPDX-FileCopyrightText: 2018 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

This file is part of the Comics Project Management Tools(CPMT).

SPDX-License-Identifier: GPL-3.0-or-later
"""

"""
A thing that parses through POT files.
"""

import sys
import os
import re

class po_file_parser():
    translationDict = {}
    translationList = []
    key_xml = False

    def __init__(self, translationLocation, key_xml = False):
        self.key_xml = key_xml
        if os.path.exists(translationLocation):
            for entry in os.scandir(translationLocation):
                if entry.name.endswith('.po') and entry.is_file():
                    self.parse_pot(os.path.join(translationLocation, entry.name))

    def parse_pot(self, location):
        if (os.path.exists(location)):
            file = open(location, "r", encoding="utf8")
            lang = "en"
            for line in file:
                if line.startswith("\"Language: "):
                    lang = line[len("\"Language: "):]
                    lang = lang.replace('\\n\"\n', "")
            file.close()
            file = open(location, "r", encoding="utf8")
            multiLine = ""
            key = None
            entry = {}
            
            def addEntryToTranslationDict(key, entry, lang):
                if len(entry.keys())>0:
                    if key is None:
                        key = ""
                    if self.key_xml:
                        text = entry.get("text", "")
                        text = re.sub("\<.*?\>", " ", text)
                        key += str(re.sub("\s+", " ", text)).strip()
                    else:
                        key += entry.get("text", None)
                    if key is not None:
                        if len(key)>0:
                            dummyDict = {}
                            dummyDict = self.translationDict.get(key, dummyDict)
                            dummyDict[lang] = entry
                            self.translationDict[key] = dummyDict
            
            for line in file:
                if line.isspace() or len(line)<1:
                    addEntryToTranslationDict(key, entry, lang)
                    entry = {}
                    key = None
                    multiLine = ""
                if line.startswith("msgid "):
                    string = line[len("msgid \""):]
                    string = string[:-len("\"\n")]
                    string = string.replace("\\\"", "\"")
                    string = string.replace("\\\'", "\'")
                    string = string.replace("\\#", "#")
                    entry["text"] = string
                    multiLine = "text"
                if line.startswith("msgstr "):
                    string = line[len("msgstr \""):]
                    string = string[:-len("\"\n")]
                    string = string.replace("\\\"", "\"")
                    string = string.replace("\\\'", "\'")
                    string = string.replace("\\#", "#")
                    entry["trans"] = string
                    multiLine = "trans"
                if line.startswith("# "):
                    #Translator comment
                    if "translComment" in entry.keys():
                        entry["translComment"] += line.replace("# ", "")
                    else:
                        entry["translComment"] = line.replace("# ", "")
                if line.startswith("#. "):
                    entry["extract"] = line.replace("#. ", "")
                if line.startswith("msgctxt "):
                    key = line[len("msgctxt \""):]
                    key = key[:-len("\"\n")]
                    key += " "
                if line.startswith("\"") and len(multiLine)>0:
                    string = line[len("\""):]
                    string = string[:-len("\"\n")]
                    string = string.replace("\\\"", "\"")
                    string = string.replace("\\\'", "\'")
                    string = string.replace("\\#", "#")
                    entry[multiLine] += string
            # ensure that the final entry gets added.
            addEntryToTranslationDict(key, entry, lang)
            if lang not in self.translationList:
                self.translationList.append(lang)
            file.close()

    def get_translation_list(self):
        return self.translationList
    
    def get_entry_for_key(self, key, lang):
        entry = {}
        entry["trans"] = " "
        if self.key_xml:
            key = re.sub("\<.*?\>", " ", key)
            key = re.sub("\s+", " ", key)
            key = key.strip()
        if key in self.translationDict.keys():
            translations = {}
            translations = self.translationDict[key]
            if lang not in translations.keys():
                print("language missing")
                return entry
            return translations[lang]
        else:
            print(str(key).encode("utf8"))
            print("translation missing from the translated strings")
            return entry
