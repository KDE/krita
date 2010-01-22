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

#include <kmimetype.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>

#ifdef KDEPIMLIBS_FOUND
#include <kabc/addressee.h>
#include <kabc/stdaddressbook.h>
#endif

#include <KoGlobal.h>
#include <KoEncryptionChecker.h>

#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPixmap>
#include <QDateTime>


class KoDocumentInfoDlg::KoDocumentInfoDlgPrivate
{
public:
    KoDocumentInfoDlgPrivate() :
            m_toggleEncryption(false),
            m_applyToggleEncryption(false),
            m_documentSaved(false) {}
    ~KoDocumentInfoDlgPrivate() {}

    KoDocumentInfo* m_info;
    Ui::KoDocumentInfoAboutWidget* m_aboutUi;
    Ui::KoDocumentInfoAuthorWidget* m_authorUi;

    bool m_toggleEncryption;
    bool m_applyToggleEncryption;
    bool m_documentSaved;
};


KoDocumentInfoDlg::KoDocumentInfoDlg(QWidget* parent, KoDocumentInfo* docInfo)
        : KPageDialog(parent)
        , d(new KoDocumentInfoDlgPrivate)
{
    d->m_info = docInfo;

    setCaption(i18n("Document Information"));
    setInitialSize(QSize(500, 500));
    setFaceType(KPageDialog::List);
    setButtons(KDialog::Ok | KDialog::Cancel);
    setDefaultButton(KDialog::Ok);

    d->m_aboutUi = new Ui::KoDocumentInfoAboutWidget();
    QWidget *infodlg = new QWidget();
    d->m_aboutUi->setupUi(infodlg);
    if (!KoEncryptionChecker::isEncryptionSupported()) {
        d->m_aboutUi->lblEncryptedDesc->setVisible(false);
        d->m_aboutUi->lblEncrypted->setVisible(false);
        d->m_aboutUi->pbEncrypt->setVisible(false);
        d->m_aboutUi->lblEncryptedPic->setVisible(false);
    }
    KPageWidgetItem *page = new KPageWidgetItem(infodlg, i18n("General"));
    page->setHeader(i18n("General"));
    KoDocument* doc = dynamic_cast< KoDocument* >(d->m_info->parent());
    KMimeType::Ptr mime = KMimeType::mimeType(doc->mimeType());
    if (! mime)
        mime = KMimeType::defaultMimeTypePtr();
    page->setIcon(KIcon(KIconLoader::global()->loadMimeTypeIcon(mime->iconName(), KIconLoader::Desktop, 48)));
    addPage(page);

    initAboutTab();

    d->m_authorUi = new Ui::KoDocumentInfoAuthorWidget();
    QWidget *authordlg = new QWidget();
    d->m_authorUi->setupUi(authordlg);
    page = new KPageWidgetItem(authordlg, i18n("Author"));
    page->setHeader(i18n("Author"));
    page->setIcon(KIcon("user-identity"));
    addPage(page);

    initAuthorTab();

    connect(this, SIGNAL(okClicked()), this, SLOT(slotApply()));
    // Saving encryption implies saving the document, this is done after closing the dialog
    connect(this, SIGNAL(hidden()), this, SLOT(slotSaveEncryption()));
}

KoDocumentInfoDlg::~KoDocumentInfoDlg()
{
    delete d->m_authorUi;
    delete d->m_aboutUi;
    delete d;
}

bool KoDocumentInfoDlg::isDocumentSaved()
{
    return d->m_documentSaved;
}

