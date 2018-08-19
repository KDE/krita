'''
A script that sorts the colors in the group.

By Wolthera(originally)

This script is licensed CC 0 1.0, so that you can learn from it.

------ CC 0 1.0 ---------------

The person who associated a work with this deed has dedicated the work to the public domain by waiving all of his or her rights to the work worldwide under copyright law, including all related and neighboring rights, to the extent allowed by law.

You can copy, modify, distribute and perform the work, even for commercial purposes, all without asking permission.

https://creativecommons.org/publicdomain/zero/1.0/legalcode

@package palette_docker
'''


# Importing the relevant dependencies:
from krita import *


class sortColors(object):

    def __init__(self, name):
        # We want people to select a palette...
        allPalettes = Application.resources("palette")
        self.paletteName = name
        self.currentPalette = Palette(allPalettes[self.paletteName])
        self.sort_all_groups()

    def sort_all_groups(self):
        self.sort_color_by_name(str())
        groupNames = self.currentPalette.groupNames()
        for groupName in groupNames:
            self.sort_color_by_name(groupName)

    def sort_color_by_name(self, groupName):
        l = {}
        colorCount = self.currentPalette.colorsCountGroup(groupName)
        for i in range(colorCount - 1, -1, -1):
            entry = self.currentPalette.colorSetEntryFromGroup((i), groupName)
            l[entry.name + str(i)] = entry
            self.currentPalette.removeEntry((i), groupName)

        for s in sorted(l):
            self.currentPalette.addEntry(l[s], groupName)

    def sort_color_by_id(self, groupName):
        l = {}
        colorCount = self.currentPalette.colorsCountGroup(groupName)
        for i in range(colorCount - 1, -1, -1):
            entry = self.currentPalette.colorSetEntryFromGroup((i), groupName)
            l[entry.id() + " " + str(i)] = entry
            self.currentPalette.removeEntry((i), groupName)

        for s in sorted(l):
            self.currentPalette.addEntry(l[s], groupName)

    def sort_by_value(self, groupName):
        l = {}
        colorCount = self.currentPalette.colorsCountGroup(groupName)
        for i in range(colorCount - 1, -1, -1):
            entry = self.currentPalette.colorSetEntryFromGroup((i), groupName)
            color = self.currentPalette.colorForEntry(entry)
            color.setColorSpace("RGBA", "U8", "sRGB built-in")
            l[color.components()[0] + color.components()[1] + color.components()[2]] = entry
            self.currentPalette.removeEntry((i), groupName)

        for s in sorted(l):
            self.currentPalette.addEntry(l[s], groupName)

    def sort_by_hue(self, stepsize, groupName):
        pass

    def palette(self):
        return self.currentPalette
