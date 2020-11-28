"""
SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

This file is part of the Comics Project Management Tools(CPMT).

SPDX-License-Identifier: GPL-3.0-or-later
"""

"""
This is a metadata editor that helps out setting the proper metadata
"""
import sys
import os  # For finding the script location.
import csv
import re
import types
from pathlib import Path  # For reading all the files in a directory.
from PyQt5.QtGui import QStandardItem, QStandardItemModel, QImage, QIcon, QPixmap, QPainter, QPalette, QFontDatabase
from PyQt5.QtWidgets import QComboBox, QCompleter, QStyledItemDelegate, QLineEdit, QDialog, QDialogButtonBox, QVBoxLayout, QFormLayout, QTabWidget, QWidget, QPlainTextEdit, QHBoxLayout, QSpinBox, QDateEdit, QPushButton, QLabel, QTableView
from PyQt5.QtCore import QDir, QLocale, QStringListModel, Qt, QDate, QSize, QUuid
"""
multi entry completer cobbled together from the two examples on stackoverflow:3779720

This allows us to let people type in comma-separated lists and get completion for those.
"""


class multi_entry_completer(QCompleter):
    punctuation = ","

    def __init__(self, parent=None):
        super(QCompleter, self).__init__(parent)

    def pathFromIndex(self, index):
        path = QCompleter.pathFromIndex(self, index)
        string = str(self.widget().text())
        split = string.split(self.punctuation)
        if len(split) > 1:
            path = "%s, %s" % (",".join(split[:-1]), path)
        return path

    def splitPath(self, path):
        split = str(path.split(self.punctuation)[-1])
        if split.startswith(" "):
            split = split[1:]
        if split.endswith(" "):
            split = split[:-1]
        return [split]
    

"""
Language combobox that can take locale codes and get the right language for it and visa-versa.
"""


class language_combo_box(QComboBox):
    languageList = []
    codesList = []

    def __init__(self, parent=None):
        super(QComboBox, self).__init__(parent)
        for i in range(1, 357):
            locale = QLocale(i)
            if locale and QLocale.languageToString(locale.language()) != "C":
                codeName = locale.name().split("_")[0]
                if codeName not in self.codesList:
                    self.codesList.append(codeName)
            self.codesList.sort()

        for lang in self.codesList:
            locale = QLocale(lang)

            if locale:
                languageName = locale.nativeLanguageName()

                if len(languageName)==0:
                    languageName = QLocale.languageToString(locale.language())

                self.languageList.append(languageName.title())
                self.setIconSize(QSize(32, 22))
                codeIcon = QImage(self.iconSize(), QImage.Format_ARGB32)
                painter = QPainter(codeIcon)
                painter.setBrush(Qt.transparent)
                codeIcon.fill(Qt.transparent)
                font = QFontDatabase().systemFont(QFontDatabase.FixedFont)
                painter.setFont(font)
                painter.setPen(self.palette().color(QPalette.Text))
                painter.drawText(codeIcon.rect(), Qt.AlignCenter,lang)
                painter.end()
                self.addItem(QIcon(QPixmap.fromImage(codeIcon)), languageName.title())

    def codeForCurrentEntry(self):
        if self.currentText() in self.languageList:
            return self.codesList[self.languageList.index(self.currentText())]

    def setEntryToCode(self, code):
        if (code == "C" and "en" in self.codesList):
            self.setCurrentIndex(self.codesList.index("en"))
        if code in self.codesList:
            self.setCurrentIndex(self.codesList.index(code))

class country_combo_box(QComboBox):
    countryList = []
    codesList = []

    def __init__(self, parent=None):
        super(QComboBox, self).__init__(parent)
        
    def set_country_for_locale(self, languageCode):
        self.clear()
        self.codesList = []
        self.countryList = []
        for locale in QLocale.matchingLocales(QLocale(languageCode).language(), QLocale.AnyScript, QLocale.AnyCountry):
            codeName = locale.name().split("_")[-1]
            if codeName not in self.codesList:
                self.codesList.append(codeName)
            self.codesList.sort()
            
        for country in self.codesList:
            locale = QLocale(languageCode+"-"+country)
            if locale:
                countryName = locale.nativeCountryName()
                self.countryList.append(countryName.title())
                self.setIconSize(QSize(32, 22))
                codeIcon = QImage(self.iconSize(), QImage.Format_ARGB32)
                painter = QPainter(codeIcon)
                painter.setBrush(Qt.transparent)
                codeIcon.fill(Qt.transparent)
                font = QFontDatabase().systemFont(QFontDatabase.FixedFont)
                painter.setFont(font)
                painter.setPen(self.palette().color(QPalette.Text))
                painter.drawText(codeIcon.rect(), Qt.AlignCenter,country)
                painter.end()
                self.addItem(QIcon(QPixmap.fromImage(codeIcon)), countryName.title())
                
    def codeForCurrentEntry(self):
        if self.currentText() in self.countryList:
            return self.codesList[self.countryList.index(self.currentText())]

    def setEntryToCode(self, code):
        if code == "C":
            self.setCurrentIndex(0)
        elif code in self.codesList:
            self.setCurrentIndex(self.codesList.index(code))