void KoDocumentInfoDlg::initAboutTab()
{
    KoDocument* doc = dynamic_cast< KoDocument* >(d->m_info->parent());
    if (!doc)
        return;

    d->m_aboutUi->filePathLabel->setText(doc->localFilePath());

    d->m_aboutUi->leTitle->setText(d->m_info->aboutInfo("title"));
    d->m_aboutUi->leSubject->setText(d->m_info->aboutInfo("subject"));

    d->m_aboutUi->leKeywords->setToolTip(i18n("Use ';' (Example: Office;KDE;KOffice)"));
    if (!d->m_info->aboutInfo("keyword").isEmpty())
        d->m_aboutUi->leKeywords->setText(d->m_info->aboutInfo("keyword"));

    d->m_aboutUi->meComments->setPlainText(d->m_info->aboutInfo("comments"));
    if (!doc->mimeType().isEmpty()) {
        KMimeType::Ptr docmime = KMimeType::mimeType(doc->mimeType());
        if (docmime)
            d->m_aboutUi->lblType->setText(docmime->comment());
    }
    if (!d->m_info->aboutInfo("creation-date").isEmpty()) {
        QDateTime t = QDateTime::fromString(d->m_info->aboutInfo("creation-date"),
                                            Qt::ISODate);
        QString s = KGlobal::locale()->formatDateTime(t);
        d->m_aboutUi->lblCreated->setText(s + ", " +
                                          d->m_info->aboutInfo("initial-creator"));
    }

    if (!d->m_info->aboutInfo("date").isEmpty()) {
        QDateTime t = QDateTime::fromString(d->m_info->aboutInfo("date"), Qt::ISODate);
        QString s = KGlobal::locale()->formatDateTime(t);
        d->m_aboutUi->lblModified->setText(s + ", " + d->m_info->authorInfo("creator"));
    }

    d->m_aboutUi->lblRevision->setText(d->m_info->aboutInfo("editing-cycles"));

    if ( doc->supportedSpecialFormats() & KoDocument::SaveEncrypted ) {
        if (doc->specialOutputFlag() == KoDocument::SaveEncrypted) {
            if (d->m_toggleEncryption) {
                QPixmap p = KIconLoader::global()->loadIcon("object-unlocked", KIconLoader::Small);
                d->m_aboutUi->lblEncrypted->setText(i18n("This document will be decrypted"));
                d->m_aboutUi->lblEncryptedPic->setPixmap(p);
                d->m_aboutUi->pbEncrypt->setText(i18n("Do not decrypt"));
            } else {
                QPixmap p = KIconLoader::global()->loadIcon("object-locked", KIconLoader::Small);
                d->m_aboutUi->lblEncrypted->setText(i18n("This document is encrypted"));
                d->m_aboutUi->lblEncryptedPic->setPixmap(p);
                d->m_aboutUi->pbEncrypt->setText(i18n("D&ecrypt"));
            }
        } else {
            if (d->m_toggleEncryption) {
                QPixmap p = KIconLoader::global()->loadIcon("object-locked", KIconLoader::Small);
                d->m_aboutUi->lblEncrypted->setText(i18n("This document will be encrypted."));
                d->m_aboutUi->lblEncryptedPic->setPixmap(p);
                d->m_aboutUi->pbEncrypt->setText(i18n("Do not encrypt"));
            } else {
                QPixmap p = KIconLoader::global()->loadIcon("object-unlocked", KIconLoader::Small);
                d->m_aboutUi->lblEncrypted->setText(i18n("This document is not encrypted"));
                d->m_aboutUi->lblEncryptedPic->setPixmap(p);
                d->m_aboutUi->pbEncrypt->setText(i18n("&Encrypt"));
            }
        }
    } else {
        d->m_aboutUi->lblEncrypted->setText(i18n("This document does not support encryption"));
        d->m_aboutUi->pbEncrypt->setEnabled( false );
    }
    connect(d->m_aboutUi->pbReset, SIGNAL(clicked()),
            this, SLOT(slotResetMetaData()));
    connect(d->m_aboutUi->pbEncrypt, SIGNAL(clicked()),
            this, SLOT(slotToggleEncryption()));
}

void KoDocumentInfoDlg::initAuthorTab()
{
    QPixmap p = KIconLoader::global()->loadIcon("office-address-book", KIconLoader::Small);
    d->m_authorUi->pbLoadKABC->setIcon(QIcon(p));
    p = KIconLoader::global()->loadIcon("edit-delete", KIconLoader::Small);
    d->m_authorUi->pbDelete->setIcon(QIcon(p));

    d->m_authorUi->leFullName->setText(d->m_info->authorInfo("creator"));
    d->m_authorUi->leInitials->setText(d->m_info->authorInfo("initial"));
    d->m_authorUi->leTitle->setText(d->m_info->authorInfo("author-title"));
    d->m_authorUi->leCompany->setText(d->m_info->authorInfo("company"));
    d->m_authorUi->leEmail->setText(d->m_info->authorInfo("email"));
    d->m_authorUi->lePhoneWork->setText(d->m_info->authorInfo("telephone-work"));
    d->m_authorUi->lePhoneHome->setText(d->m_info->authorInfo("telephone"));
    d->m_authorUi->leFax->setText(d->m_info->authorInfo("fax"));
    d->m_authorUi->leCountry->setText(d->m_info->authorInfo("country"));
    d->m_authorUi->lePostal->setText(d->m_info->authorInfo("postal-code"));
    d->m_authorUi->leCity->setText(d->m_info->authorInfo("city"));
    d->m_authorUi->leStreet->setText(d->m_info->authorInfo("street"));
    d->m_authorUi->lePosition->setText(d->m_info->authorInfo("position"));

#ifdef KDEPIMLIBS_FOUND
    connect(d->m_authorUi->pbLoadKABC, SIGNAL(clicked()),
            this, SLOT(slotLoadFromKABC()));
#else
    d->m_authorUi->pbLoadKABC->hide();
#endif

    connect(d->m_authorUi->pbDelete, SIGNAL(clicked()),
            this, SLOT(slotDeleteAuthorInfo()));
}

