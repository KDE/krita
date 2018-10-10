/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2007 Krzysztof Lichota (lichota@mimuw.edu.pl)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <QApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QEvent>
#include <QMap>
#include <QSettings>
#include <QSharedPointer>
#include <QStandardPaths>
#include <QDebug>

#include "kswitchlanguagedialog_p.h"

#include <klanguagebutton.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>

// Believe it or not we can't use KConfig from here
// (we need KConfig during QCoreApplication ctor which is too early for it)
// So we cooked a QSettings based solution
typedef QSharedPointer<QSettings> QSettingsPtr;

static QSettingsPtr localeOverridesSettings()
{
    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    const QDir configDir(configPath);
    if (!configDir.exists()) {
        configDir.mkpath(QStringLiteral("."));
    }
    return QSettingsPtr(new QSettings(configPath + QStringLiteral("/klanguageoverridesrc"), QSettings::IniFormat));
}

static QByteArray getApplicationSpecificLanguage(const QByteArray &defaultCode = QByteArray())
{
    QSettingsPtr settings = localeOverridesSettings();
    settings->beginGroup(QStringLiteral("Language"));
    //qDebug() << "our language" << settings->value(qAppName(), defaultCode).toByteArray();
    return settings->value(qAppName(), defaultCode).toByteArray();
}

static void setApplicationSpecificLanguage(const QByteArray &languageCode)
{
    QSettingsPtr settings = localeOverridesSettings();
    settings->beginGroup(QStringLiteral("Language"));

    if (languageCode.isEmpty()) {
        settings->remove(qAppName());
    } else {
        settings->setValue(qAppName(), languageCode);
    }
}

static void initializeLanguages()
{
    const QByteArray languageCode = getApplicationSpecificLanguage();

    if (!languageCode.isEmpty()) {
        QByteArray languages = qgetenv("LANGUAGE");
        if (languages.isEmpty()) {
            qputenv("LANGUAGE", languageCode);
        } else {
            qputenv("LANGUAGE", languageCode + ":" + languages);
        }
    }
    //qDebug() << ">>>>>>>>>>>>>> LANGUAGE" << qgetenv("LANGUAGE");
    //qDebug() << ">>>>>>>>>>>>>> DATADIRS" << qgetenv("XDG_DATA_DIRS");
}

Q_COREAPP_STARTUP_FUNCTION(initializeLanguages)

namespace KDEPrivate
{

struct LanguageRowData {
    LanguageRowData()
    {
        label = 0;
        languageButton = 0;
        removeButton = 0;
    }
    QLabel *label;
    KLanguageButton *languageButton;
    QPushButton *removeButton;

    void setRowWidgets(
        QLabel *label,
        KLanguageButton *languageButton,
        QPushButton *removeButton
    )
    {
        this->label = label;
        this->languageButton = languageButton;
        this->removeButton = removeButton;
    }

};

class KSwitchLanguageDialogPrivate
{
public:
    KSwitchLanguageDialogPrivate(KSwitchLanguageDialog *parent);

    KSwitchLanguageDialog *p; //parent class

    /**
        Fills language button with names of languages for which given application has translation.
    */
    void fillApplicationLanguages(KLanguageButton *button);

    /**
        Adds one button with language to widget.
    */
    void addLanguageButton(const QString &languageCode, bool primaryLanguage);

    /**
        Returns list of languages chosen for application or default languages is they are not set.
    */
    QStringList applicationLanguageList();

