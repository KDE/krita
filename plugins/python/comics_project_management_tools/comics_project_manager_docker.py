"""
SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

This file is part of the Comics Project Management Tools(CPMT).

SPDX-License-Identifier: GPL-3.0-or-later
"""

"""
This is a docker that helps you organise your comics project.
"""
import sys
import json
import os
import zipfile  # quick reading of documents
import shutil
import enum
from math import floor
import xml.etree.ElementTree as ET
from PyQt5.QtCore import QElapsedTimer, QSize, Qt, QRect, QFileSystemWatcher, QTimer
from PyQt5.QtGui import QStandardItem, QStandardItemModel, QImage, QIcon, QPixmap, QFontMetrics, QPainter, QPalette, QFont
from PyQt5.QtWidgets import QHBoxLayout, QVBoxLayout, QListView, QToolButton, QMenu, QAction, QPushButton, QSpacerItem, QSizePolicy, QWidget, QAbstractItemView, QProgressDialog, QDialog, QFileDialog, QDialogButtonBox, qApp, QSplitter, QSlider, QLabel, QStyledItemDelegate, QStyle, QMessageBox
import math
from krita import *
from . import comics_metadata_dialog, comics_exporter, comics_export_dialog, comics_project_setup_wizard, comics_template_dialog, comics_project_settings_dialog, comics_project_page_viewer, comics_project_translation_scraper

"""
A very simple class so we can have a label that is single line, but doesn't force the
widget size to be bigger.
This is used by the project name.
"""


class Elided_Text_Label(QLabel):
    mainText = str()

    def __init__(self, parent=None):
        super(QLabel, self).__init__(parent)
        self.setMinimumWidth(self.fontMetrics().width("..."))
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Minimum)

    def setMainText(self, text=str()):
        self.mainText = text
        self.elideText()

    def elideText(self):
        self.setText(self.fontMetrics().elidedText(self.mainText, Qt.ElideRight, self.width()))

    def resizeEvent(self, event):
        self.elideText()

class CPE(enum.IntEnum):
    TITLE = Qt.DisplayRole
    URL = Qt.UserRole + 1
    KEYWORDS = Qt.UserRole+2
    DESCRIPTION = Qt.UserRole+3
    LASTEDIT = Qt.UserRole+4
    EDITOR = Qt.UserRole+5
    IMAGE = Qt.DecorationRole

class comic_page_delegate(QStyledItemDelegate):

    def __init__(self, devicePixelRatioF, parent=None):
        super(QStyledItemDelegate, self).__init__(parent)
        self.devicePixelRatioF = devicePixelRatioF

    def paint(self, painter, option, index):
        
        if (index.isValid() == False):
            return
        painter.save()
        painter.setOpacity(0.6)
        if(option.state & QStyle.State_Selected):
            painter.fillRect(option.rect, option.palette.highlight())
        if (option.state & QStyle.State_MouseOver):
            painter.setOpacity(0.25)
            painter.fillRect(option.rect, option.palette.highlight())
        painter.setOpacity(1.0)
        painter.setFont(option.font)
        metrics = QFontMetrics(option.font)
        regular = QFont(option.font)
        italics = QFont(option.font)
        italics.setItalic(True)
        icon = QIcon(index.data(CPE.IMAGE))
        rect = option.rect
        margin = 4
        decoratonSize = QSize(option.decorationSize)
        imageSize = icon.actualSize(option.decorationSize)
        imageSizeHighDPI = imageSize*self.devicePixelRatioF
        leftSideThumbnail = (decoratonSize.width()-imageSize.width())/2
        if (rect.width() < decoratonSize.width()):
            leftSideThumbnail = max(0, (rect.width()-imageSize.width())/2)
        topSizeThumbnail = ((rect.height()-imageSize.height())/2)+rect.top()
        thumbImage = icon.pixmap(imageSizeHighDPI).toImage()
        thumbImage.setDevicePixelRatio(self.devicePixelRatioF)
        painter.drawImage(QRect(int(leftSideThumbnail), int(topSizeThumbnail), int(imageSize.width()), int(imageSize.height())), thumbImage)
        
        labelWidth = rect.width()-decoratonSize.width()-(margin*3)
        
        if (decoratonSize.width()+(margin*2)< rect.width()):
        
            textRect = QRect(decoratonSize.width()+margin, margin+rect.top(), labelWidth, metrics.height())
            textTitle = metrics.elidedText(str(index.row()+1)+". "+index.data(CPE.TITLE), Qt.ElideRight, labelWidth)
            painter.drawText(textRect, Qt.TextWordWrap, textTitle)
            
            if rect.height()/(metrics.lineSpacing()+margin) > 5 or index.data(CPE.KEYWORDS) is not None:
                painter.setOpacity(0.6)
                textRect = QRect(textRect.left(), textRect.bottom()+margin, labelWidth, metrics.height())
                if textRect.bottom() < rect.bottom():
                    textKeyWords = index.data(CPE.KEYWORDS)
                    if textKeyWords == None:
                        textKeyWords = i18n("No keywords")
                        painter.setOpacity(0.3)
                        painter.setFont(italics)
                    textKeyWords = metrics.elidedText(textKeyWords, Qt.ElideRight, labelWidth)
                    painter.drawText(textRect, Qt.TextWordWrap, textKeyWords)
            
            painter.setFont(regular)
            
            if rect.height()/(metrics.lineSpacing()+margin) > 3:
                painter.setOpacity(0.6)
                textRect = QRect(textRect.left(), textRect.bottom()+margin, labelWidth, metrics.height())
                if textRect.bottom()+metrics.height() < rect.bottom():
                    textLastEdit = index.data(CPE.LASTEDIT)
                    if textLastEdit is None:
                        textLastEdit = i18n("No last edit timestamp")
                    if index.data(CPE.EDITOR) is not None:
                        textLastEdit += " - " + index.data(CPE.EDITOR)
                    if (index.data(CPE.LASTEDIT) is None) and (index.data(CPE.EDITOR) is None):
                        painter.setOpacity(0.3)
                        painter.setFont(italics)
                    textLastEdit = metrics.elidedText(textLastEdit, Qt.ElideRight, labelWidth)
                    painter.drawText(textRect, Qt.TextWordWrap, textLastEdit)
            
            painter.setFont(regular)
            
            descRect = QRect(textRect.left(), textRect.bottom()+margin, labelWidth, (rect.bottom()-margin) - (textRect.bottom()+margin))
            if textRect.bottom()+metrics.height() < rect.bottom():
                textRect.setBottom(int(textRect.bottom()+(margin/2)))
                textRect.setLeft(int(textRect.left()-(margin/2)))
                painter.setOpacity(0.4)
                painter.drawLine(textRect.bottomLeft(), textRect.bottomRight())
                painter.setOpacity(1.0)
                textDescription = index.data(CPE.DESCRIPTION)
                if textDescription is None:
                    textDescription = i18n("No description")
                    painter.setOpacity(0.3)
                    painter.setFont(italics)
                linesTotal = floor(descRect.height()/metrics.lineSpacing())
                if linesTotal == 1:
                    textDescription = metrics.elidedText(textDescription, Qt.ElideRight, labelWidth)
                    painter.drawText(descRect, Qt.TextWordWrap, textDescription)
                else:
                    descRect.setHeight(linesTotal*metrics.lineSpacing())
                    totalDescHeight = metrics.boundingRect(descRect, Qt.TextWordWrap, textDescription).height()
                    if totalDescHeight>descRect.height():
                        if totalDescHeight-metrics.lineSpacing()>descRect.height():
                            painter.setOpacity(0.5)
                            painter.drawText(descRect, Qt.TextWordWrap, textDescription)
                            descRect.setHeight((linesTotal-1)*metrics.lineSpacing())
                            painter.drawText(descRect, Qt.TextWordWrap, textDescription)
                            descRect.setHeight((linesTotal-2)*metrics.lineSpacing())
                            painter.drawText(descRect, Qt.TextWordWrap, textDescription)
                        else:
                            painter.setOpacity(0.75)
                            painter.drawText(descRect, Qt.TextWordWrap, textDescription)
                            descRect.setHeight((linesTotal-1)*metrics.lineSpacing())
                            painter.drawText(descRect, Qt.TextWordWrap, textDescription)
                    else:
                        painter.drawText(descRect, Qt.TextWordWrap, textDescription)
            
            painter.setFont(regular)
        
        painter.restore()