void KoDocumentInfoDlg::slotApply()
{
    saveAboutData();
    saveAuthorData();
}

void KoDocumentInfoDlg::saveAboutData()
{
    d->m_info->setAboutInfo("keyword", d->m_aboutUi->leKeywords->text());
    d->m_info->setAboutInfo("title", d->m_aboutUi->leTitle->text());
    d->m_info->setAboutInfo("subject", d->m_aboutUi->leSubject->text());
    d->m_info->setAboutInfo("comments", d->m_aboutUi->meComments->toPlainText());
    d->m_applyToggleEncryption = d->m_toggleEncryption;
}

void KoDocumentInfoDlg::saveAuthorData()
{
    d->m_info->setAuthorInfo("creator", d->m_authorUi->leFullName->text());
    d->m_info->setAuthorInfo("initial", d->m_authorUi->leInitials->text());
    d->m_info->setAuthorInfo("author-title", d->m_authorUi->leTitle->text());
    d->m_info->setAuthorInfo("company", d->m_authorUi->leCompany->text());
    d->m_info->setAuthorInfo("email", d->m_authorUi->leEmail->text());
    d->m_info->setAuthorInfo("telephone-work", d->m_authorUi->lePhoneWork->text());
    d->m_info->setAuthorInfo("telephone", d->m_authorUi->lePhoneHome->text());
    d->m_info->setAuthorInfo("fax", d->m_authorUi->leFax->text());
    d->m_info->setAuthorInfo("country", d->m_authorUi->leCountry->text());
    d->m_info->setAuthorInfo("postal-code", d->m_authorUi->lePostal->text());
    d->m_info->setAuthorInfo("city", d->m_authorUi->leCity->text());
    d->m_info->setAuthorInfo("street", d->m_authorUi->leStreet->text());
    d->m_info->setAuthorInfo("position", d->m_authorUi->lePosition->text());

    KConfig* config = KoGlobal::kofficeConfig();
    KConfigGroup cgs(config, "Author");
    cgs.writeEntry("telephone", d->m_authorUi->lePhoneHome->text());
    cgs.writeEntry("telephone-work", d->m_authorUi->lePhoneWork->text());
    cgs.writeEntry("fax", d->m_authorUi->leFax->text());
    cgs.writeEntry("country", d->m_authorUi->leCountry->text());
    cgs.writeEntry("postal-code", d->m_authorUi->lePostal->text());
    cgs.writeEntry("city",  d->m_authorUi->leCity->text());
    cgs.writeEntry("street", d->m_authorUi->leStreet->text());
    cgs.sync();
}

void KoDocumentInfoDlg::slotResetMetaData()
{
    d->m_info->resetMetaData();

    if (!d->m_info->aboutInfo("creation-date").isEmpty()) {
        QDateTime t = QDateTime::fromString(d->m_info->aboutInfo("creation-date"),
                                            Qt::ISODate);
        QString s = KGlobal::locale()->formatDateTime(t);
        d->m_aboutUi->lblCreated->setText(s + ", " +
                                          d->m_info->aboutInfo("initial-creator"));
    }

    if (!d->m_info->aboutInfo("date").isEmpty()) {
        QDateTime t = QDateTime::fromString(d->m_info->aboutInfo("date"), Qt::ISODate);
        QString s = KGlobal::locale()->formatDateTime(t);
        d->m_aboutUi->lblModified->setText(s + ", " + d->m_info->authorInfo("creator"));
    }

    d->m_aboutUi->lblRevision->setText(d->m_info->aboutInfo("editing-cycles"));
}