"""
A combobox that fills up with licenses from a CSV, and also sets tooltips from that
csv.
"""


class license_combo_box(QComboBox):
    def __init__(self, parent=None):
        super(QComboBox, self).__init__(parent)
        mainP = os.path.dirname(__file__)
        languageP = os.path.join(mainP, "LicenseList.csv")
        model = QStandardItemModel()
        if (os.path.exists(languageP)):
            file = open(languageP, "r", newline="", encoding="utf8")
            languageReader = csv.reader(file)
            for row in languageReader:
                license = QStandardItem(row[0])
                license.setToolTip(row[1])
                model.appendRow(license)
            file.close()
        self.setModel(model)


"""
Allows us to set completers on the author roles.
"""


class author_delegate(QStyledItemDelegate):
    completerStrings = []
    completerColumn = 0
    languageColumn = 0

    def __init__(self, parent=None):
        super(QStyledItemDelegate, self).__init__(parent)

    def setCompleterData(self, completerStrings=[str()], completerColumn=0):
        self.completerStrings = completerStrings
        self.completerColumn = completerColumn

    def setLanguageData(self, languageColumn=0):
        self.languageColumn = languageColumn

    def createEditor(self, parent, option, index):
        if index.column() != self.languageColumn:
            editor = QLineEdit(parent)
        else:
            editor = QComboBox(parent)
            editor.addItem("")
            for i in range(2, 356):
                if QLocale(i, QLocale.AnyScript, QLocale.AnyCountry) is not None:
                    languagecode = QLocale(i, QLocale.AnyScript, QLocale.AnyCountry).name().split("_")[0]
                    if languagecode != "C":
                        editor.addItem(languagecode)
            editor.model().sort(0)

        if index.column() == self.completerColumn:
            editor.setCompleter(QCompleter(self.completerStrings))
            editor.completer().setCaseSensitivity(False)

        return editor


"""
A comic project metadata editing dialog that can take our config diactionary and set all the relevant information.

To help our user, the dialog loads up lists of keywords to populate several autocompletion methods.
"""


