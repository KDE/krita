"""
Part of the comics project management tools (CPMT).

This is a metadata editor that helps out setting the proper metadata
"""
import sys
import os  # For finding the script location.
import csv
from pathlib import Path  # For reading all the files in a directory.
from PyQt5.QtGui import QStandardItem, QStandardItemModel
from PyQt5.QtWidgets import QComboBox, QCompleter, QStyledItemDelegate, QLineEdit, QDialog, QDialogButtonBox, QVBoxLayout, QFormLayout, QTabWidget, QWidget, QPlainTextEdit, QHBoxLayout, QSpinBox, QDateEdit, QPushButton, QLabel, QTableView
from PyQt5.QtCore import QDir, QLocale, QStringListModel, Qt, QDate
"""
mult entry completer cobbled together from the two examples on stackoverflow:3779720

This allows us to let people type in comma seperated lists and get completion for those.
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
        mainP = os.path.dirname(__file__)
        languageP = os.path.join(mainP, "isoLanguagesList.csv")
        if (os.path.exists(languageP)):
            file = open(languageP, "r", newline="", encoding="utf8")
            languageReader = csv.reader(file)
            for row in languageReader:
                self.languageList.append(row[0])
                self.codesList.append(row[1])
                self.addItem(row[0])
            file.close()

    def codeForCurrentEntry(self):
        if self.currentText() in self.languageList:
            return self.codesList[self.languageList.index(self.currentText())]

    def setEntryToCode(self, code):
        if (code == "C" and "en" in self.codesList):
            self.setCurrentIndex(self.codesList.index("en"))
        if code in self.codesList:
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
    acbfGenreList = {"science_fiction":str(i18n("Science Fiction")), "fantasy":str(i18n("Fantasy")), "adventure":str(i18n("Adventure")), "horror":str(i18n("Horror")), "mystery":str(i18n("Mystery")), "crime":str(i18n("Crime")), "military":str(i18n("Military")), "real_life":str(i18n("Real Life")), "superhero":str(i18n("Superhero")), "humor":str(i18n("Humor")), "western":str(i18n("Western")), "manga":str(i18n("Manga")), "politics":str(i18n("Politics")), "caricature":str(i18n("Caricature")), "sports":str(i18n("Sports")), "history":str(i18n("History")), "biography":str(i18n("Biography")), "education":str(i18n("Education")), "computer":str(i18n("Computer")), "religion":str(i18n("Religion")), "romance":str(i18n("Romance")), "children":str(i18n("Children")), "non-fiction":str(i18n("Non Fiction")), "adult":str(i18n("Adult")), "alternative":str(i18n("Alternative")), "other":str(i18n("Other"))}
    acbfAuthorRolesList = {"Writer":str(i18n("Writer")), "Adapter":str(i18n("Adapter")), "Artist":str(i18n("Artist")), "Penciller":str(i18n("Penciller")), "Inker":str(i18n("Inker")), "Colorist":str(i18n("Colorist")), "Letterer":str(i18n("Letterer")), "Cover Artist":str(i18n("Cover Artist")), "Photographer":str(i18n("Photographer")), "Editor":str(i18n("Editor")), "Assistant Editor":str(i18n("Assistant Editor")), "Translator":str(i18n("Translator")), "Other":str(i18n("Other"))}

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
        self.lnGenre.setToolTip(i18n("The genre of the work. Prefilled values are from the ACBF, but you can fill in your own. Seperate genres with commas. Try to limit the amount to about two or three"))

        self.lnCharacters = QLineEdit()
        characterCompletion = multi_entry_completer()
        characterCompletion.setModel(QStringListModel(self.characterKeysList))
        characterCompletion.setCaseSensitivity(False)
        characterCompletion.setFilterMode(Qt.MatchContains)  # So that if there is a list of names with last names, people can type in a last name.
        self.lnCharacters.setCompleter(characterCompletion)
        self.lnCharacters.setToolTip(i18n("The names of the characters that this comic revolves around. Comma seperate."))

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
        self.spnSeriesNumber.setPrefix("No. ")
        self.spnSeriesVol = QSpinBox()
        self.spnSeriesVol.setPrefix("Vol. ")
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
        self.lnOtherKeywords.setToolTip(i18n("Other keywords that don't fit in the previously mentioned sets. As always, comma seperate"))

        self.cmbLanguage = language_combo_box()
        self.cmbReadingMode = QComboBox()
        self.cmbReadingMode.addItem(i18n("Left to Right"))
        self.cmbReadingMode.addItem(i18n("Right to Left"))

        self.cmbCoverPage = QComboBox()
        self.cmbCoverPage.setToolTip(i18n("Which page is the cover page? This will be empty if there's no pages."))

        mformLayout.addRow(i18n("Title:"), self.lnTitle)
        mformLayout.addRow(i18n("Cover Page:"), self.cmbCoverPage)
        mformLayout.addRow(i18n("Summary:"), self.teSummary)
        mformLayout.addRow(i18n("Language:"), self.cmbLanguage)
        mformLayout.addRow(i18n("Reading Direction:"), self.cmbReadingMode)
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
        explaination = QLabel(i18n("The following is a table of the authors that contributed to this comic. You can set their nickname, proper names (first, middle, last), Role (Penciller, Inker, etc), email and homepage."))
        explaination.setWordWrap(True)
        self.authorModel = QStandardItemModel(0, 7)
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
        delegate.setLanguageData(len(labels)-1)
        self.authorTable.setItemDelegate(delegate)
        author_button_layout = QWidget()
        author_button_layout.setLayout(QHBoxLayout())
        btn_add_author = QPushButton(i18n("Add Author"))
        btn_add_author.clicked.connect(self.slot_add_author)
        btn_remove_author = QPushButton(i18n("Remove Author"))
        btn_remove_author.clicked.connect(self.slot_remove_author)
        author_button_layout.layout().addWidget(btn_add_author)
        author_button_layout.layout().addWidget(btn_remove_author)
        authorPage.layout().addWidget(explaination)
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
        publisherLayout.addRow(i18n("Name:"), self.publisherName)
        publisherLayout.addRow(i18n("City:"), self.publishCity)
        publisherLayout.addRow(i18n("Date:"), publishDateLayout)
        publisherLayout.addRow(i18n("ISBN:"), self.isbn)
        publisherLayout.addRow(i18n("License:"), self.license)

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
                    if r is 0:
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
            for genre in config["genre"]:
                if genre in self.acbfGenreList.keys():
                    genreList.append(self.acbfGenreList[genre])
                else:
                    genreList.append(genre)
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
                code = code.split("_")[0]
            self.cmbLanguage.setEntryToCode(code)
        if "readingDirection" in config.keys():
            if config["readingDirection"] is "leftToRight":
                self.cmbReadingMode.setCurrentIndex(0)
            else:
                self.cmbReadingMode.setCurrentIndex(1)
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
                    authorNickName = QStandardItem()
                    if "nickname" in author.keys():
                        authorNickName.setText(author["nickname"])
                    listItems.append(authorNickName)
                    authorFirstName = QStandardItem()
                    if "first-name" in author.keys():
                        authorFirstName.setText(author["first-name"])
                    listItems.append(authorFirstName)
                    authorMiddleName = QStandardItem()
                    if "initials" in author.keys():
                        authorMiddleName.setText(author["initials"])
                    listItems.append(authorMiddleName)
                    authorLastName = QStandardItem()
                    if "last-name" in author.keys():
                        authorLastName.setText(author["last-name"])
                    listItems.append(authorLastName)
                    authorRole = QStandardItem()
                    if "role" in author.keys():
                        role = author["role"]
                        if author["role"] in self.acbfAuthorRolesList.keys():
                            role = self.acbfAuthorRolesList[author["role"]]
                        authorRole.setText(role)
                    listItems.append(authorRole)
                    authorEMail = QStandardItem()
                    if "email" in author.keys():
                        authorEMail.setText(author["email"])
                    listItems.append(authorEMail)
                    authorHomePage = QStandardItem()
                    if "homepage" in author.keys():
                        authorHomePage.setText(author["homepage"])
                    listItems.append(authorHomePage)
                    authorLanguage = QStandardItem()
                    if "language" in author.keys():
                        authorLanguage.setText(author["language"])
                        pass
                    listItems.append(authorLanguage)
                    self.authorModel.appendRow(listItems)
        else:
            self.slot_add_author()

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
            genreList = []
            for genre in self.lnGenre.text().split(", "):
                if genre in self.acbfGenreList.values():
                    i = list(self.acbfGenreList.values()).index(genre)
                    genreList.append(list(self.acbfGenreList.keys())[i])
                else:
                    genreList.append(genre)
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
        elif "characters" in config.keys():
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
        config["language"] = str(self.cmbLanguage.codeForCurrentEntry())
        if self.cmbReadingMode is Qt.LeftToRight:
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
        config["license"] = self.license.currentText()

        return config
