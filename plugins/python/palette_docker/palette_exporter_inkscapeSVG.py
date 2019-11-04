# A script that converts the palette named "Default" to a SVG so that
# Inkscape may use the colors

# The icc-color stuff doesn't work right, because we'd need the
# ability to get the url of the colorprofile somehow, and then we can
# make color-profile things in the definitions.

# By Wolthera(originally)

# This script is licensed CC 0 1.0, so that you can learn from it.

# ------ CC 0 1.0 ---------------

# The person who associated a work with this deed has dedicated the
# work to the public domain by waiving all of his or her rights to the
# work worldwide under copyright law, including all related and
# neighboring rights, to the extent allowed by law.

# You can copy, modify, distribute and perform the work, even for
# commercial purposes, all without asking permission.

# https://creativecommons.org/publicdomain/zero/1.0/legalcode

# @package palette_docker


# Importing the relevant dependencies:
from PyQt5.QtXml import QDomDocument
from PyQt5.QtWidgets import QFileDialog, QMessageBox
from krita import Palette


class inkscapeSVGExporter:

    def __init__(self, name):
        # We want people to select a palette and a location to save to...
        self.fileName = QFileDialog.getExistingDirectory()
        allPalettes = Application.resources("palette")
        self.paletteName = name
        self.currentPalette = Palette(allPalettes[self.paletteName])
        self.export()
        done = QMessageBox()
        done.setWindowTitle(i18n("Export Successful"))
        done.setText(
            str(i18n("{input} has been exported to {output}.")).format(
                input=self.paletteName, output=self.fileName))
        done.exec_()
        pass

    def export(self):
        # open the appropriate file...
        svgFile = open(self.fileName + "/" + self.paletteName + ".svg", "w")
        svgDoc = QDomDocument()
        svgBaseElement = svgDoc.createElement("svg")
        svgBaseElement.setAttribute(
            "xmlns:osb", "http://www.openswatchbook.org/uri/2009/osb")
        svgBaseElement.setAttribute(
            "xmlns:svg", "http://www.w3.org/2000/svg")
        svgBaseElement.setAttribute(
            "xmlns:dc", "http://purl.org/dc/elements/1.1/")
        svgBaseElement.setAttribute(
            "xmlns:cc", "http://creativecommons.org/ns#")
        svgBaseElement.setAttribute(
            "xmlns:rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#")
        svgDefs = svgDoc.createElement("defs")
        svgSwatches = svgDoc.createElement("g")
        svgSwatches.setAttribute("id", "Swatches")

        svgMeta = svgDoc.createElement("metadata")
        svgBaseElement.appendChild(svgMeta)
        rdf = svgDoc.createElement("rdf:RDF")
        ccwork = svgDoc.createElement("cc:Work")
        dctitle = svgDoc.createElement("dc:title")
        dcdescription = svgDoc.createElement("dc:description")
        dctitle.appendChild(svgDoc.createTextNode(self.paletteName))
        dcdescription.appendChild(svgDoc.createTextNode(
            self.currentPalette.comment()))
        ccwork.appendChild(dctitle)
        ccwork.appendChild(dcdescription)
        rdf.appendChild(ccwork)
        svgMeta.appendChild(rdf)
        Row = 0
        Column = 0
        iccProfileList = []

        colorCount = self.currentPalette.colorsCountGroup("")

        for i in range(colorCount):
            entry = self.currentPalette.colorSetEntryFromGroup(i, "")
            color = self.currentPalette.colorForEntry(entry)

            iccColor = "icc-color(" + color.colorProfile()
            for c in range(len(color.componentsOrdered()) - 1):
                iccColor = "{col},{c}".format(
                    col=iccColor, c=color.componentsOrdered()[c])
            iccColor = iccColor + ")"
            if color.colorProfile() not in iccProfileList:
                iccProfileList.append(color.colorProfile())

            # convert to sRGB
            color.setColorSpace("RGBA", "U8", "sRGB built-in")
            red = max(min(int(color.componentsOrdered()[0] * 255), 255), 0)
            green = max(min(int(color.componentsOrdered()[1] * 255), 255), 0)
            blue = max(min(int(color.componentsOrdered()[2] * 255), 255), 0)
            hexcode = "#{red:02x}{green:02x}{blue:02x}".format(
                red=red, green=green, blue=blue)
            swatchName = "{i}-{name}".format(i=i, name=entry.name())
            swatchName = swatchName.replace(" ", "-")
            swatchName = swatchName.replace("(", "-")
            swatchName = swatchName.replace(")", "-")
            swatchMain = svgDoc.createElement("linearGradient")
            swatchMain.setAttribute("osb:paint", "solid")
            swatchMain.setAttribute("id", swatchName)
            swatchSub = svgDoc.createElement("stop")
            swatchSub.setAttribute(
                "style",
                "stop-color: {hex} {color};stop-opacity:1;".format(
                    hex=hexcode, color=iccColor))
            swatchMain.appendChild(swatchSub)
            svgDefs.appendChild(swatchMain)
            svgSingleSwatch = svgDoc.createElement("rect")
            svgSingleSwatch.setAttribute("x", str(int(Column * 20)))
            svgSingleSwatch.setAttribute("y", str(int(Row * 20)))
            svgSingleSwatch.setAttribute("width", str(int(20)))
            svgSingleSwatch.setAttribute("height", str(int(20)))
            svgSingleSwatch.setAttribute("fill", "url(#%s)" % swatchName)
            svgSingleSwatch.setAttribute("id", "swatch %s" % swatchName)
            if entry.spotColor() is True:
                svgSingleSwatch.setAttribute("rx", str(10))
                svgSingleSwatch.setAttribute("ry", str(10))
            svgSwatches.appendChild(svgSingleSwatch)
            Column += 1
            if (Column >= self.currentPalette.columnCount()):
                Column = 0
                Row += 1

        groupNames = self.currentPalette.groupNames()
        for groupName in groupNames:
            Column = 0
            Row += 1
            groupTitle = svgDoc.createElement("text")
            groupTitle.setAttribute("x", str(int(Column * 20)))
            groupTitle.setAttribute("y", str(int(Row * 20) + 15))
            groupTitle.appendChild(svgDoc.createTextNode(groupName))
            svgSwatches.appendChild(groupTitle)
            Row += 1
            colorCount = self.currentPalette.colorsCountGroup(groupName)
            for i in range(colorCount):
                entry = self.currentPalette.colorSetEntryFromGroup(
                    i, groupName)
                color = self.currentPalette.colorForEntry(entry)
                iccColor = "icc-color(" + color.colorProfile()
                for c in range(len(color.componentsOrdered()) - 1):
                    iccColor = "{col},{c}".format(
                        col=iccColor, c=color.componentsOrdered()[c])
                iccColor = iccColor + ")"
                if color.colorProfile() not in iccProfileList:
                    iccProfileList.append(color.colorProfile())
                # convert to sRGB
                color.setColorSpace("RGBA", "U8", "sRGB built-in")
                red = max(
                    min(int(color.componentsOrdered()[0] * 255), 255), 0)
                green = max(
                    min(int(color.componentsOrdered()[1] * 255), 255), 0)
                blue = max(
                    min(int(color.componentsOrdered()[2] * 255), 255), 0)
                hexcode = "#{red:02x}{green:02x}{blue:02x}".format(
                    red=red, green=green, blue=blue)
                swatchName = groupName + str(i) + "-" + entry.name()
                swatchName = swatchName.replace(" ", "-")
                swatchName = swatchName.replace("(", "-")
                swatchName = swatchName.replace(")", "-")
                swatchMain = svgDoc.createElement("linearGradient")
                swatchMain.setAttribute("osb:paint", "solid")
                swatchMain.setAttribute("id", swatchName)
                swatchSub = svgDoc.createElement("stop")
                swatchSub.setAttribute(
                    "style",
                    "stop-color: {hex} {color};stop-opacity:1;".format(
                        hex=hexcode, color=iccColor))

                swatchMain.appendChild(swatchSub)
                svgDefs.appendChild(swatchMain)
                svgSingleSwatch = svgDoc.createElement("rect")
                svgSingleSwatch.setAttribute("x", str(int(Column * 20)))
                svgSingleSwatch.setAttribute("y", str(int(Row * 20)))
                svgSingleSwatch.setAttribute("width", str(int(20)))
                svgSingleSwatch.setAttribute("height", str(int(20)))
                svgSingleSwatch.setAttribute("fill", "url(#%s)" % swatchName)
                svgSingleSwatch.setAttribute("id", "swatch %s" % swatchName)
                if entry.spotColor() is True:
                    svgSingleSwatch.setAttribute("rx", str(10))
                    svgSingleSwatch.setAttribute("ry", str(10))
                svgSwatches.appendChild(svgSingleSwatch)
                Column += 1
                if (Column >= self.currentPalette.columnCount()):
                    Column = 0
                    Row += 1

        for profile in iccProfileList:
            svgProfileDesc = svgDoc.createElement("color-profile")
            svgProfileDesc.setAttribute("name", profile)
            # This is incomplete because python api doesn't have any
            # way to ask for this data yet.
            # svgProfileDesc.setAttribute("local", "sRGB")
            # svgProfileDesc.setAttribute("xlink:href", colorprofileurl)
            svgProfileDesc.setAttribute("rendering-intent", "perceptual")
            svgDefs.appendChild(svgProfileDesc)
        svgBaseElement.appendChild(svgDefs)
        svgBaseElement.appendChild(svgSwatches)
        svgBaseElement.setAttribute(
            "viewBox",
            "0 0 {cols} {row}".format(
                cols=self.currentPalette.columnCount() * 20,
                row=int((Row + 1) * 20)))
        svgDoc.appendChild(svgBaseElement)
        svgFile.write(svgDoc.toString())
        svgFile.close()