    QMap<QPushButton *, LanguageRowData> languageRows;
    QList<KLanguageButton *> languageButtons;
    QGridLayout *languagesLayout;
};

/*************************** KSwitchLanguageDialog **************************/

KSwitchLanguageDialog::KSwitchLanguageDialog(QWidget *parent)
    : QDialog(parent),
      d(new KSwitchLanguageDialogPrivate(this))
{
    setWindowTitle(i18n("Switch Application Language"));

    QVBoxLayout *topLayout = new QVBoxLayout;
    setLayout(topLayout);

    QLabel *label = new QLabel(i18n("Please choose the language which should be used for this application:"), this);
    topLayout->addWidget(label);

    QHBoxLayout *languageHorizontalLayout = new QHBoxLayout();
    topLayout->addLayout(languageHorizontalLayout);

    d->languagesLayout = new QGridLayout();
    languageHorizontalLayout->addLayout(d->languagesLayout);
    languageHorizontalLayout->addStretch();

    const QStringList defaultLanguages = d->applicationLanguageList();

    int count = defaultLanguages.count();
    for (int i = 0; i < count; ++i) {
        QString language = defaultLanguages[i];
        bool primaryLanguage = (i == 0);
        d->addLanguageButton(language, primaryLanguage);
    }

    if (!count) {
        QLocale l;
        d->addLanguageButton(l.name(), true);
    }

    QHBoxLayout *addButtonHorizontalLayout = new QHBoxLayout();
    topLayout->addLayout(addButtonHorizontalLayout);

    QPushButton *addLangButton = new QPushButton(i18n("Add Fallback Language"), this);
    addLangButton->setToolTip(i18n("Adds one more language which will be used if other translations do not contain a proper translation."));
    connect(addLangButton, SIGNAL(clicked()), this, SLOT(slotAddLanguageButton()));
    addButtonHorizontalLayout->addWidget(addLangButton);
    addButtonHorizontalLayout->addStretch();

    topLayout->addStretch(10);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok
                                  | QDialogButtonBox::Cancel
                                  | QDialogButtonBox::RestoreDefaults);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::RestoreDefaults), KStandardGuiItem::defaults());

    topLayout->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotOk()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked()),
            this, SLOT(slotDefault()));
}

KSwitchLanguageDialog::~KSwitchLanguageDialog()
{
    delete d;
}

void KSwitchLanguageDialog::slotAddLanguageButton()
{
    //adding new button with en_US as it should always be present
    d->addLanguageButton(QStringLiteral("en_US"), d->languageButtons.isEmpty());
}

void KSwitchLanguageDialog::removeButtonClicked()
{
    QObject const *signalSender = sender();
    if (!signalSender) {
        qCritical() << "KSwitchLanguageDialog::removeButtonClicked() called directly, not using signal" << endl;
        return;
    }

    QPushButton *removeButton = const_cast<QPushButton *>(::qobject_cast<const QPushButton *>(signalSender));
    if (!removeButton) {
        qCritical() << "KSwitchLanguageDialog::removeButtonClicked() called from something else than QPushButton" << endl;
        return;
    }

    QMap<QPushButton *, LanguageRowData>::iterator it = d->languageRows.find(removeButton);
    if (it == d->languageRows.end()) {
        qCritical() << "KSwitchLanguageDialog::removeButtonClicked called from unknown QPushButton" << endl;
        return;
    }

    LanguageRowData languageRowData = it.value();

    d->languageButtons.removeAll(languageRowData.languageButton);

    languageRowData.label->deleteLater();
    languageRowData.languageButton->deleteLater();
    languageRowData.removeButton->deleteLater();
    d->languageRows.erase(it);
}

void KSwitchLanguageDialog::languageOnButtonChanged(const QString &languageCode)
{
    Q_UNUSED(languageCode);
#if 0
    for (int i = 0, count = d->languageButtons.count(); i < count; ++i) {
        KLanguageButton *languageButton = d->languageButtons[i];
        if (languageButton->current() == languageCode) {
            //update all buttons which have matching id
            //might update buttons which were not changed, but well...
            languageButton->setText(KLocale::global()->languageCodeToName(languageCode));
        }
    }
#endif
}

void KSwitchLanguageDialog::slotOk()
{
    QStringList languages;

    for (int i = 0, count = d->languageButtons.count(); i < count; ++i) {
        KLanguageButton *languageButton = d->languageButtons[i];
        languages << languageButton->current();
    }

    if (d->applicationLanguageList() != languages) {
        QString languageString = languages.join(QLatin1Char(':'));
        //list is different from defaults or saved languages list
        setApplicationSpecificLanguage(languageString.toLatin1());

        QMessageBox::information(this,
                                 i18nc("@title:window:", "Application Language Changed"), //caption
                                 i18n("The language for this application has been changed. The change will take effect the next time the application is started."));
    }

    accept();
}

void KSwitchLanguageDialog::slotDefault()
{
    const QStringList defaultLanguages = d->applicationLanguageList();

    setApplicationSpecificLanguage(QByteArray());

    // read back the new default
    QString language = QString::fromLatin1(getApplicationSpecificLanguage("en_US"));

    if (defaultLanguages != (QStringList() << language)) {

        KMessageBox::information(
            this,
            i18n("The language for this application has been changed. The change will take effect the next time the application is started."), //text
            i18n("Application Language Changed"), //caption
            QStringLiteral("ApplicationLanguageChangedWarning") //dontShowAgainName
        );
    }

    accept();
}

/************************ KSwitchLanguageDialogPrivate ***********************/