class comic_meta_data_editor(QDialog):
    configGroup = "ComicsProjectManagementTools"

    # Translatable genre dictionary that has it's translated entries added to the genrelist and from which the untranslated items are taken.
    acbfGenreList = {"science_fiction": str(i18n("Science Fiction")), "fantasy": str(i18n("Fantasy")), "adventure": str(i18n("Adventure")), "horror": str(i18n("Horror")), "mystery": str(i18n("Mystery")), "crime": str(i18n("Crime")), "military": str(i18n("Military")), "real_life": str(i18n("Real Life")), "superhero": str(i18n("Superhero")), "humor": str(i18n("Humor")), "western": str(i18n("Western")), "manga": str(i18n("Manga")), "politics": str(i18n("Politics")), "caricature": str(i18n("Caricature")), "sports": str(i18n("Sports")), "history": str(i18n("History")), "biography": str(i18n("Biography")), "education": str(i18n("Education")), "computer": str(i18n("Computer")), "religion": str(i18n("Religion")), "romance": str(i18n("Romance")), "children": str(i18n("Children")), "non-fiction": str(i18n("Non Fiction")), "adult": str(i18n("Adult")), "alternative": str(i18n("Alternative")), "artbook": str(i18n("Artbook")), "other": str(i18n("Other"))}
    acbfAuthorRolesList = {"Writer": str(i18n("Writer")), "Adapter": str(i18n("Adapter")), "Artist": str(i18n("Artist")), "Penciller": str(i18n("Penciller")), "Inker": str(i18n("Inker")), "Colorist": str(i18n("Colorist")), "Letterer": str(i18n("Letterer")), "Cover Artist": str(i18n("Cover Artist")), "Photographer": str(i18n("Photographer")), "Editor": str(i18n("Editor")), "Assistant Editor": str(i18n("Assistant Editor")), "Designer": str(i18n("Designer")), "Translator": str(i18n("Translator")), "Other": str(i18n("Other"))}

    def __init__(self):
        super().__init__()
        # Get the keys for the autocompletion.
        self.genreKeysList = []
        self.characterKeysList = []
        self.ratingKeysList = {}
        self.formatKeysList = []
        self.otherKeysList = []
        self.authorRoleList = []
        for g in self.acbfGenreList.values():
            self.genreKeysList.append(g)
        for r in self.acbfAuthorRolesList.values():
            self.authorRoleList.append(r)
        mainP = Path(os.path.abspath(__file__)).parent
        self.get_auto_completion_keys(mainP)
        extraKeyP = Path(QDir.homePath()) / Application.readSetting(self.configGroup, "extraKeysLocation", str())
        self.get_auto_completion_keys(extraKeyP)

        # Setup the dialog.
        self.setLayout(QVBoxLayout())
        mainWidget = QTabWidget()
        self.layout().addWidget(mainWidget)
        self.setWindowTitle(i18n("Comic Metadata"))
        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        self.layout().addWidget(buttons)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)

        # Title, concept, summary, genre, characters, format, rating, language, series, other keywords
        metadataPage = QWidget()
        mformLayout = QFormLayout()
        metadataPage.setLayout(mformLayout)

        self.lnTitle = QLineEdit()
        self.lnTitle.setToolTip(i18n("The proper title of the comic."))

        self.teSummary = QPlainTextEdit()
        self.teSummary.setToolTip(i18n("What will you tell others to entice them to read your comic?"))

        self.lnGenre = QLineEdit()
        genreCompletion = multi_entry_completer()
        genreCompletion.setModel(QStringListModel(self.genreKeysList))
        self.lnGenre.setCompleter(genreCompletion)
        genreCompletion.setCaseSensitivity(False)
        self.lnGenre.setToolTip(i18n("The genre of the work. Prefilled values are from the ACBF, but you can fill in your own. Separate genres with commas. Try to limit the amount to about two or three."))

        self.lnCharacters = QLineEdit()
        characterCompletion = multi_entry_completer()
        characterCompletion.setModel(QStringListModel(self.characterKeysList))
        characterCompletion.setCaseSensitivity(False)
        characterCompletion.setFilterMode(Qt.MatchContains)  # So that if there is a list of names with last names, people can type in a last name.
        self.lnCharacters.setCompleter(characterCompletion)
        self.lnCharacters.setToolTip(i18n("The names of the characters that this comic revolves around. Comma-separated."))

        self.lnFormat = QLineEdit()
        formatCompletion = multi_entry_completer()
        formatCompletion.setModel(QStringListModel(self.formatKeysList))
        formatCompletion.setCaseSensitivity(False)
        self.lnFormat.setCompleter(formatCompletion)

        ratingLayout = QHBoxLayout()
        self.cmbRatingSystem = QComboBox()
        self.cmbRatingSystem.addItems(self.ratingKeysList.keys())
        self.cmbRatingSystem.setEditable(True)
        self.cmbRating = QComboBox()
        self.cmbRating.setEditable(True)
        self.cmbRatingSystem.currentIndexChanged.connect(self.slot_refill_ratings)
        ratingLayout.addWidget(self.cmbRatingSystem)
        ratingLayout.addWidget(self.cmbRating)

        self.lnSeriesName = QLineEdit()
        self.lnSeriesName.setToolTip(i18n("If this is part of a series, enter the name of the series and the number."))
        self.spnSeriesNumber = QSpinBox()
        self.spnSeriesNumber.setPrefix(i18n("No. "))
        self.spnSeriesVol = QSpinBox()
        self.spnSeriesVol.setPrefix(i18n("Vol. "))
        seriesLayout = QHBoxLayout()
        seriesLayout.addWidget(self.lnSeriesName)
        seriesLayout.addWidget(self.spnSeriesVol)
        seriesLayout.addWidget(self.spnSeriesNumber)

        otherCompletion = multi_entry_completer()
        otherCompletion.setModel(QStringListModel(self.otherKeysList))
        otherCompletion.setCaseSensitivity(False)
        otherCompletion.setFilterMode(Qt.MatchContains)
        self.lnOtherKeywords = QLineEdit()
        self.lnOtherKeywords.setCompleter(otherCompletion)
        self.lnOtherKeywords.setToolTip(i18n("Other keywords that do not fit in the previously mentioned sets. As always, comma-separated."))

        self.cmbLanguage = language_combo_box()
        self.cmbCountry = country_combo_box()
        self.cmbLanguage.currentIndexChanged.connect(self.slot_update_countries)
        self.cmbReadingMode = QComboBox()
        self.cmbReadingMode.addItem(i18n("Left to Right"))
        self.cmbReadingMode.addItem(i18n("Right to Left"))

        self.cmbCoverPage = QComboBox()
        self.cmbCoverPage.setToolTip(i18n("Which page is the cover page? This will be empty if there are no pages."))

        mformLayout.addRow(i18n("Title:"), self.lnTitle)
        mformLayout.addRow(i18n("Cover page:"), self.cmbCoverPage)
        mformLayout.addRow(i18n("Summary:"), self.teSummary)
        mformLayout.addRow(i18n("Language:"), self.cmbLanguage)
        mformLayout.addRow("", self.cmbCountry)
        mformLayout.addRow(i18n("Reading direction:"), self.cmbReadingMode)
        mformLayout.addRow(i18n("Genre:"), self.lnGenre)
        mformLayout.addRow(i18n("Characters:"), self.lnCharacters)
        mformLayout.addRow(i18n("Format:"), self.lnFormat)
        mformLayout.addRow(i18n("Rating:"), ratingLayout)
        mformLayout.addRow(i18n("Series:"), seriesLayout)
        mformLayout.addRow(i18n("Other:"), self.lnOtherKeywords)

        mainWidget.addTab(metadataPage, i18n("Work"))

        # The page for the authors.
        authorPage = QWidget()
        authorPage.setLayout(QVBoxLayout())
        explanation = QLabel(i18n("The following is a table of the authors that contributed to this comic. You can set their nickname, proper names (first, middle, last), role (penciller, inker, etc), email and homepage."))
        explanation.setWordWrap(True)
        self.authorModel = QStandardItemModel(0, 8)
        labels = [i18n("Nick Name"), i18n("Given Name"), i18n("Middle Name"), i18n("Family Name"), i18n("Role"), i18n("Email"), i18n("Homepage"), i18n("Language")]
        self.authorModel.setHorizontalHeaderLabels(labels)
        self.authorTable = QTableView()
        self.authorTable.setModel(self.authorModel)
        self.authorTable.verticalHeader().setDragEnabled(True)
        self.authorTable.verticalHeader().setDropIndicatorShown(True)
        self.authorTable.verticalHeader().setSectionsMovable(True)
        self.authorTable.verticalHeader().sectionMoved.connect(self.slot_reset_author_row_visual)
        delegate = author_delegate()
        delegate.setCompleterData(self.authorRoleList, 4)
        delegate.setLanguageData(len(labels) - 1)
        self.authorTable.setItemDelegate(delegate)
        author_button_layout = QWidget()
        author_button_layout.setLayout(QHBoxLayout())
        btn_add_author = QPushButton(i18n("Add Author"))
        btn_add_author.clicked.connect(self.slot_add_author)
        btn_remove_author = QPushButton(i18n("Remove Author"))
        btn_remove_author.clicked.connect(self.slot_remove_author)
        author_button_layout.layout().addWidget(btn_add_author)
        author_button_layout.layout().addWidget(btn_remove_author)
        authorPage.layout().addWidget(explanation)
        authorPage.layout().addWidget(self.authorTable)
        authorPage.layout().addWidget(author_button_layout)
        mainWidget.addTab(authorPage, i18n("Authors"))

        # The page with publisher information.
        publisherPage = QWidget()
        publisherLayout = QFormLayout()
        publisherPage.setLayout(publisherLayout)
        self.publisherName = QLineEdit()
        self.publisherName.setToolTip(i18n("The name of the company, group or person who is responsible for the final version the reader gets."))
        publishDateLayout = QHBoxLayout()
        self.publishDate = QDateEdit()
        self.publishDate.setDisplayFormat(QLocale().system().dateFormat())
        currentDate = QPushButton(i18n("Set Today"))
        currentDate.setToolTip(i18n("Sets the publish date to the current date."))
        currentDate.clicked.connect(self.slot_set_date)
        publishDateLayout.addWidget(self.publishDate)
        publishDateLayout.addWidget(currentDate)
        self.publishCity = QLineEdit()
        self.publishCity.setToolTip(i18n("Traditional publishers are always mentioned in source with the city they are located."))
        self.isbn = QLineEdit()
        self.license = license_combo_box()  # Maybe ought to make this a QLineEdit...
        self.license.setEditable(True)
        self.license.completer().setCompletionMode(QCompleter.PopupCompletion)
        dataBaseReference = QVBoxLayout()
        self.ln_database_name = QLineEdit()
        self.ln_database_name.setToolTip(i18n("If there is an entry in a comics data base, that should be added here. It is unlikely to be a factor for comics from scratch, but useful when doing a conversion."))
        self.cmb_entry_type = QComboBox()
        self.cmb_entry_type.addItems(["IssueID", "SeriesID", "URL"])
        self.cmb_entry_type.setEditable(True)
        self.ln_source = QLineEdit()
        self.ln_source.setToolTip(i18n("Whether the comic is an adaptation of an existing source, and if so, how to find information about that source. So for example, for an adapted webcomic, the official website url should go here."))
        self.label_uuid = QLabel()
        self.label_uuid.setToolTip(i18n("By default this will be filled with a generated universal unique identifier. The ID by itself is merely so that comic book library management programs can figure out if this particular comic is already in their database and whether it has been rated. Of course, the UUID can be changed into something else by manually changing the JSON, but this is advanced usage."))
        self.ln_database_entry = QLineEdit()
        dbHorizontal = QHBoxLayout()
        dbHorizontal.addWidget(self.ln_database_name)
        dbHorizontal.addWidget(self.cmb_entry_type)
        dataBaseReference.addLayout(dbHorizontal)
        dataBaseReference.addWidget(self.ln_database_entry)
        publisherLayout.addRow(i18n("Name:"), self.publisherName)
        publisherLayout.addRow(i18n("City:"), self.publishCity)
        publisherLayout.addRow(i18n("Date:"), publishDateLayout)
        publisherLayout.addRow(i18n("ISBN:"), self.isbn)
        publisherLayout.addRow(i18n("Source:"), self.ln_source)
        publisherLayout.addRow(i18n("UUID:"), self.label_uuid)
        publisherLayout.addRow(i18n("License:"), self.license)
        publisherLayout.addRow(i18n("Database:"), dataBaseReference)

        mainWidget.addTab(publisherPage, i18n("Publisher"))
    """
    Ensure that the drag and drop of authors doesn't mess up the labels.
    """

    def slot_reset_author_row_visual(self):
        headerLabelList = []
        for i in range(self.authorTable.verticalHeader().count()):
            headerLabelList.append(str(i))
        for i in range(self.authorTable.verticalHeader().count()):
            logicalI = self.authorTable.verticalHeader().logicalIndex(i)
            headerLabelList[logicalI] = str(i + 1)
        self.authorModel.setVerticalHeaderLabels(headerLabelList)
    """
    Set the publish date to the current date.
    """

    def slot_set_date(self):
        self.publishDate.setDate(QDate().currentDate())
        
    def slot_update_countries(self):
        code = self.cmbLanguage.codeForCurrentEntry()
        self.cmbCountry.set_country_for_locale(code)

    """
    Append keys to autocompletion lists from the directory mainP.
    """

    def get_auto_completion_keys(self, mainP=Path()):
        genre = Path(mainP / "key_genre")
        characters = Path(mainP / "key_characters")
        rating = Path(mainP / "key_rating")
        format = Path(mainP / "key_format")
        keywords = Path(mainP / "key_other")
        authorRole = Path(mainP / "key_author_roles")
        if genre.exists():
            for t in list(genre.glob('**/*.txt')):
                file = open(str(t), "r", errors="replace")
                for l in file:
                    if str(l).strip("\n") not in self.genreKeysList:
                        self.genreKeysList.append(str(l).strip("\n"))
                file.close()
        if characters.exists():
            for t in list(characters.glob('**/*.txt')):
                file = open(str(t), "r", errors="replace")
                for l in file:
                    if str(l).strip("\n") not in self.characterKeysList:
                        self.characterKeysList.append(str(l).strip("\n"))
                file.close()
        if format.exists():
            for t in list(format.glob('**/*.txt')):
                file = open(str(t), "r", errors="replace")
                for l in file:
                    if str(l).strip("\n") not in self.formatKeysList:
                        self.formatKeysList.append(str(l).strip("\n"))
                file.close()
        if rating.exists():
            for t in list(rating.glob('**/*.csv')):
                file = open(str(t), "r", newline="", encoding="utf-8")
                ratings = csv.reader(file)
                title = os.path.basename(str(t))
                r = 0
                for row in ratings:
                    listItem = []
                    if r == 0:
                        title = row[1]
                    else:
                        listItem = self.ratingKeysList[title]
                        item = []
                        item.append(row[0])
                        item.append(row[1])
                        listItem.append(item)
                    self.ratingKeysList[title] = listItem
                    r += 1
                file.close()
        if keywords.exists():
            for t in list(keywords.glob('**/*.txt')):
                file = open(str(t), "r", errors="replace")
                for l in file:
                    if str(l).strip("\n") not in self.otherKeysList:
                        self.otherKeysList.append(str(l).strip("\n"))
                file.close()
        if authorRole.exists():
            for t in list(authorRole.glob('**/*.txt')):
                file = open(str(t), "r", errors="replace")
                for l in file:
                    if str(l).strip("\n") not in self.authorRoleList:
                        self.authorRoleList.append(str(l).strip("\n"))
                file.close()

    """
    Refill the ratings box.
    This is called whenever the rating system changes.
    """

    def slot_refill_ratings(self):
        if self.cmbRatingSystem.currentText() in self.ratingKeysList.keys():
            self.cmbRating.clear()
            model = QStandardItemModel()
            for i in self.ratingKeysList[self.cmbRatingSystem.currentText()]:
                item = QStandardItem()
                item.setText(i[0])
                item.setToolTip(i[1])
                model.appendRow(item)
            self.cmbRating.setModel(model)

    """
    Add an author with default values initialised.
    """

    def slot_add_author(self):
        listItems = []
        listItems.append(QStandardItem(i18n("Anon")))  # Nick name
        listItems.append(QStandardItem(i18n("John")))  # First name
        listItems.append(QStandardItem())  # Middle name
        listItems.append(QStandardItem(i18n("Doe")))  # Last name
        listItems.append(QStandardItem())  # role
        listItems.append(QStandardItem())  # email
        listItems.append(QStandardItem())  # homepage
        language = QLocale.system().name().split("_")[0]
        if language == "C":
            language = "en"
        listItems.append(QStandardItem(language))  # Language
        self.authorModel.appendRow(listItems)

    """
    Remove the selected author from the author list.
    """

    def slot_remove_author(self):
        self.authorModel.removeRow(self.authorTable.currentIndex().row())

    """
    Load the UI values from the config dictionary given.
    """

    def setConfig(self, config):

        if "title" in config.keys():
            self.lnTitle.setText(config["title"])
        self.teSummary.clear()
        if "pages" in config.keys():
            self.cmbCoverPage.clear()
            for page in config["pages"]:
                self.cmbCoverPage.addItem(page)
            if "cover" in config.keys():
                if config["cover"] in config["pages"]:
                    self.cmbCoverPage.setCurrentText(config["cover"])
        if "summary" in config.keys():
            self.teSummary.appendPlainText(config["summary"])
        if "genre" in config.keys():
            genreList = []
            genreListConf = config["genre"]
            totalMatch = 100
            if isinstance(config["genre"], dict):
                genreListConf = config["genre"].keys()
                totalMatch = 0
            for genre in genreListConf:
                genreKey = genre
                if genre in self.acbfGenreList:
                    genreKey = self.acbfGenreList[genre]
                if isinstance(config["genre"], dict):
                    genreValue = config["genre"][genre]
                    if genreValue > 0:
                        genreKey = str(genreKey + "(" + str(genreValue) + ")")
                genreList.append(genreKey)
            self.lnGenre.setText(", ".join(genreList))
        if "characters" in config.keys():
            self.lnCharacters.setText(", ".join(config["characters"]))
        if "format" in config.keys():
            self.lnFormat.setText(", ".join(config["format"]))
        if "rating" in config.keys():
            self.cmbRating.setCurrentText(config["rating"])
        else:
            self.cmbRating.setCurrentText("")
        if "ratingSystem" in config.keys():
            self.cmbRatingSystem.setCurrentText(config["ratingSystem"])
        else:
            self.cmbRatingSystem.setCurrentText("")
        if "otherKeywords" in config.keys():
            self.lnOtherKeywords.setText(", ".join(config["otherKeywords"]))
        if "seriesName" in config.keys():
            self.lnSeriesName.setText(config["seriesName"])
        if "seriesVolume" in config.keys():
            self.spnSeriesVol.setValue(config["seriesVolume"])
        if "seriesNumber" in config.keys():
            self.spnSeriesNumber.setValue(config["seriesNumber"])
        if "language" in config.keys():
            code = config["language"]
            if "_" in code:
                self.cmbLanguage.setEntryToCode(code.split("_")[0])
                self.cmbCountry.setEntryToCode(code.split("_")[-1])
            elif "-" in code:
                self.cmbLanguage.setEntryToCode(code.split("-")[0])
                self.cmbCountry.setEntryToCode(code.split("-")[-1])
            else:
                self.cmbLanguage.setEntryToCode(code)
        if "readingDirection" in config.keys():
            if config["readingDirection"] == "leftToRight":
                self.cmbReadingMode.setCurrentIndex(int(Qt.LeftToRight))
            else:
                self.cmbReadingMode.setCurrentIndex(int(Qt.RightToLeft))
        else:
            self.cmbReadingMode.setCurrentIndex(QLocale(self.cmbLanguage.codeForCurrentEntry()).textDirection())
        if "publisherName" in config.keys():
            self.publisherName.setText(config["publisherName"])
        if "publisherCity" in config.keys():
            self.publishCity.setText(config["publisherCity"])
        if "publishingDate" in config.keys():
            self.publishDate.setDate(QDate.fromString(config["publishingDate"], Qt.ISODate))
        if "isbn-number" in config.keys():
            self.isbn.setText(config["isbn-number"])
        if "source" in config.keys():
            self.ln_source.setText(config["source"])
        elif "acbfSource" in config.keys():
            self.ln_source.setText(config["acbfSource"])
        if "uuid" in config.keys():
            self.label_uuid.setText(config["uuid"])
        else:
            uuid = str()
            if "acbfID" in config.keys():
                uuid = config["acbfID"]
                uuid = uuid.strip("{")
                uuid = uuid.strip("}")
                uuidVerify = uuid.split("-")
                if len(uuidVerify[0])!=8 or len(uuidVerify[1])!=4 or len(uuidVerify[2])!=4 or len(uuidVerify[3])!=4 or len(uuidVerify[4])!=12:
                    uuid = QUuid.createUuid().toString()
            self.label_uuid.setText(uuid)
            config["uuid"] = uuid
        if "license" in config.keys():
            self.license.setCurrentText(config["license"])
        else:
            self.license.setCurrentText("")  # I would like to keep it ambiguous whether the artist has thought about the license or not.
        if "authorList" in config.keys():
            authorList = config["authorList"]
            for i in range(len(authorList)):
                author = authorList[i]
                if len(author.keys()) > 0:
                    listItems = []
                    listItems = []
                    listItems.append(QStandardItem(author.get("nickname", "")))
                    listItems.append(QStandardItem(author.get("first-name", "")))
                    listItems.append(QStandardItem(author.get("initials", "")))
                    listItems.append(QStandardItem(author.get("last-name", "")))
                    role = author.get("role", "")
                    if role in self.acbfAuthorRolesList.keys():
                        role = self.acbfAuthorRolesList[role]
                    listItems.append(QStandardItem(role))
                    listItems.append(QStandardItem(author.get("email", "")))
                    listItems.append(QStandardItem(author.get("homepage", "")))
                    listItems.append(QStandardItem(author.get("language", "")))
                    self.authorModel.appendRow(listItems)
        else:
            self.slot_add_author()
        dbRef = config.get("databaseReference", {})
        self.ln_database_name.setText(dbRef.get("name", ""))
        self.ln_database_entry.setText(dbRef.get("entry", ""))
        stringCmbEntryType = self.cmb_entry_type.itemText(0)
        self.cmb_entry_type.setCurrentText(dbRef.get("type", stringCmbEntryType))

    """
    Store the GUI values into the config dictionary given.
    
    @return the config diactionary filled with new values.
    """

    def getConfig(self, config):

        text = self.lnTitle.text()
        if len(text) > 0 and text.isspace() is False:
            config["title"] = text
        elif "title" in config.keys():
            config.pop("title")
        config["cover"] = self.cmbCoverPage.currentText()
        listkeys = self.lnGenre.text()
        if len(listkeys) > 0 and listkeys.isspace() is False:
            preSplit = self.lnGenre.text().split(",")
            genreMatcher = re.compile(r'\((\d+)\)')
            genreList = {}
            totalValue = 0
            for key in preSplit:
                m = genreMatcher.search(key)
                if m:
                    genre = str(genreMatcher.sub("", key)).strip()
                    match = int(m.group()[:-1][1:])
                else:
                    genre = key.strip()
                    match = 0
                if genre in self.acbfGenreList.values():
                    i = list(self.acbfGenreList.values()).index(genre)
                    genreList[list(self.acbfGenreList.keys())[i]] = match
                else:
                    genreList[genre] = match
                totalValue += match
            # Normalize the values:
            for key in genreList.keys():
                if genreList[key] > 0:
                    genreList[key] = round(genreList[key] / totalValue * 100)
            config["genre"] = genreList
        elif "genre" in config.keys():
            config.pop("genre")
        listkeys = self.lnCharacters.text()
        if len(listkeys) > 0 and listkeys.isspace() is False:
            config["characters"] = self.lnCharacters.text().split(", ")
        elif "characters" in config.keys():
            config.pop("characters")
        listkeys = self.lnFormat.text()
        if len(listkeys) > 0 and listkeys.isspace() is False:
            config["format"] = self.lnFormat.text().split(", ")
        elif "format" in config.keys():
            config.pop("format")
        config["ratingSystem"] = self.cmbRatingSystem.currentText()
        config["rating"] = self.cmbRating.currentText()
        listkeys = self.lnOtherKeywords.text()
        if len(listkeys) > 0 and listkeys.isspace() is False:
            config["otherKeywords"] = self.lnOtherKeywords.text().split(", ")
        elif "otherKeywords" in config.keys():
            config.pop("otherKeywords")
        text = self.teSummary.toPlainText()
        if len(text) > 0 and text.isspace() is False:
            config["summary"] = text
        elif "summary" in config.keys():
            config.pop("summary")
        if len(self.lnSeriesName.text()) > 0:
            config["seriesName"] = self.lnSeriesName.text()
            config["seriesNumber"] = self.spnSeriesNumber.value()
            if self.spnSeriesVol.value() > 0:
                config["seriesVolume"] = self.spnSeriesVol.value()
        config["language"] = str(self.cmbLanguage.codeForCurrentEntry()+"-"+self.cmbCountry.codeForCurrentEntry())
        if self.cmbReadingMode.currentIndex() is int(Qt.LeftToRight):
            config["readingDirection"] = "leftToRight"
        else:
            config["readingDirection"] = "rightToLeft"
        authorList = []
        for row in range(self.authorTable.verticalHeader().count()):
            logicalIndex = self.authorTable.verticalHeader().logicalIndex(row)
            listEntries = ["nickname", "first-name", "initials", "last-name", "role", "email", "homepage", "language"]
            author = {}
            for i in range(len(listEntries)):
                entry = self.authorModel.data(self.authorModel.index(logicalIndex, i))
                if entry is None:
                    entry = " "
                if entry.isspace() is False and len(entry) > 0:
                    if listEntries[i] == "role":
                        if entry in self.acbfAuthorRolesList.values():
                            entryI = list(self.acbfAuthorRolesList.values()).index(entry)
                            entry = list(self.acbfAuthorRolesList.keys())[entryI]
                    author[listEntries[i]] = entry
                elif listEntries[i] in author.keys():
                    author.pop(listEntries[i])
            authorList.append(author)
        config["authorList"] = authorList
        config["publisherName"] = self.publisherName.text()
        config["publisherCity"] = self.publishCity.text()
        config["publishingDate"] = self.publishDate.date().toString(Qt.ISODate)
        config["isbn-number"] = self.isbn.text()
        config["source"] = self.ln_source.text()
        config["license"] = self.license.currentText()
        if self.ln_database_name.text().isalnum() and self.ln_database_entry.text().isalnum():
            dbRef = {}
            dbRef["name"] = self.ln_database_name.text()
            dbRef["entry"] = self.ln_database_entry.text()
            dbRef["type"] = self.cmb_entry_type.currentText()
            config["databaseReference"] = dbRef

        return config
