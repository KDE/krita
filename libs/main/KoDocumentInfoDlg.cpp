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
#include "KoDocument.h"
#include "KoMainWindow.h"
#include "KoGlobal.h"
#include <KoEncryptionChecker.h>
#include "KoPageWidgetItem.h"
#include <KoDocumentRdfBase.h>
#include <KoIcon.h>

#include <kmimetype.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>


#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPixmap>
#include <QDateTime>


class KoDocumentInfoDlg::KoDocumentInfoDlgPrivate
{
public:
    KoDocumentInfoDlgPrivate() :
        toggleEncryption(false),
        applyToggleEncryption(false),
        documentSaved(false) {}
    ~KoDocumentInfoDlgPrivate() {}

    KoDocumentInfo* info;
    QList<KPageWidgetItem*> pages;
    Ui::KoDocumentInfoAboutWidget* aboutUi;
    Ui::KoDocumentInfoAuthorWidget* authorUi;

    bool toggleEncryption;
    bool applyToggleEncryption;
    bool documentSaved;
};


KoDocumentInfoDlg::KoDocumentInfoDlg(QWidget* parent, KoDocumentInfo* docInfo)
    : KPageDialog(parent)
    , d(new KoDocumentInfoDlgPrivate)
{
    d->info = docInfo;

    setCaption(i18n("Document Information"));
    setInitialSize(QSize(500, 500));
    setFaceType(KPageDialog::List);
    setButtons(KDialog::Ok | KDialog::Cancel);
    setDefaultButton(KDialog::Ok);

    d->aboutUi = new Ui::KoDocumentInfoAboutWidget();
    QWidget *infodlg = new QWidget();
    d->aboutUi->setupUi(infodlg);
    if (!KoEncryptionChecker::isEncryptionSupported()) {
        d->aboutUi->lblEncryptedDesc->setVisible(false);
        d->aboutUi->lblEncrypted->setVisible(false);
        d->aboutUi->pbEncrypt->setVisible(false);
        d->aboutUi->lblEncryptedPic->setVisible(false);
    }
    d->aboutUi->cbLanguage->addItems(KoGlobal::listOfLanguages());
    d->aboutUi->cbLanguage->setCurrentIndex(-1);

    KPageWidgetItem *page = new KPageWidgetItem(infodlg, i18n("General"));
    page->setHeader(i18n("General"));

    // Ugly hack, the mimetype should be a parameter, instead
    KoDocument* doc = dynamic_cast< KoDocument* >(d->info->parent());
    if (doc) {
        KMimeType::Ptr mime = KMimeType::mimeType(doc->mimeType());
        if (! mime)
            mime = KMimeType::defaultMimeTypePtr();
        page->setIcon(KIcon(mime->iconName()));
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

    // Saving encryption implies saving the document, this is done after closing the dialog
    connect(this, SIGNAL(hidden()), this, SLOT(slotSaveEncryption()));

}

KoDocumentInfoDlg::~KoDocumentInfoDlg()
{
    delete d->authorUi;
    delete d->aboutUi;
    delete d;
}

void KoDocumentInfoDlg::slotButtonClicked(int button)
{
    emit buttonClicked(static_cast<KDialog::ButtonCode>(button));
    switch (button) {
    case Ok:
        foreach(KPageWidgetItem* item, d->pages) {
            KoPageWidgetItem *page = dynamic_cast<KoPageWidgetItem*>(item);
            if (page) {
                if (page->shouldDialogCloseBeVetoed()) {
                    return;
                }
            }
        }
        slotApply();
        accept();
        return;
    }
    KPageDialog::slotButtonClicked(button);
}

bool KoDocumentInfoDlg::isDocumentSaved()
{
    return d->documentSaved;
}

void KoDocumentInfoDlg::initAboutTab()
{
    KoDocument* doc = dynamic_cast< KoDocument* >(d->info->parent());

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

    d->aboutUi->meComments->setPlainText(d->info->aboutInfo("description"));
    if (doc && !doc->mimeType().isEmpty()) {
        KMimeType::Ptr docmime = KMimeType::mimeType(doc->mimeType());
        if (docmime)
            d->aboutUi->lblType->setText(docmime->comment());
    }
    if (!d->info->aboutInfo("creation-date").isEmpty()) {
        QDateTime t = QDateTime::fromString(d->info->aboutInfo("creation-date"),
                                            Qt::ISODate);
        QString s = KGlobal::locale()->formatDateTime(t);
        d->aboutUi->lblCreated->setText(s + ", " +
                                        d->info->aboutInfo("initial-creator"));
    }

    if (!d->info->aboutInfo("date").isEmpty()) {
        QDateTime t = QDateTime::fromString(d->info->aboutInfo("date"), Qt::ISODate);
        QString s = KGlobal::locale()->formatDateTime(t);
        d->aboutUi->lblModified->setText(s + ", " + d->info->authorInfo("creator"));
    }

    d->aboutUi->lblRevision->setText(d->info->aboutInfo("editing-cycles"));

    if (doc && (doc->supportedSpecialFormats() & KoDocument::SaveEncrypted)) {
        if (doc->specialOutputFlag() == KoDocument::SaveEncrypted) {
            if (d->toggleEncryption) {
                d->aboutUi->lblEncrypted->setText(i18n("This document will be decrypted"));
                d->aboutUi->lblEncryptedPic->setPixmap(koSmallIcon("object-unlocked"));
                d->aboutUi->pbEncrypt->setText(i18n("Do not decrypt"));
            } else {
                d->aboutUi->lblEncrypted->setText(i18n("This document is encrypted"));
                d->aboutUi->lblEncryptedPic->setPixmap(koSmallIcon("object-locked"));
                d->aboutUi->pbEncrypt->setText(i18n("D&ecrypt"));
            }
        } else {
            if (d->toggleEncryption) {
                d->aboutUi->lblEncrypted->setText(i18n("This document will be encrypted."));
                d->aboutUi->lblEncryptedPic->setPixmap(koSmallIcon("object-locked"));
                d->aboutUi->pbEncrypt->setText(i18n("Do not encrypt"));
            } else {
                d->aboutUi->lblEncrypted->setText(i18n("This document is not encrypted"));
                d->aboutUi->lblEncryptedPic->setPixmap(koSmallIcon("object-unlocked"));
                d->aboutUi->pbEncrypt->setText(i18n("&Encrypt"));
            }
        }
    } else {
        d->aboutUi->lblEncrypted->setText(i18n("This document does not support encryption"));
        d->aboutUi->pbEncrypt->setEnabled( false );
    }
    connect(d->aboutUi->pbReset, SIGNAL(clicked()),
            this, SLOT(slotResetMetaData()));
    connect(d->aboutUi->pbEncrypt, SIGNAL(clicked()),
            this, SLOT(slotToggleEncryption()));
}

void KoDocumentInfoDlg::initAuthorTab()
{
    d->authorUi->fullName->setText(d->info->authorInfo("creator"));
    d->authorUi->initials->setText(d->info->authorInfo("initial"));
    d->authorUi->title->setText(d->info->authorInfo("author-title"));
    d->authorUi->company->setText(d->info->authorInfo("company"));
    d->authorUi->email->setText(d->info->authorInfo("email"));
    d->authorUi->phoneWork->setText(d->info->authorInfo("telephone-work"));
    d->authorUi->phoneHome->setText(d->info->authorInfo("telephone"));
    d->authorUi->fax->setText(d->info->authorInfo("fax"));
    d->authorUi->country->setText(d->info->authorInfo("country"));
    d->authorUi->postal->setText(d->info->authorInfo("postal-code"));
    d->authorUi->city->setText(d->info->authorInfo("city"));
    d->authorUi->street->setText(d->info->authorInfo("street"));
    d->authorUi->position->setText(d->info->authorInfo("position"));
}

void KoDocumentInfoDlg::slotApply()
{
    saveAboutData();
    foreach(KPageWidgetItem* item, d->pages) {
        KoPageWidgetItem *page = dynamic_cast<KoPageWidgetItem*>(item);
        if (page) {
            page->apply();
        }
    }
}

void KoDocumentInfoDlg::saveAboutData()
{
    d->info->setAboutInfo("keyword", d->aboutUi->leKeywords->text());
    d->info->setAboutInfo("title", d->aboutUi->leTitle->text());
    d->info->setAboutInfo("subject", d->aboutUi->leSubject->text());
    d->info->setAboutInfo("description", d->aboutUi->meComments->toPlainText());
    d->info->setAboutInfo("language", KoGlobal::tagOfLanguage(d->aboutUi->cbLanguage->currentText()));
    d->applyToggleEncryption = d->toggleEncryption;
}

void KoDocumentInfoDlg::slotResetMetaData()
{
    d->info->resetMetaData();

    if (!d->info->aboutInfo("creation-date").isEmpty()) {
        QDateTime t = QDateTime::fromString(d->info->aboutInfo("creation-date"),
                                            Qt::ISODate);
        QString s = KGlobal::locale()->formatDateTime(t);
        d->aboutUi->lblCreated->setText(s + ", " +
                                        d->info->aboutInfo("initial-creator"));
    }

    if (!d->info->aboutInfo("date").isEmpty()) {
        QDateTime t = QDateTime::fromString(d->info->aboutInfo("date"), Qt::ISODate);
        QString s = KGlobal::locale()->formatDateTime(t);
        d->aboutUi->lblModified->setText(s + ", " + d->info->authorInfo("creator"));
    }

    d->aboutUi->lblRevision->setText(d->info->aboutInfo("editing-cycles"));
}

void KoDocumentInfoDlg::slotToggleEncryption()
{
    KoDocument* doc = dynamic_cast< KoDocument* >(d->info->parent());
    if (!doc)
        return;

    d->toggleEncryption = !d->toggleEncryption;

    if (doc->specialOutputFlag() == KoDocument::SaveEncrypted) {
        if (d->toggleEncryption) {
            d->aboutUi->lblEncrypted->setText(i18n("This document will be decrypted"));
            d->aboutUi->lblEncryptedPic->setPixmap(koSmallIcon("object-unlocked"));
            d->aboutUi->pbEncrypt->setText(i18n("Do not decrypt"));
        } else {
            d->aboutUi->lblEncrypted->setText(i18n("This document is encrypted"));
            d->aboutUi->lblEncryptedPic->setPixmap(koSmallIcon("object-locked"));
            d->aboutUi->pbEncrypt->setText(i18n("D&ecrypt"));
        }
    } else {
        if (d->toggleEncryption) {
            d->aboutUi->lblEncrypted->setText(i18n("This document will be encrypted."));
            d->aboutUi->lblEncryptedPic->setPixmap(koSmallIcon("object-locked"));
            d->aboutUi->pbEncrypt->setText(i18n("Do not encrypt"));
        } else {
            d->aboutUi->lblEncrypted->setText(i18n("This document is not encrypted"));
            d->aboutUi->lblEncryptedPic->setPixmap(koSmallIcon("object-unlocked"));
            d->aboutUi->pbEncrypt->setText(i18n("&Encrypt"));
        }
    }
}

void KoDocumentInfoDlg::slotSaveEncryption()
{
    if (!d->applyToggleEncryption)
        return;

    KoDocument* doc = dynamic_cast< KoDocument* >(d->info->parent());
    if (!doc)
        return;
    KoMainWindow* mainWindow = dynamic_cast< KoMainWindow* >(parent());

    if (doc->specialOutputFlag() == KoDocument::SaveEncrypted) {
        // Decrypt
        if (KMessageBox::warningContinueCancel(
                    this,
                    i18n("<qt>Decrypting the document will remove the password protection from it."
                         "<p>Do you still want to decrypt the file?</qt>"),
                    i18n("Confirm Decrypt"),
                    KGuiItem(i18n("Decrypt")),
                    KStandardGuiItem::cancel(),
                    "DecryptConfirmation"
                    ) != KMessageBox::Continue) {
            return;
        }
        bool modified = doc->isModified();
        doc->setOutputMimeType(doc->outputMimeType(), doc->specialOutputFlag() & ~KoDocument::SaveEncrypted);
        if (!mainWindow) {
            KMessageBox::information(
                        this,
                        i18n("<qt>Your document could not be saved automatically."
                             "<p>To complete the decryption, please save the document.</qt>"),
                        i18n("Save Document"),
                        "DecryptSaveMessage");
            return;
        }
        if (modified && KMessageBox::questionYesNo(
                    this,
                    i18n("<qt>The document has been changed since it was opened. To complete the decryption the document needs to be saved."
                         "<p>Do you want to save the document now?</qt>"),
                    i18n("Save Document"),
                    KStandardGuiItem::save(),
                    KStandardGuiItem::dontSave(),
                    "DecryptSaveConfirmation"
                    ) != KMessageBox::Yes) {
            return;
        }
    } else {
        // Encrypt
        bool modified = doc->isModified();
        if (!doc->url().isEmpty() && !(doc->mimeType().startsWith("application/vnd.oasis.opendocument.") && doc->specialOutputFlag() == 0)) {
            KMimeType::Ptr mime = KMimeType::mimeType(doc->mimeType());
            QString comment = mime ? mime->comment() : i18n("%1 (unknown file type)", QString::fromLatin1(doc->mimeType()));
            if (KMessageBox::warningContinueCancel(
                        this,
                        i18n("<qt>The document is currently saved as %1. The document needs to be changed to <b>OASIS OpenDocument</b> to be encrypted."
                             "<p>Do you want to change the file to OASIS OpenDocument?</qt>", QString("<b>%1</b>").arg(comment)),
                        i18n("Change Filetype"),
                        KGuiItem(i18n("Change")),
                        KStandardGuiItem::cancel(),
                        "EncryptChangeFiletypeConfirmation"
                        ) != KMessageBox::Continue) {
                return;
            }
            doc->resetURL();
        }
        doc->setMimeType(doc->nativeOasisMimeType());
        doc->setOutputMimeType(doc->nativeOasisMimeType(), KoDocument::SaveEncrypted);
        if (!mainWindow) {
            KMessageBox::information(
                        this,
                        i18n("<qt>Your document could not be saved automatically."
                             "<p>To complete the encryption, please save the document.</qt>"),
                        i18n("Save Document"),
                        "EncryptSaveMessage");
            return;
        }
        if (modified && KMessageBox::questionYesNo(
                    this,
                    i18n("<qt>The document has been changed since it was opened. To complete the encryption the document needs to be saved."
                         "<p>Do you want to save the document now?</qt>"),
                    i18n("Save Document"),
                    KStandardGuiItem::save(),
                    KStandardGuiItem::dontSave(),
                    "EncryptSaveConfirmation"
                    ) != KMessageBox::Yes) {
            return;
        }
    }
    // Why do the dirty work ourselves?
    mainWindow->slotFileSave();
    d->toggleEncryption = false;
    d->applyToggleEncryption = false;
    // Detects when the user cancelled saving
    d->documentSaved = !doc->url().isEmpty();
}

QList<KPageWidgetItem*> KoDocumentInfoDlg::pages() const
{
    return d->pages;
}

void KoDocumentInfoDlg::setReadOnly(bool ro)
{
    d->aboutUi->meComments->setReadOnly(ro);

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
    KPageWidgetItem * page = new KPageWidgetItem(item->widget(), item->name());
    page->setHeader(item->name());
    page->setIcon(koIcon(item->icon()));

    addPage(page);
    d->pages.append(page);
}

#include <KoDocumentInfoDlg.moc>