KSwitchLanguageDialogPrivate::KSwitchLanguageDialogPrivate(
    KSwitchLanguageDialog *parent)
    : p(parent)
{
    //NOTE: do NOT use "p" in constructor, it is not fully constructed
}

static bool stripCountryCode(QString *languageCode)
{
    const int idx = languageCode->indexOf(QLatin1String("_"));
    if (idx != -1) {
        *languageCode = languageCode->left(idx);
        return true;
    }
    return false;
}

void KSwitchLanguageDialogPrivate::fillApplicationLanguages(KLanguageButton *button)
{
    QLocale defaultLocale;
    QLocale cLocale(QLocale::C);
    QLocale::setDefault(cLocale);
    QSet<QString> insertedLanguges;

    const QList<QLocale> allLocales = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);
    Q_FOREACH (const QLocale &l, allLocales) {
        QString languageCode = l.name();
        if (l != cLocale) {
            const QString nativeName = l.nativeLanguageName();
            // For some languages the native name might be empty.
            // In this case use the non native language name as fallback.
            // See: QTBUG-51323
            const QString languageName = nativeName.isEmpty() ? QLocale::languageToString(l.language()) : nativeName;
            if (!insertedLanguges.contains(languageCode) && KLocalizedString::isApplicationTranslatedInto(languageCode)) {
                button->insertLanguage(languageCode, languageName);
                insertedLanguges << languageCode;
            } else if (stripCountryCode(&languageCode)) {
                if (!insertedLanguges.contains(languageCode) && KLocalizedString::isApplicationTranslatedInto(languageCode)) {
                    button->insertLanguage(languageCode, languageName);
                    insertedLanguges << languageCode;
                }
            }
        }
    }

    QLocale::setDefault(defaultLocale);
}

QStringList KSwitchLanguageDialogPrivate::applicationLanguageList()
{
    QStringList languagesList;

    QByteArray languageCode = getApplicationSpecificLanguage();
    if (!languageCode.isEmpty()) {
        languagesList = QString::fromLatin1(languageCode).split(QLatin1Char(':'));
    }
    if (languagesList.isEmpty()) {
        QLocale l;
        languagesList = l.uiLanguages();

        // We get en-US here but we use en_US
        for (int i = 0; i < languagesList.count(); ++i) {
            languagesList[i].replace(QLatin1String("-"), QLatin1String("_"));
        }
    }

    for (int i = 0; i < languagesList.count();) {
        QString languageCode = languagesList[i];
        if (!KLocalizedString::isApplicationTranslatedInto(languageCode)) {
            if (stripCountryCode(&languageCode)) {
                if (KLocalizedString::isApplicationTranslatedInto(languageCode)) {
                    languagesList[i] = languageCode;
                    ++i;
                    continue;
                }
            }
            languagesList.removeAt(i);
        } else {
            ++i;
        }
    }

    return languagesList;
}

void KSwitchLanguageDialogPrivate::addLanguageButton(const QString &languageCode, bool primaryLanguage)
{
    QString labelText = primaryLanguage ? i18n("Primary language:") : i18n("Fallback language:");

    KLanguageButton *languageButton = new KLanguageButton(p);

    fillApplicationLanguages(languageButton);

    languageButton->setCurrentItem(languageCode);

    QObject::connect(
        languageButton,
        SIGNAL(activated(QString)),
        p,
        SLOT(languageOnButtonChanged(QString))
    );

    LanguageRowData languageRowData;
    QPushButton *removeButton = 0;

    if (!primaryLanguage) {
        removeButton = new QPushButton(i18n("Remove"), p);

        QObject::connect(
            removeButton,
            SIGNAL(clicked()),
            p,
            SLOT(removeButtonClicked())
        );
    }

    languageButton->setToolTip(primaryLanguage
                               ? i18n("This is the main application language which will be used first, before any other languages.")
                               : i18n("This is the language which will be used if any previous languages do not contain a proper translation."));

    int numRows = languagesLayout->rowCount();

    QLabel *languageLabel = new QLabel(labelText, p);
    languagesLayout->addWidget(languageLabel, numRows + 1, 1, Qt::AlignLeft);
    languagesLayout->addWidget(languageButton, numRows + 1, 2, Qt::AlignLeft);

    if (!primaryLanguage) {
        languagesLayout->addWidget(removeButton, numRows + 1, 3, Qt::AlignLeft);
        languageRowData.setRowWidgets(languageLabel, languageButton, removeButton);
        removeButton->show();
    }

    languageRows.insert(removeButton, languageRowData);

    languageButtons.append(languageButton);
    languageButton->show();
    languageLabel->show();
}

}