void KoDocumentInfoDlg::slotToggleEncryption()
{
    KoDocument* doc = dynamic_cast< KoDocument* >(d->m_info->parent());
    if (!doc)
        return;

    d->m_toggleEncryption = !d->m_toggleEncryption;

    if (doc->specialOutputFlag() == KoDocument::SaveEncrypted) {
        if (d->m_toggleEncryption) {
            QPixmap p = KIconLoader::global()->loadIcon("object-unlocked", KIconLoader::Small);
            d->m_aboutUi->lblEncrypted->setText(i18n("This document will be decrypted"));
            d->m_aboutUi->lblEncryptedPic->setPixmap(p);
            d->m_aboutUi->pbEncrypt->setText(i18n("Do not decrypt"));
        } else {
            QPixmap p = KIconLoader::global()->loadIcon("object-locked", KIconLoader::Small);
            d->m_aboutUi->lblEncrypted->setText(i18n("This document is encrypted"));
            d->m_aboutUi->lblEncryptedPic->setPixmap(p);
            d->m_aboutUi->pbEncrypt->setText(i18n("D&ecrypt"));
        }
    } else {
        if (d->m_toggleEncryption) {
            QPixmap p = KIconLoader::global()->loadIcon("object-locked", KIconLoader::Small);
            d->m_aboutUi->lblEncrypted->setText(i18n("This document will be encrypted."));
            d->m_aboutUi->lblEncryptedPic->setPixmap(p);
            d->m_aboutUi->pbEncrypt->setText(i18n("Do not encrypt"));
        } else {
            QPixmap p = KIconLoader::global()->loadIcon("object-unlocked", KIconLoader::Small);
            d->m_aboutUi->lblEncrypted->setText(i18n("This document is not encrypted"));
            d->m_aboutUi->lblEncryptedPic->setPixmap(p);
            d->m_aboutUi->pbEncrypt->setText(i18n("&Encrypt"));
        }
    }
}

void KoDocumentInfoDlg::slotDeleteAuthorInfo()
{
    d->m_authorUi->leFullName->clear();
    d->m_authorUi->leInitials->clear();
    d->m_authorUi->leTitle->clear();
    d->m_authorUi->leCompany->clear();
    d->m_authorUi->leEmail->clear();
    d->m_authorUi->lePhoneHome->clear();
    d->m_authorUi->lePhoneWork->clear();
    d->m_authorUi->leFax->clear();
    d->m_authorUi->leCountry->clear();
    d->m_authorUi->lePostal->clear();
    d->m_authorUi->leCity->clear();
    d->m_authorUi->leStreet->clear();
}

void KoDocumentInfoDlg::slotLoadFromKABC()
{
#ifdef KDEPIMLIBS_FOUND
    KABC::StdAddressBook *ab = static_cast<KABC::StdAddressBook*>
                               (KABC::StdAddressBook::self());
    if (!ab)
        return;

    KABC::Addressee addr = ab->whoAmI();
    if (addr.isEmpty()) {
        KMessageBox::sorry(0, i18n("No personal contact data set, please use the option \
                                    \"Set as Personal Contact Data\" from the \"Edit\"     menu in KAddressbook to set one."));
        return;
    }

    d->m_authorUi->leFullName->setText(addr.formattedName());
    d->m_authorUi->leInitials->setText(addr.givenName()[ 0 ] + ". " +
                                       addr.familyName()[ 0 ] + '.');
    d->m_authorUi->leTitle->setText(addr.title());
    d->m_authorUi->leCompany->setText(addr.organization());
    d->m_authorUi->leEmail->setText(addr.preferredEmail());

    KABC::PhoneNumber phone = addr.phoneNumber(KABC::PhoneNumber::Home);
    d->m_authorUi->lePhoneHome->setText(phone.number());
    phone = addr.phoneNumber(KABC::PhoneNumber::Work);
    d->m_authorUi->lePhoneWork->setText(phone.number());

    phone = addr.phoneNumber(KABC::PhoneNumber::Fax);
    d->m_authorUi->leFax->setText(phone.number());

    KABC::Address a = addr.address(KABC::Address::Home);
    d->m_authorUi->leCountry->setText(a.country());
    d->m_authorUi->lePostal->setText(a.postalCode());
    d->m_authorUi->leCity->setText(a.locality());
    d->m_authorUi->leStreet->setText(a.street());
#endif
}

void KoDocumentInfoDlg::slotSaveEncryption()
{
    if (!d->m_applyToggleEncryption)
        return;

    KoDocument* doc = dynamic_cast< KoDocument* >(d->m_info->parent());
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
    d->m_toggleEncryption = false;
    d->m_applyToggleEncryption = false;
    // Detects when the user cancelled saving
    d->m_documentSaved = !doc->url().isEmpty();
}

#include <KoDocumentInfoDlg.moc>
