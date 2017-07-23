'''
A script that converts the palette named "Default" to a SVG so that Inkscape may use the colors
This ideally needs some gui and the like to select the palette to export.. also, the icc-color stuff doesn't work right
ecause we'd need the ability to get the url of the colorprofile somehow, and then we can make @colorprofile things in the definitions.
By Wolthera.
'''


# Importing the relevant dependancies:
import sys
from PyQt5.QtGui import *
from PyQt5.QtXml import *
from PyQt5.QtWidgets import *
import math
from krita import *

allPalettes = Application.resources("palette")
paletteName = "Default"
self.currentPalette = Palette(allPalettes[paletteName])
# open the appropriate file...
svgFile = open(paletteName+".svg", "w")
svgDoc = QDomDocument()
svgBaseElement = svgDoc.createElement("svg")
svgBaseElement.setAttribute("xmlns:osb", "http://www.openswatchbook.org/uri/2009/osb")
svgBaseElement.setAttribute("xmlns:svg", "http://www.w3.org/2000/svg")
svgDefs = svgDoc.createElement("defs")
svgSwatches = svgDoc.createElement("g")
svgSwatches.setAttribute("id", "Swatches")
Row = 0
Column = 0

colorCount = self.currentPalette.colorsCountGroup("")
        
for i in range(colorCount):
    entry = self.currentPalette.colorSetEntryFromGroup(i, "")
    color = self.currentPalette.colorForEntry(entry)

    iccColor = "icc-color("+color.colorProfile()
    for c in range(len(color.componentsOrdered())-1):
        iccColor = iccColor+","+str(color.componentsOrdered()[c])
    iccColor = iccColor+")"
    #convert to sRGB
    color.setColorSpace("RGBA", "U8", "sRGB built-in")
    red   = max(min(int(color.componentsOrdered()[0]*255), 255), 0)
    green = max(min(int(color.componentsOrdered()[1]*255), 255), 0)
    blue  = max(min(int(color.componentsOrdered()[2]*255), 255), 0)
    hexcode = "#"+str(format(red, '02x'))+str(format(green, '02x'))+str(format(blue, '02x'))
    swatchName = str(i)+"-"+entry.name
    swatchName = swatchName.replace(" ", "-")
    swatchMain = svgDoc.createElement("linearGradient")
    swatchMain.setAttribute("osb:paint", "solid")
    swatchMain.setAttribute("id", swatchName)
    swatchSub = svgDoc.createElement("stop")
    swatchSub.setAttribute("style", "stop-color: "+hexcode+" "+iccColor+";stop-opacity:1;")
    swatchMain.appendChild(swatchSub)
    svgDefs.appendChild(swatchMain)
    svgSingleSwatch = svgDoc.createElement("rect")
    svgSingleSwatch.setAttribute("x", str(int(Column*20)))
    svgSingleSwatch.setAttribute("y", str(int(Row*20)))
    svgSingleSwatch.setAttribute("width", str(int(20)))
    svgSingleSwatch.setAttribute("height", str(int(20)))
    svgSingleSwatch.setAttribute("fill", "url(#"+swatchName+")")
    svgSingleSwatch.setAttribute("id", "swatch"+swatchName)
    svgSwatches.appendChild(svgSingleSwatch)
    Column += 1
    if (Column >= self.currentPalette.columnCount()):
        Column = 0
        Row +=1
    
groupNames = self.currentPalette.groupNames()
for groupName in groupNames:
    Column=0
    Row+=1
    groupTitle = svgDoc.createElement("text")
    groupTitle.setAttribute("x", str(int(Column*20)))
    groupTitle.setAttribute("y", str(int(Row*20)+15))
    groupTitle.appendChild(svgDoc.createTextNode(groupName))
    svgSwatches.appendChild(groupTitle)
    Row+=1
    colorCount = self.currentPalette.colorsCountGroup(groupName)
    for i in range(colorCount):
        entry = self.currentPalette.colorSetEntryFromGroup(i, groupName)
        color = self.currentPalette.colorForEntry(entry)
        iccColor = "icc-color("+color.colorProfile()
        for c in range(len(color.componentsOrdered())-1):
            iccColor = iccColor+","+str(color.componentsOrdered()[c])
        iccColor = iccColor+")"
        #convert to sRGB
        color.setColorSpace("RGBA", "U8", "sRGB built-in")
        red   = max(min(int(color.componentsOrdered()[0]*255), 255), 0)
        green = max(min(int(color.componentsOrdered()[1]*255), 255), 0)
        blue  = max(min(int(color.componentsOrdered()[2]*255), 255), 0)
        hexcode = "#"+str(format(red, '02x'))+str(format(green, '02x'))+str(format(blue, '02x'))
    
        swatchName = groupName+str(i)+"-"+entry.name
        swatchName = swatchName.replace(" ", "-")
        swatchMain = svgDoc.createElement("linearGradient")
        swatchMain.setAttribute("osb:paint", "solid")
        swatchMain.setAttribute("id", swatchName)
        swatchSub = svgDoc.createElement("stop")
        swatchSub.setAttribute("style", "stop-color: "+hexcode+" "+iccColor+";stop-opacity:1;")
        swatchMain.appendChild(swatchSub)
        svgDefs.appendChild(swatchMain)
        svgSingleSwatch = svgDoc.createElement("rect")
        svgSingleSwatch.setAttribute("x", str(int(Column*20)))
        svgSingleSwatch.setAttribute("y", str(int(Row*20)))
        svgSingleSwatch.setAttribute("width", str(int(20)))
        svgSingleSwatch.setAttribute("height", str(int(20)))
        svgSingleSwatch.setAttribute("fill", "url(#"+swatchName+")")
        svgSingleSwatch.setAttribute("id", "swatch"+swatchName)
        svgSwatches.appendChild(svgSingleSwatch)
        Column += 1
        if (Column >= self.currentPalette.columnCount()):
            Column = 0
            Row +=1

svgBaseElement.appendChild(svgDefs)
svgBaseElement.appendChild(svgSwatches)
svgBaseElement.setAttribute("viewBox", "0 0 "+str(self.currentPalette.columnCount()*20)+" "+str(int(Row*20)))
svgDoc.appendChild(svgBaseElement)
svgFile.write(svgDoc.toString())
svgFile.close()