"""
This is a Krita docker called 'Comics Manager'.

It allows people to create comics project files, load those files, add pages, remove pages, move pages, manage the metadata,
and finally export the result.

The logic behind this docker is that it is very easy to get lost in a comics project due to the massive amount of files.
By having a docker that gives the user quick access to the pages and also allows them to do all of the meta-stuff, like
meta data, but also reordering the pages, the chaos of managing the project should take up less time, and more time can be focused on actual writing and drawing.
"""


class comics_project_manager_docker(DockWidget):
    setupDictionary = {}
    stringName = i18n("Comics Manager")
    projecturl = None
    pagesWatcher = None
    updateurls = []

    def __init__(self):
        super().__init__()
        self.setWindowTitle(self.stringName)
        self.setProperty("ShowOnWelcomePage", True);

        # Setup layout:
        base = QHBoxLayout()
        widget = QWidget()
        widget.setLayout(base)
        baseLayout = QSplitter()
        base.addWidget(baseLayout)
        self.setWidget(widget)
        buttonLayout = QVBoxLayout()
        buttonBox = QWidget()
        buttonBox.setLayout(buttonLayout)
        baseLayout.addWidget(buttonBox)

        # Comic page list and pages model
        self.comicPageList = QListView()
        self.comicPageList.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.comicPageList.setDragEnabled(True)
        self.comicPageList.setDragDropMode(QAbstractItemView.InternalMove)
        self.comicPageList.setDefaultDropAction(Qt.MoveAction)
        self.comicPageList.setAcceptDrops(True)
        self.comicPageList.setItemDelegate(comic_page_delegate(self.devicePixelRatioF()))
        self.pagesModel = QStandardItemModel()
        self.comicPageList.doubleClicked.connect(self.slot_open_page)
        self.comicPageList.setIconSize(QSize(128, 128))
        # self.comicPageList.itemDelegate().closeEditor.connect(self.slot_write_description)
        self.pagesModel.layoutChanged.connect(self.slot_write_config)
        self.pagesModel.rowsInserted.connect(self.slot_write_config)
        self.pagesModel.rowsRemoved.connect(self.slot_write_config)
        self.pagesModel.rowsMoved.connect(self.slot_write_config)
        self.comicPageList.setModel(self.pagesModel)
        pageBox = QWidget()
        pageBox.setLayout(QVBoxLayout())
        zoomSlider = QSlider(Qt.Horizontal, None)
        zoomSlider.setRange(1, 8)
        zoomSlider.setValue(4)
        zoomSlider.setTickInterval(1)
        zoomSlider.setMinimumWidth(10)
        zoomSlider.valueChanged.connect(self.slot_scale_thumbnails)
        self.projectName = Elided_Text_Label()
        pageBox.layout().addWidget(self.projectName)
        pageBox.layout().addWidget(zoomSlider)
        pageBox.layout().addWidget(self.comicPageList)
        baseLayout.addWidget(pageBox)

        self.btn_project = QToolButton()
        self.btn_project.setPopupMode(QToolButton.MenuButtonPopup)
        self.btn_project.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Minimum)
        menu_project = QMenu()
        self.action_new_project = QAction(i18n("New Project"), self)
        self.action_new_project.triggered.connect(self.slot_new_project)
        self.action_load_project = QAction(i18n("Open Project"), self)
        self.action_load_project.triggered.connect(self.slot_open_config)
        menu_project.addAction(self.action_new_project)
        menu_project.addAction(self.action_load_project)
        self.btn_project.setMenu(menu_project)
        self.btn_project.setDefaultAction(self.action_load_project)
        buttonLayout.addWidget(self.btn_project)

        # Settings dropdown with actions for the different settings menus.
        self.btn_settings = QToolButton()
        self.btn_settings.setPopupMode(QToolButton.MenuButtonPopup)
        self.btn_settings.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Minimum)
        self.action_edit_project_settings = QAction(i18n("Project Settings"), self)
        self.action_edit_project_settings.triggered.connect(self.slot_edit_project_settings)
        self.action_edit_meta_data = QAction(i18n("Meta Data"), self)
        self.action_edit_meta_data.triggered.connect(self.slot_edit_meta_data)
        self.action_edit_export_settings = QAction(i18n("Export Settings"), self)
        self.action_edit_export_settings.triggered.connect(self.slot_edit_export_settings)
        menu_settings = QMenu()
        menu_settings.addAction(self.action_edit_project_settings)
        menu_settings.addAction(self.action_edit_meta_data)
        menu_settings.addAction(self.action_edit_export_settings)
        self.btn_settings.setDefaultAction(self.action_edit_project_settings)
        self.btn_settings.setMenu(menu_settings)
        buttonLayout.addWidget(self.btn_settings)
        self.btn_settings.setDisabled(True)

        # Add page drop down with different page actions.
        self.btn_add_page = QToolButton()
        self.btn_add_page.setPopupMode(QToolButton.MenuButtonPopup)
        self.btn_add_page.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Minimum)

        self.action_add_page = QAction(i18n("Add Page"), self)
        self.action_add_page.triggered.connect(self.slot_add_new_page_single)
        self.action_add_template = QAction(i18n("Add Page from Template"), self)
        self.action_add_template.triggered.connect(self.slot_add_new_page_from_template)
        self.action_add_existing = QAction(i18n("Add Existing Pages"), self)
        self.action_add_existing.triggered.connect(self.slot_add_page_from_url)
        self.action_remove_selected_page = QAction(i18n("Remove Page"), self)
        self.action_remove_selected_page.triggered.connect(self.slot_remove_selected_page)
        self.action_resize_all_pages = QAction(i18n("Batch Resize"), self)
        self.action_resize_all_pages.triggered.connect(self.slot_batch_resize)
        self.btn_add_page.setDefaultAction(self.action_add_page)
        self.action_show_page_viewer = QAction(i18n("View Page In Window"), self)
        self.action_show_page_viewer.triggered.connect(self.slot_show_page_viewer)
        self.action_scrape_authors = QAction(i18n("Scrape Author Info"), self)
        self.action_scrape_authors.setToolTip(i18n("Search for author information in documents and add it to the author list. This does not check for duplicates."))
        self.action_scrape_authors.triggered.connect(self.slot_scrape_author_list)
        self.action_scrape_translations = QAction(i18n("Scrape Text for Translation"), self)
        self.action_scrape_translations.triggered.connect(self.slot_scrape_translations)
        actionList = []
        menu_page = QMenu()
        actionList.append(self.action_add_page)
        actionList.append(self.action_add_template)
        actionList.append(self.action_add_existing)
        actionList.append(self.action_remove_selected_page)
        actionList.append(self.action_resize_all_pages)
        actionList.append(self.action_show_page_viewer)
        actionList.append(self.action_scrape_authors)
        actionList.append(self.action_scrape_translations)
        menu_page.addActions(actionList)
        self.btn_add_page.setMenu(menu_page)
        buttonLayout.addWidget(self.btn_add_page)
        self.btn_add_page.setDisabled(True)

        self.comicPageList.setContextMenuPolicy(Qt.ActionsContextMenu)
        self.comicPageList.addActions(actionList)

        # Export button that... exports.
        self.btn_export = QPushButton(i18n("Export Comic"))
        self.btn_export.clicked.connect(self.slot_export)
        buttonLayout.addWidget(self.btn_export)
        self.btn_export.setDisabled(True)

        self.btn_project_url = QPushButton(i18n("Copy Location"))
        self.btn_project_url.setToolTip(i18n("Copies the path of the project to the clipboard. Useful for quickly copying to a file manager or the like."))
        self.btn_project_url.clicked.connect(self.slot_copy_project_url)
        self.btn_project_url.setDisabled(True)
        buttonLayout.addWidget(self.btn_project_url)

        self.page_viewer_dialog = comics_project_page_viewer.comics_project_page_viewer()
        
        self.pagesWatcher = QFileSystemWatcher()
        self.pagesWatcher.fileChanged.connect(self.slot_start_delayed_check_page_update)

        buttonLayout.addItem(QSpacerItem(0, 0, QSizePolicy.Minimum, QSizePolicy.MinimumExpanding))

    """
    Open the config file and load the json file into a dictionary.
    """

    def slot_open_config(self):
        self.path_to_config = QFileDialog.getOpenFileName(caption=i18n("Please select the JSON comic config file."), filter=str(i18n("JSON files") + "(*.json)"))[0]
        if os.path.exists(self.path_to_config) is True:
            if os.access(self.path_to_config, os.W_OK) is False:
                QMessageBox.warning(None, i18n("Config cannot be used"), i18n("Krita doesn't have write access to this folder, so new files cannot be made. Please configure the folder access or move the project to a folder that can be written to."), QMessageBox.Ok)
                return
            configFile = open(self.path_to_config, "r", newline="", encoding="utf-16")
            self.setupDictionary = json.load(configFile)
            self.projecturl = os.path.dirname(str(self.path_to_config))
            configFile.close()
            self.load_config()
    """
    Further config loading.
    """

    def load_config(self):
        self.projectName.setMainText(text=str(self.setupDictionary["projectName"]))
        self.fill_pages()
        self.btn_settings.setEnabled(True)
        self.btn_add_page.setEnabled(True)
        self.btn_export.setEnabled(True)
        self.btn_project_url.setEnabled(True)

    """
    Fill the pages model with the pages from the pages list.
    """

    def fill_pages(self):
        self.loadingPages = True
        self.pagesModel.clear()
        if len(self.pagesWatcher.files())>0:
            self.pagesWatcher.removePaths(self.pagesWatcher.files())
        pagesList = []
        if "pages" in self.setupDictionary.keys():
            pagesList = self.setupDictionary["pages"]
        progress = QProgressDialog()
        progress.setMinimum(0)
        progress.setMaximum(len(pagesList))
        progress.setWindowTitle(i18n("Loading Pages..."))
        for url in pagesList:
            absurl = os.path.join(self.projecturl, url)
            relative = os.path.relpath(absurl, self.projecturl)
            if (os.path.exists(absurl)):
                #page = Application.openDocument(absurl)
                page = zipfile.ZipFile(absurl, "r")
                # Note: We load preview.png instead of mergedimage.png as each mergedimage.png can take hundreds of MiB
                # when loaded in memory.
                thumbnail = QImage.fromData(page.read("preview.png"))
                thumbnail.setDevicePixelRatio(self.devicePixelRatioF())
                pageItem = QStandardItem()
                dataList = self.get_description_and_title(page.read("documentinfo.xml"))
                if (dataList[0].isspace() or len(dataList[0]) < 1):
                    dataList[0] = os.path.basename(url)
                pageItem.setText(dataList[0].replace("_", " "))
                pageItem.setDragEnabled(True)
                pageItem.setDropEnabled(False)
                pageItem.setEditable(False)
                pageItem.setIcon(QIcon(QPixmap.fromImage(thumbnail)))
                pageItem.setData(dataList[1], role = CPE.DESCRIPTION)
                pageItem.setData(relative, role = CPE.URL)
                self.pagesWatcher.addPath(absurl)
                pageItem.setData(dataList[2], role = CPE.KEYWORDS)
                pageItem.setData(dataList[3], role = CPE.LASTEDIT)
                pageItem.setData(dataList[4], role = CPE.EDITOR)
                pageItem.setToolTip(relative)
                page.close()
                self.pagesModel.appendRow(pageItem)
                progress.setValue(progress.value() + 1)
        progress.setValue(len(pagesList))
        self.loadingPages = False
    """
    Function that is triggered by the zoomSlider
    Resizes the thumbnails.
    """

    def slot_scale_thumbnails(self, multiplier=4):
        self.comicPageList.setIconSize(QSize(multiplier * 32, multiplier * 32))

    """
    Function that takes the documentinfo.xml and parses it for the title, subject and abstract tags,
    to get the title and description.
    
    @returns a stringlist with the name on 0 and the description on 1.
    """

    def get_description_and_title(self, string):
        xmlDoc = ET.fromstring(string)
        calligra = str("{http://www.calligra.org/DTD/document-info}")
        name = ""
        if ET.iselement(xmlDoc[0].find(calligra + 'title')):
            name = xmlDoc[0].find(calligra + 'title').text
            if name is None:
                name = " "
        desc = ""
        if ET.iselement(xmlDoc[0].find(calligra + 'subject')):
            desc = xmlDoc[0].find(calligra + 'subject').text
        if desc is None or desc.isspace() or len(desc) < 1:
            if ET.iselement(xmlDoc[0].find(calligra + 'abstract')):
                desc = xmlDoc[0].find(calligra + 'abstract').text
                if desc is not None:
                    if desc.startswith("<![CDATA["):
                        desc = desc[len("<![CDATA["):]
                    if desc.startswith("]]>"):
                        desc = desc[:-len("]]>")]
        keywords = ""
        if ET.iselement(xmlDoc[0].find(calligra + 'keyword')):
            keywords = xmlDoc[0].find(calligra + 'keyword').text
        date = ""
        if ET.iselement(xmlDoc[0].find(calligra + 'date')):
            date = xmlDoc[0].find(calligra + 'date').text
        author = []
        if ET.iselement(xmlDoc[1].find(calligra + 'creator-first-name')):
            string = xmlDoc[1].find(calligra + 'creator-first-name').text
            if string is not None:
                author.append(string)
        if ET.iselement(xmlDoc[1].find(calligra + 'creator-last-name')):
            string = xmlDoc[1].find(calligra + 'creator-last-name').text
            if string is not None:
                author.append(string)
        if ET.iselement(xmlDoc[1].find(calligra + 'full-name')):
            string = xmlDoc[1].find(calligra + 'full-name').text
            if string is not None:
                author.append(string)
            
        return [name, desc, keywords, date, " ".join(author)]

    """
    Scrapes authors from the author data in the document info and puts them into the author list.
    Doesn't check for duplicates.
    """

    def slot_scrape_author_list(self):
        listOfAuthors = []
        if "authorList" in self.setupDictionary.keys():
            listOfAuthors = self.setupDictionary["authorList"]
        if "pages" in self.setupDictionary.keys():
            for relurl in self.setupDictionary["pages"]:
                absurl = os.path.join(self.projecturl, relurl)
                page = zipfile.ZipFile(absurl, "r")
                xmlDoc = ET.fromstring(page.read("documentinfo.xml"))
                calligra = str("{http://www.calligra.org/DTD/document-info}")
                authorelem = xmlDoc.find(calligra + 'author')
                author = {}
                if ET.iselement(authorelem.find(calligra + 'full-name')):
                    author["nickname"] = str(authorelem.find(calligra + 'full-name').text)

                if ET.iselement(authorelem.find(calligra + 'creator-first-name')):
                    author["first-name"] = str(authorelem.find(calligra + 'creator-first-name').text)

                if ET.iselement(authorelem.find(calligra + 'initial')):
                    author["initials"] = str(authorelem.find(calligra + 'initial').text)

                if ET.iselement(authorelem.find(calligra + 'creator-last-name')):
                    author["last-name"] = str(authorelem.find(calligra + 'creator-last-name').text)

                if ET.iselement(authorelem.find(calligra + 'email')):
                    author["email"] = str(authorelem.find(calligra + 'email').text)

                if ET.iselement(authorelem.find(calligra + 'contact')):
                    contact = authorelem.find(calligra + 'contact')
                    contactMode = contact.get("type")
                    if contactMode == "email":
                        author["email"] = str(contact.text)
                    if contactMode == "homepage":
                        author["homepage"] = str(contact.text)

                if ET.iselement(authorelem.find(calligra + 'position')):
                    author["role"] = str(authorelem.find(calligra + 'position').text)
                listOfAuthors.append(author)
                page.close()
        self.setupDictionary["authorList"] = listOfAuthors

    """
    Edit the general project settings like the project name, concept, pages location, export location, template location, metadata
    """

    def slot_edit_project_settings(self):
        dialog = comics_project_settings_dialog.comics_project_details_editor(self.projecturl)
        dialog.setConfig(self.setupDictionary, self.projecturl)

        if dialog.exec_() == QDialog.Accepted:
            self.setupDictionary = dialog.getConfig(self.setupDictionary)
            self.slot_write_config()
            self.projectName.setMainText(str(self.setupDictionary["projectName"]))

    """
    This allows users to select existing pages and add them to the pages list. The pages are currently not copied to the pages folder. Useful for existing projects.
    """

    def slot_add_page_from_url(self):
        # get the pages.
        urlList = QFileDialog.getOpenFileNames(caption=i18n("Which existing pages to add?"), directory=self.projecturl, filter=str(i18n("Krita files") + "(*.kra)"))[0]

        # get the existing pages list.
        pagesList = []
        if "pages" in self.setupDictionary.keys():
            pagesList = self.setupDictionary["pages"]

        # And add each url in the url list to the pages list and the model.
        for url in urlList:
            if self.projecturl not in urlList:
                newUrl = os.path.join(self.projecturl, self.setupDictionary["pagesLocation"], os.path.basename(url))
                shutil.move(url, newUrl)
                url = newUrl
            relative = os.path.relpath(url, self.projecturl)
            if url not in pagesList:
                page = zipfile.ZipFile(url, "r")
                thumbnail = QImage.fromData(page.read("preview.png"))
                dataList = self.get_description_and_title(page.read("documentinfo.xml"))
                if (dataList[0].isspace() or len(dataList[0]) < 1):
                    dataList[0] = os.path.basename(url)
                newPageItem = QStandardItem()
                newPageItem.setIcon(QIcon(QPixmap.fromImage(thumbnail)))
                newPageItem.setDragEnabled(True)
                newPageItem.setDropEnabled(False)
                newPageItem.setEditable(False)
                newPageItem.setText(dataList[0].replace("_", " "))
                newPageItem.setData(dataList[1], role = CPE.DESCRIPTION)
                newPageItem.setData(relative, role = CPE.URL)
                self.pagesWatcher.addPath(url)
                newPageItem.setData(dataList[2], role = CPE.KEYWORDS)
                newPageItem.setData(dataList[3], role = CPE.LASTEDIT)
                newPageItem.setData(dataList[4], role = CPE.EDITOR)
                newPageItem.setToolTip(relative)
                page.close()
                self.pagesModel.appendRow(newPageItem)

    """
    Remove the selected page from the list of pages. This does not remove it from disk(far too dangerous).
    """

    def slot_remove_selected_page(self):
        index = self.comicPageList.currentIndex()
        self.pagesModel.removeRow(index.row())

    """
    This function adds a new page from the default template. If there's no default template, or the file does not exist, it will 
    show the create/import template dialog. It will remember the selected item as the default template.
    """

    def slot_add_new_page_single(self):
        templateUrl = "templatepage"
        templateExists = False

        if "singlePageTemplate" in self.setupDictionary.keys():
            templateUrl = self.setupDictionary["singlePageTemplate"]
        if os.path.exists(os.path.join(self.projecturl, templateUrl)):
            templateExists = True

        if templateExists is False:
            if "templateLocation" not in self.setupDictionary.keys():
                self.setupDictionary["templateLocation"] = os.path.relpath(QFileDialog.getExistingDirectory(caption=i18n("Where are the templates located?"), options=QFileDialog.ShowDirsOnly), self.projecturl)

            templateDir = os.path.join(self.projecturl, self.setupDictionary["templateLocation"])
            template = comics_template_dialog.comics_template_dialog(templateDir)

            if template.exec_() == QDialog.Accepted:
                templateUrl = os.path.relpath(template.url(), self.projecturl)
                self.setupDictionary["singlePageTemplate"] = templateUrl
        if os.path.exists(os.path.join(self.projecturl, templateUrl)):
            self.add_new_page(templateUrl)

    """
    This function always asks for a template showing the new template window. This allows users to have multiple different
    templates created for back covers, spreads, other and have them accessible, while still having the convenience of a singular
    "add page" that adds a default.
    """

    def slot_add_new_page_from_template(self):
        if "templateLocation" not in self.setupDictionary.keys():
            self.setupDictionary["templateLocation"] = os.path.relpath(QFileDialog.getExistingDirectory(caption=i18n("Where are the templates located?"), options=QFileDialog.ShowDirsOnly), self.projecturl)

        templateDir = os.path.join(self.projecturl, self.setupDictionary["templateLocation"])
        template = comics_template_dialog.comics_template_dialog(templateDir)

        if template.exec_() == QDialog.Accepted:
            templateUrl = os.path.relpath(template.url(), self.projecturl)
            self.add_new_page(templateUrl)

    """
    This is the actual function that adds the template using the template url.
    It will attempt to name the new page projectName+number.
    """

    def add_new_page(self, templateUrl):

        # check for page list and or location.
        pagesList = []
        if "pages" in self.setupDictionary.keys():
            pagesList = self.setupDictionary["pages"]
        if not "pageNumber" in self.setupDictionary.keys():
            self.setupDictionary['pageNumber'] = 0

        if (str(self.setupDictionary["pagesLocation"]).isspace()):
            self.setupDictionary["pagesLocation"] = os.path.relpath(QFileDialog.getExistingDirectory(caption=i18n("Where should the pages go?"), options=QFileDialog.ShowDirsOnly), self.projecturl)

        # Search for the possible name.
        extraUnderscore = str()
        if str(self.setupDictionary["projectName"])[-1].isdigit():
            extraUnderscore = "_"
        self.setupDictionary['pageNumber'] += 1
        pageName = str(self.setupDictionary["projectName"]).replace(" ", "_") + extraUnderscore + str(format(self.setupDictionary['pageNumber'], "03d"))
        url = os.path.join(str(self.setupDictionary["pagesLocation"]), pageName + ".kra")

        # open the page by opening the template and resaving it, or just opening it.
        absoluteUrl = os.path.join(self.projecturl, url)
        if (os.path.exists(absoluteUrl)):
            newPage = Application.openDocument(absoluteUrl)
        else:
            booltemplateExists = os.path.exists(os.path.join(self.projecturl, templateUrl))
            if booltemplateExists is False:
                templateUrl = os.path.relpath(QFileDialog.getOpenFileName(caption=i18n("Which image should be the basis the new page?"), directory=self.projecturl, filter=str(i18n("Krita files") + "(*.kra)"))[0], self.projecturl)
            newPage = Application.openDocument(os.path.join(self.projecturl, templateUrl))
            newPage.waitForDone()
            newPage.setFileName(absoluteUrl)
            newPage.setName(pageName.replace("_", " "))
            newPage.save()
            newPage.waitForDone()

        # Get out the extra data for the standard item.
        newPageItem = QStandardItem()
        newPageItem.setIcon(QIcon(QPixmap.fromImage(newPage.thumbnail(256, 256))))
        newPageItem.setDragEnabled(True)
        newPageItem.setDropEnabled(False)
        newPageItem.setEditable(False)
        newPageItem.setText(pageName.replace("_", " "))
        newPageItem.setData("", role = CPE.DESCRIPTION)
        newPageItem.setData(url, role = CPE.URL)
        newPageItem.setData("", role = CPE.KEYWORDS)
        newPageItem.setData("", role = CPE.LASTEDIT)
        newPageItem.setData("", role = CPE.EDITOR)
        newPageItem.setToolTip(url)

        # close page document.
        while os.path.exists(absoluteUrl) is False:
            qApp.processEvents()

        self.pagesWatcher.addPath(absoluteUrl)
        newPage.close()

        # add item to page.
        self.pagesModel.appendRow(newPageItem)

    """
    Write to the json configuration file.
    This also checks the current state of the pages list.
    """

    def slot_write_config(self):

        # Don't load when the pages are still being loaded, otherwise we'll be overwriting our own pages list.
        if (self.loadingPages is False):
            print("CPMT: writing comic configuration...")

            # Generate a pages list from the pagesmodel.
            pagesList = []
            for i in range(self.pagesModel.rowCount()):
                index = self.pagesModel.index(i, 0)
                url = str(self.pagesModel.data(index, role=CPE.URL))
                if url not in pagesList:
                    pagesList.append(url)
            self.setupDictionary["pages"] = pagesList

            # Save to our json file.
            configFile = open(self.path_to_config, "w", newline="", encoding="utf-16")
            json.dump(self.setupDictionary, configFile, indent=4, sort_keys=True, ensure_ascii=False)
            configFile.close()
            print("CPMT: done")

    """
    Open a page in the pagesmodel in Krita.
    """

    def slot_open_page(self, index):
        if index.column() == 0:
            # Get the absolute url from the relative one in the pages model.
            absoluteUrl = os.path.join(self.projecturl, str(self.pagesModel.data(index, role=CPE.URL)))

            # Make sure the page exists.
            if os.path.exists(absoluteUrl):
                page = Application.openDocument(absoluteUrl)

                # Set the title to the filename if it was empty. It looks a bit neater.
                if page.name().isspace or len(page.name()) < 1:
                    page.setName(str(self.pagesModel.data(index, role=Qt.DisplayRole)).replace("_", " "))

                # Add views for the document so the user can use it.
                Application.activeWindow().addView(page)
                Application.setActiveDocument(page)
            else:
                print("CPMT: The page cannot be opened because the file doesn't exist:", absoluteUrl)

    """
    Call up the metadata editor dialog. Only when the dialog is "Accepted" will the metadata be saved.
    """

    def slot_edit_meta_data(self):
        dialog = comics_metadata_dialog.comic_meta_data_editor()

        dialog.setConfig(self.setupDictionary)
        if (dialog.exec_() == QDialog.Accepted):
            self.setupDictionary = dialog.getConfig(self.setupDictionary)
            self.slot_write_config()

    """
    An attempt at making the description editable from the comic pages list.
    It is currently not working because ZipFile has no overwrite mechanism,
    and I don't have the energy to write one yet.
    """

    def slot_write_description(self, index):

        for row in range(self.pagesModel.rowCount()):
            index = self.pagesModel.index(row, 1)
            indexUrl = self.pagesModel.index(row, 0)
            absoluteUrl = os.path.join(self.projecturl, str(self.pagesModel.data(indexUrl, role=CPE.URL)))
            page = zipfile.ZipFile(absoluteUrl, "a")
            xmlDoc = ET.ElementTree()
            ET.register_namespace("", "http://www.calligra.org/DTD/document-info")
            location = os.path.join(self.projecturl, "documentinfo.xml")
            xmlDoc.parse(location)
            xmlroot = ET.fromstring(page.read("documentinfo.xml"))
            calligra = "{http://www.calligra.org/DTD/document-info}"
            aboutelem = xmlroot.find(calligra + 'about')
            if ET.iselement(aboutelem.find(calligra + 'subject')):
                desc = aboutelem.find(calligra + 'subject')
                desc.text = self.pagesModel.data(index, role=Qt.EditRole)
                xmlstring = ET.tostring(xmlroot, encoding='unicode', method='xml', short_empty_elements=False)
                page.writestr(zinfo_or_arcname="documentinfo.xml", data=xmlstring)
                for document in Application.documents():
                    if str(document.fileName()) == str(absoluteUrl):
                        document.setDocumentInfo(xmlstring)
            page.close()

    """
    Calls up the export settings dialog. Only when accepted will the configuration be written.
    """

    def slot_edit_export_settings(self):
        dialog = comics_export_dialog.comic_export_setting_dialog()
        dialog.setConfig(self.setupDictionary)

        if (dialog.exec_() == QDialog.Accepted):
            self.setupDictionary = dialog.getConfig(self.setupDictionary)
            self.slot_write_config()

    """
    Export the comic. Won't work without export settings set.
    """

    def slot_export(self):
        
        #ensure there is a unique identifier
        if "uuid" not in self.setupDictionary.keys():
            uuid = str()
            if "acbfID" in self.setupDictionary.keys():
                uuid = str(self.setupDictionary["acbfID"])
            else:
                uuid = QUuid.createUuid().toString()
            self.setupDictionary["uuid"] = uuid
        
        exporter = comics_exporter.comicsExporter()
        exporter.set_config(self.setupDictionary, self.projecturl)
        exportSuccess = exporter.export()
        if exportSuccess:
            print("CPMT: Export success! The files have been written to the export folder!")
            QMessageBox.information(self, i18n("Export success"), i18n("The files have been written to the export folder."), QMessageBox.Ok)

    """
    Calls up the comics project setup wizard so users can create a new json file with the basic information.
    """

    def slot_new_project(self):
        setup = comics_project_setup_wizard.ComicsProjectSetupWizard()
        setup.showDialog()
        self.path_to_config = os.path.join(setup.projectDirectory, "comicConfig.json")
        if os.path.exists(self.path_to_config) is True:
            configFile = open(self.path_to_config, "r", newline="", encoding="utf-16")
            self.setupDictionary = json.load(configFile)
            self.projecturl = os.path.dirname(str(self.path_to_config))
            configFile.close()
            self.load_config()
    """
    This is triggered by any document save.
    It checks if the given url is in the pages list, and if so,
    updates the appropriate page thumbnail.
    This helps with the management of the pages, because the user
    will be able to see the thumbnails as a todo for the whole comic,
    giving a good overview over whether they still need to ink, color or
    the like for a given page, and it thus also rewards the user whenever
    they save.
    """

    def slot_start_delayed_check_page_update(self, url):
        # It can happen that there are multiple signals from QFileSystemWatcher at once.
        # Since QTimer cannot take any arguments, we need to keep a list of files to update.
        # Otherwise only the last file would be updated and all subsequent calls
        #   of `slot_check_for_page_update` would not know which files to update now.
        # https://bugs.kde.org/show_bug.cgi?id=426701
        self.updateurls.append(url)
        QTimer.singleShot(200, Qt.PreciseTimer, self.slot_check_for_page_update)
         

    def slot_check_for_page_update(self):
        url = self.updateurls.pop(0)
        if url:
            if "pages" in self.setupDictionary.keys():
                relUrl = os.path.relpath(url, self.projecturl)
                if relUrl in self.setupDictionary["pages"]:
                    index = self.pagesModel.index(self.setupDictionary["pages"].index(relUrl), 0)
                    if index.isValid():
                        if os.path.exists(url) is False:
                            # we cannot check from here whether the file in question has been renamed or deleted.
                            self.pagesModel.removeRow(index.row())
                            return
                        else:
                            # Krita will trigger the filesystemwatcher when doing backupfiles,
                            # so ensure the file is still watched if it exists.
                            self.pagesWatcher.addPath(url)
                        pageItem = self.pagesModel.itemFromIndex(index)
                        page = zipfile.ZipFile(url, "r")
                        dataList = self.get_description_and_title(page.read("documentinfo.xml"))
                        if (dataList[0].isspace() or len(dataList[0]) < 1):
                            dataList[0] = os.path.basename(url)
                        thumbnail = QImage.fromData(page.read("preview.png"))
                        pageItem.setIcon(QIcon(QPixmap.fromImage(thumbnail)))
                        pageItem.setText(dataList[0])
                        pageItem.setData(dataList[1], role = CPE.DESCRIPTION)
                        pageItem.setData(relUrl, role = CPE.URL)
                        pageItem.setData(dataList[2], role = CPE.KEYWORDS)
                        pageItem.setData(dataList[3], role = CPE.LASTEDIT)
                        pageItem.setData(dataList[4], role = CPE.EDITOR)
                        self.pagesModel.setItem(index.row(), index.column(), pageItem)

    """
    Resize all the pages in the pages list.
    It will show a dialog with the options for resizing.
    Then, it will try to pop up a progress dialog while resizing.
    The progress dialog shows the remaining time and pages.
    """

    def slot_batch_resize(self):
        dialog = QDialog()
        dialog.setWindowTitle(i18n("Resize all Pages"))
        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttons.accepted.connect(dialog.accept)
        buttons.rejected.connect(dialog.reject)
        sizesBox = comics_export_dialog.comic_export_resize_widget("Scale", batch=True, fileType=False)
        exporterSizes = comics_exporter.sizesCalculator()
        dialog.setLayout(QVBoxLayout())
        dialog.layout().addWidget(sizesBox)
        dialog.layout().addWidget(buttons)

        if dialog.exec_() == QDialog.Accepted:
            progress = QProgressDialog(i18n("Resizing pages..."), str(), 0, len(self.setupDictionary["pages"]))
            progress.setWindowTitle(i18n("Resizing Pages"))
            progress.setCancelButton(None)
            timer = QElapsedTimer()
            timer.start()
            config = {}
            config = sizesBox.get_config(config)
            for p in range(len(self.setupDictionary["pages"])):
                absoluteUrl = os.path.join(self.projecturl, self.setupDictionary["pages"][p])
                progress.setValue(p)
                timePassed = timer.elapsed()
                if (p > 0):
                    timeEstimated = (len(self.setupDictionary["pages"]) - p) * (timePassed / p)
                    passedString = str(int(timePassed / 60000)) + ":" + format(int(timePassed / 1000), "02d") + ":" + format(timePassed % 1000, "03d")
                    estimatedString = str(int(timeEstimated / 60000)) + ":" + format(int(timeEstimated / 1000), "02d") + ":" + format(int(timeEstimated % 1000), "03d")
                    progress.setLabelText(str(i18n("{pages} of {pagesTotal} done. \nTime passed: {passedString}:\n Estimated:{estimated}")).format(pages=p, pagesTotal=len(self.setupDictionary["pages"]), passedString=passedString, estimated=estimatedString))
                    qApp.processEvents()
                if os.path.exists(absoluteUrl):
                    doc = Application.openDocument(absoluteUrl)
                    listScales = exporterSizes.get_scale_from_resize_config(config["Scale"], [doc.width(), doc.height(), doc.resolution(), doc.resolution()])
                    doc.scaleImage(listScales[0], listScales[1], listScales[2], listScales[3], "bicubic")
                    doc.waitForDone()
                    doc.save()
                    doc.waitForDone()
                    doc.close()

    def slot_show_page_viewer(self):
        index = int(self.comicPageList.currentIndex().row())
        self.page_viewer_dialog.load_comic(self.path_to_config)
        self.page_viewer_dialog.go_to_page_index(index)
        self.page_viewer_dialog.show()

    """
    Function to copy the current project location into the clipboard.
    This is useful for users because they'll be able to use that url to quickly
    move to the project location in outside applications.
    """

    def slot_copy_project_url(self):
        if self.projecturl is not None:
            clipboard = qApp.clipboard()
            clipboard.setText(str(self.projecturl))

    """
    Scrape text files with the textlayer keys for text, and put those in a POT
    file. This makes it possible to handle translations.
    """

    def slot_scrape_translations(self):
        translationFolder = self.setupDictionary.get("translationLocation", "translations")
        fullTranslationPath = os.path.join(self.projecturl, translationFolder)
        os.makedirs(fullTranslationPath, exist_ok=True)
        textLayersToSearch = self.setupDictionary.get("textLayerNames", ["text"])

        scraper = comics_project_translation_scraper.translation_scraper(self.projecturl, translationFolder, textLayersToSearch, self.setupDictionary["projectName"])
        # Run text scraper.
        language = self.setupDictionary.get("language", "en")
        metadata = {}
        metadata["title"] = self.setupDictionary.get("title", "")
        metadata["summary"] = self.setupDictionary.get("summary", "")
        metadata["keywords"] = ", ".join(self.setupDictionary.get("otherKeywords", [""]))
        metadata["transnotes"] = self.setupDictionary.get("translatorHeader", "Translator's Notes")
        scraper.start(self.setupDictionary["pages"], language, metadata)
        QMessageBox.information(self, i18n("Scraping success"), str(i18n("POT file has been written to: {file}")).format(file=fullTranslationPath), QMessageBox.Ok)
    """
    This is required by the dockwidget class, otherwise unused.
    """

    def canvasChanged(self, canvas):
        pass


"""
Add docker to program
"""
Application.addDockWidgetFactory(DockWidgetFactory("comics_project_manager_docker", DockWidgetFactoryBase.DockRight, comics_project_manager_docker))
