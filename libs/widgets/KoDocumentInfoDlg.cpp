/* This file is part of the KDE project
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>
                 2006 Martin Pfeiffer <hubipete@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoDocumentInfoDlg.h"

#include "ui_koDocumentInfoAboutWidget.h"
#include "ui_koDocumentInfoAuthorWidget.h"

#include "KoDocumentInfo.h"
#include "KoDocumentBase.h"
#include "KoGlobal.h"
#include "KoPageWidgetItem.h"
#include <KoIcon.h>


#include <klocalizedstring.h>

#include <kmainwindow.h>
#include <KoDialog.h>
#include <QUrl>
#include <QCompleter>
#include <QLineEdit>
#include <QDateTime>
#include <KisMimeDatabase.h>

class KoPageWidgetItemAdapter : public KPageWidgetItem
{
public:
    KoPageWidgetItemAdapter(KoPageWidgetItem *item)
        : KPageWidgetItem(item->widget(), item->name())
        , m_item(item)
    {
        setHeader(item->name());
        setIcon(KisIconUtils::loadIcon(item->iconName()));
    }
    ~KoPageWidgetItemAdapter() override { delete m_item; }

    bool shouldDialogCloseBeVetoed() { return m_item->shouldDialogCloseBeVetoed(); }
    void apply() { m_item->apply(); }

private:
    KoPageWidgetItem * const m_item;
};


class KoDocumentInfoDlg::KoDocumentInfoDlgPrivate
{
public:
    KoDocumentInfoDlgPrivate() :
        documentSaved(false) {}
    ~KoDocumentInfoDlgPrivate() {}

    KoDocumentInfo* info;
    QList<KPageWidgetItem*> pages;
    Ui::KoDocumentInfoAboutWidget* aboutUi;
    Ui::KoDocumentInfoAuthorWidget* authorUi;

    bool documentSaved;
};


KoDocumentInfoDlg::KoDocumentInfoDlg(QWidget* parent, KoDocumentInfo* docInfo)
    : KPageDialog(parent)
    , d(new KoDocumentInfoDlgPrivate)
{
    d->info = docInfo;

    setWindowTitle(i18n("Document Information"));
    //    setInitialSize(QSize(500, 500));
    setFaceType(KPageDialog::List);
    setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    button(QDialogButtonBox::Ok)->setDefault(true);

    d->aboutUi = new Ui::KoDocumentInfoAboutWidget();
    QWidget *infodlg = new QWidget();
    d->aboutUi->setupUi(infodlg);
    d->aboutUi->cbLanguage->addItems(KoGlobal::listOfLanguages());
    d->aboutUi->cbLanguage->setCurrentIndex(-1);
    QStringList licenseExamples;
    licenseExamples << "CC-BY 4.0" << "CC-BY-SA 4.0" << "CC-BY-SA-NC 4.0" << "CC-0";
    QCompleter *examples = new QCompleter(licenseExamples);
    examples->setCaseSensitivity(Qt::CaseInsensitive);
    examples->setCompletionMode(QCompleter::PopupCompletion);
    d->aboutUi->leLicense->setCompleter(examples);

    KPageWidgetItem *page = new KPageWidgetItem(infodlg, i18n("General"));
    page->setHeader(i18n("General"));

    // Ugly hack, the mimetype should be a parameter, instead
    KoDocumentBase* doc = dynamic_cast< KoDocumentBase* >(d->info->parent());
    if (doc) {
        page->setIcon(KisIconUtils::loadIcon(KisMimeDatabase::iconNameForMimeType(doc->mimeType())));
    } else {
        // hide all entries not used in pages for KoDocumentInfoPropsPage
        d->aboutUi->filePathInfoLabel->setVisible(false);
        d->aboutUi->filePathLabel->setVisible(false);
        d->aboutUi->filePathSeparatorLine->setVisible(false);
        d->aboutUi->lblTypeDesc->setVisible(false);
        d->aboutUi->lblType->setVisible(false);
    }
    addPage(page);
    d->pages.append(page);

    initAboutTab();

    d->authorUi = new Ui::KoDocumentInfoAuthorWidget();
    QWidget *authordlg = new QWidget();
    d->authorUi->setupUi(authordlg);
    page = new KPageWidgetItem(authordlg, i18n("Author"));
    page->setHeader(i18n("Last saved by"));
    page->setIcon(koIcon("user-identity"));
    addPage(page);
    d->pages.append(page);

    initAuthorTab();
}

KoDocumentInfoDlg::~KoDocumentInfoDlg()
{
    delete d->authorUi;
    delete d->aboutUi;
    delete d;
}

void KoDocumentInfoDlg::accept()
{
    // check if any pages veto the close
    Q_FOREACH (KPageWidgetItem* item, d->pages) {
        KoPageWidgetItemAdapter *page = dynamic_cast<KoPageWidgetItemAdapter*>(item);
        if (page) {
            if (page->shouldDialogCloseBeVetoed()) {
                return;
            }
        }
    }

    // all fine, go and apply
    saveAboutData();
    Q_FOREACH (KPageWidgetItem* item, d->pages) {
        KoPageWidgetItemAdapter *page = dynamic_cast<KoPageWidgetItemAdapter*>(item);
        if (page) {
            page->apply();
        }
    }

    KPageDialog::accept();
}

bool KoDocumentInfoDlg::isDocumentSaved()
{
    return d->documentSaved;
}

void KoDocumentInfoDlg::initAboutTab()
{
    KoDocumentBase* doc = dynamic_cast< KoDocumentBase* >(d->info->parent());

    if (doc) {
        d->aboutUi->filePathLabel->setText(doc->localFilePath());
    }

    d->aboutUi->leTitle->setText(d->info->aboutInfo("title"));
    d->aboutUi->leSubject->setText(d->info->aboutInfo("subject"));
    QString language = KoGlobal::languageFromTag(d->info->aboutInfo("language"));
    d->aboutUi->cbLanguage->setCurrentIndex(d->aboutUi->cbLanguage->findText(language));

    d->aboutUi->leKeywords->setToolTip(i18n("Use ';' (Example: Office;KDE;Calligra)"));
    if (!d->info->aboutInfo("keyword").isEmpty())
        d->aboutUi->leKeywords->setText(d->info->aboutInfo("keyword"));

    if (!d->info->aboutInfo("license").isEmpty()) {
        d->aboutUi->leLicense->setText(d->info->aboutInfo("license"));
    }
    d->aboutUi->meDescription->setPlainText(d->info->aboutInfo("abstract"));
    if (doc && !doc->mimeType().isEmpty()) {
        d->aboutUi->lblType->setText(KisMimeDatabase::descriptionForMimeType(doc->mimeType()));
    }
    if (!d->info->aboutInfo("creation-date").isEmpty()) {
        QDateTime t = QDateTime::fromString(d->info->aboutInfo("creation-date"),
                                            Qt::ISODate);
        QString s = QLocale().toString(t);
        d->aboutUi->lblCreated->setText(s + ", " +
                                        d->info->aboutInfo("initial-creator"));
    }

    if (!d->info->aboutInfo("date").isEmpty()) {
        QDateTime t = QDateTime::fromString(d->info->aboutInfo("date"), Qt::ISODate);
        QString s = QLocale().toString(t);
        d->aboutUi->lblModified->setText(s + ", " + d->info->authorInfo("creator"));
    }

    d->aboutUi->lblRevision->setText(d->info->aboutInfo("editing-cycles"));

    updateEditingTime();

    connect(d->aboutUi->pbReset, SIGNAL(clicked()),
            this, SLOT(slotResetMetaData()));
}

void KoDocumentInfoDlg::initAuthorTab()
{
    d->authorUi->nickName->setText(d->info->authorInfo("creator"));
    d->authorUi->firstName->setText(d->info->authorInfo("creator-first-name"));
    d->authorUi->lastName->setText(d->info->authorInfo("creator-last-name"));
    d->authorUi->initials->setText(d->info->authorInfo("initial"));
    d->authorUi->title->setText(d->info->authorInfo("author-title"));
    d->authorUi->company->setText(d->info->authorInfo("company"));
    d->authorUi->position->setText(d->info->authorInfo("position"));
    QListWidget *contact = d->authorUi->leContact;
    Q_FOREACH(QString contactMode, d->info->authorContactInfo()) {
        if (!contactMode.isEmpty()) {
            contact->addItem(contactMode);
        }
    }
}

void KoDocumentInfoDlg::saveAboutData()
{
    d->info->setAboutInfo("keyword", d->aboutUi->leKeywords->text());
    d->info->setAboutInfo("title", d->aboutUi->leTitle->text());
    d->info->setAboutInfo("subject", d->aboutUi->leSubject->text());
    d->info->setAboutInfo("abstract", d->aboutUi->meDescription->toPlainText());
    d->info->setAboutInfo("license", d->aboutUi->leLicense->text());
    d->info->setAboutInfo("language", KoGlobal::tagOfLanguage(d->aboutUi->cbLanguage->currentText()));
}

void KoDocumentInfoDlg::hideEvent( QHideEvent *event )
{
    Q_UNUSED(event);
}

void KoDocumentInfoDlg::slotResetMetaData()
{
    d->info->resetMetaData();

    if (!d->info->aboutInfo("creation-date").isEmpty()) {
        QDateTime t = QDateTime::fromString(d->info->aboutInfo("creation-date"),
                                            Qt::ISODate);
        QString s = QLocale().toString(t);
        d->aboutUi->lblCreated->setText(s + ", " +
                                        d->info->aboutInfo("initial-creator"));
    }

    if (!d->info->aboutInfo("date").isEmpty()) {
        QDateTime t = QDateTime::fromString(d->info->aboutInfo("date"), Qt::ISODate);
        QString s = QLocale().toString(t);
        d->aboutUi->lblModified->setText(s + ", " + d->info->authorInfo("creator"));
    }

    d->aboutUi->lblRevision->setText(d->info->aboutInfo("editing-cycles"));
}

QList<KPageWidgetItem*> KoDocumentInfoDlg::pages() const
{
    return d->pages;
}

void KoDocumentInfoDlg::setReadOnly(bool ro)
{
    d->aboutUi->meDescription->setReadOnly(ro);

    Q_FOREACH(KPageWidgetItem* page, d->pages) {
        Q_FOREACH(QLineEdit* le, page->widget()->findChildren<QLineEdit *>()) {
            le->setReadOnly(ro);
        }
        Q_FOREACH(QPushButton* le, page->widget()->findChildren<QPushButton *>()) {
            le->setDisabled(ro);
        }
    }
}

void KoDocumentInfoDlg::addPageItem(KoPageWidgetItem *item)
{
    KPageWidgetItem * page = new KoPageWidgetItemAdapter(item);

    addPage(page);
    d->pages.append(page);
}

void KoDocumentInfoDlg::updateEditingTime()
{
    const int timeElapsed = d->info->aboutInfo("editing-time").toInt();

    const int secondsElapsed = timeElapsed % 60;
    const int minutesElapsed = (timeElapsed / 60) % 60;
    const int hoursElapsed = (timeElapsed / 3600) % 24;
    const int daysElapsed = (timeElapsed / 86400) % 7;
    const int weeksElapsed = timeElapsed / 604800;

    QString majorTimeUnit;
    QString minorTimeUnit;

    if (weeksElapsed > 0) {
        majorTimeUnit = i18np("%1 week", "%1 weeks", weeksElapsed);
        minorTimeUnit = i18np("%1 day", "%1 days", daysElapsed);
    } else if (daysElapsed > 0) {
        majorTimeUnit = i18np("%1 day", "%1 days", daysElapsed);
        minorTimeUnit = i18np("%1 hour", "%1 hours", hoursElapsed);
    } else if (hoursElapsed > 0) {
        majorTimeUnit = i18np("%1 hour", "%1 hours", hoursElapsed);
        minorTimeUnit = i18np("%1 minute", "%1 minutes", minutesElapsed);
    } else if (minutesElapsed > 0) {
        majorTimeUnit = i18np("%1 minute", "%1 minutes", minutesElapsed);
        minorTimeUnit = i18np("%1 second", "%1 seconds", secondsElapsed);
    } else {
        d->aboutUi->lblEditing->setText(i18np("%1 second", "%1 seconds", secondsElapsed));
        return;
    }

    d->aboutUi->lblEditing->setText(i18nc(
                                        "major time unit and minor time unit",
                                        "%1 and %2",
                                        majorTimeUnit,
                                        minorTimeUnit
                                        ));
}
