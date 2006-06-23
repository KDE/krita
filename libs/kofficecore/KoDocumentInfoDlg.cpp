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
#include <kmimetype.h>
#include <klocale.h>
#include <kglobal.h>
#include <kabc/addressee.h>
#include <kabc/stdaddressbook.h>
#include <KoGlobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPixmap>
#include <QDateTime>

class KoDocumentInfoDlg::KoDocumentInfoDlgPrivate
{
public:
    KoDocumentInfoDlgPrivate()
    {}
    ~KoDocumentInfoDlgPrivate()
    {}

    KoDocumentInfo* m_info;
    Ui::KoDocumentInfoAboutWidget* m_aboutUi;
    Ui::KoDocumentInfoAuthorWidget* m_authorUi;
};




KoDocumentInfoDlg::KoDocumentInfoDlg( QWidget* parent, KoDocumentInfo* docInfo )
    : KPageDialog( parent )
{
    d = new KoDocumentInfoDlgPrivate;
    d->m_info = docInfo;

    setCaption( i18n( "Document Information" ) );
    setInitialSize( QSize( 500, 500 ) );
    setFaceType( KPageDialog::Tabbed );
    setButtons( KDialog::Ok|KDialog::Cancel );
    setDefaultButton( KDialog::Ok );

    d->m_aboutUi = new Ui::KoDocumentInfoAboutWidget();
    KDialog *infodlg = new KDialog(this);
    d->m_aboutUi->setupUi( infodlg );
    addPage( infodlg, i18n( "General" ) );

    initAboutTab();

    d->m_authorUi = new Ui::KoDocumentInfoAuthorWidget();

    KDialog *authordlg = new KDialog(this);
    d->m_authorUi->setupUi( authordlg );

    initAuthorTab();

    connect( this, SIGNAL( applyClicked() ), this, SLOT( slotApply() ) );
}

KoDocumentInfoDlg::~KoDocumentInfoDlg()
{
    delete d->m_authorUi;
    delete d->m_aboutUi;
    delete d;
}

void KoDocumentInfoDlg::initAboutTab()
{
    KoDocument* doc = dynamic_cast< KoDocument* >( d->m_info->parent() );
    if( !doc )
        return;

    d->m_aboutUi->leFileName->setText( doc->file() );
    QPixmap p = KMimeType::mimeType( doc->mimeType() )->pixmap( K3Icon::Desktop, 48 );
    d->m_aboutUi->lblPixmap->setPixmap( p );

    d->m_aboutUi->leTitle->setText( d->m_info->aboutInfo( "title" ) );
    d->m_aboutUi->leSubject->setText( d->m_info->aboutInfo( "subject" ) );

    if( d->m_info->aboutInfo( "keyword" ).isEmpty() )
        d->m_aboutUi->leKeywords->setText( i18n("Use ';' (Example: Office;KDE;KOffice)" ) );
    else
        d->m_aboutUi->leKeywords->setText( d->m_info->aboutInfo( "keyword" ) );

    d->m_aboutUi->meComments->setPlainText( d->m_info->aboutInfo( "comments" ) );

    d->m_aboutUi->lblType->setText( KMimeType::mimeType( doc->mimeType() )->comment() );

    if ( !d->m_info->aboutInfo( "creation-date" ).isEmpty() )
    {
        QDateTime t = QDateTime::fromString( d->m_info->aboutInfo( "creation-date" ),
                Qt::ISODate );
        QString s = KGlobal::locale()->formatDateTime( t );
        d->m_aboutUi->lblCreated->setText( s + ", " +
                d->m_info->aboutInfo( "initial-creator" ) );
    }

    if ( !d->m_info->aboutInfo( "date" ).isEmpty() )
    {
        QDateTime t = QDateTime::fromString( d->m_info->aboutInfo( "date" ), Qt::ISODate );
        QString s = KGlobal::locale()->formatDateTime( t );
        d->m_aboutUi->lblModified->setText( s + ", " + d->m_info->authorInfo( "creator" ) );
    }

    d->m_aboutUi->lblRevision->setText( d->m_info->aboutInfo( "editing-cycles" ) );

    connect( d->m_aboutUi->pbReset, SIGNAL( clicked() ),
            this, SLOT( slotResetMetaData() ) );
}

void KoDocumentInfoDlg::initAuthorTab()
{
    QPixmap p = KGlobal::iconLoader()->loadIcon( "personal", K3Icon::Desktop, 48 );
    d->m_authorUi->lblAuthor->setPixmap( p );
    p = KGlobal::iconLoader()->loadIcon( "kaddressbook", K3Icon::Small );
    d->m_authorUi->pbLoadKABC->setIcon( QIcon( p ) );
    p= KGlobal::iconLoader()->loadIcon( "eraser", K3Icon::Small );
    d->m_authorUi->pbDelete->setIcon( QIcon( p ) );

    d->m_authorUi->leFullName->setText( d->m_info->authorInfo( "creator" ) );
    d->m_authorUi->leInitials->setText( d->m_info->authorInfo( "initial" ) );
    d->m_authorUi->leTitle->setText( d->m_info->authorInfo( "author-title" ) );
    d->m_authorUi->leCompany->setText( d->m_info->authorInfo( "company" ) );
    d->m_authorUi->leEmail->setText( d->m_info->authorInfo( "email" ) );
    d->m_authorUi->lePhoneWork->setText( d->m_info->authorInfo( "telephone-work" ) );
    d->m_authorUi->lePhoneHome->setText( d->m_info->authorInfo( "telephone" ) );
    d->m_authorUi->leFax->setText( d->m_info->authorInfo( "fax" ) );
    d->m_authorUi->leCountry->setText( d->m_info->authorInfo( "country" ) );
    d->m_authorUi->lePostal->setText( d->m_info->authorInfo( "postal-code" ) );
    d->m_authorUi->leCity->setText( d->m_info->authorInfo( "city" ) );
    d->m_authorUi->leStreet->setText( d->m_info->authorInfo( "street" ) );
    d->m_authorUi->lePosition->setText( d->m_info->authorInfo( "position" ) );

    connect( d->m_authorUi->pbLoadKABC, SIGNAL( clicked() ),
            this, SLOT( slotLoadFromKABC() ) );
    connect( d->m_authorUi->pbDelete, SIGNAL( clicked() ),
            this, SLOT( slotDeleteAuthorInfo() ) );
}

void KoDocumentInfoDlg::slotApply()
{
    saveAboutData();
    saveAuthorData();
}

void KoDocumentInfoDlg::saveAboutData()
{
    if( d->m_aboutUi->leKeywords->text() !=
            i18n( "Use ';' (Example: Office;KDE;KOffice)" ) )
        d->m_info->setAboutInfo( "keyword", d->m_aboutUi->leKeywords->text() );

    d->m_info->setAboutInfo( "title", d->m_aboutUi->leTitle->text() );
    d->m_info->setAboutInfo( "subject", d->m_aboutUi->leSubject->text() );
    d->m_info->setAboutInfo( "comments", d->m_aboutUi->meComments->toPlainText() );
}

void KoDocumentInfoDlg::saveAuthorData()
{
    d->m_info->setAuthorInfo( "creator", d->m_authorUi->leFullName->text() );
    d->m_info->setAuthorInfo( "initial", d->m_authorUi->leInitials->text() );
    d->m_info->setAuthorInfo( "title", d->m_authorUi->leTitle->text() );
    d->m_info->setAuthorInfo( "company", d->m_authorUi->leCompany->text() );
    d->m_info->setAuthorInfo( "email", d->m_authorUi->leEmail->text() );
    d->m_info->setAuthorInfo( "telephone-work", d->m_authorUi->lePhoneWork->text() );
    d->m_info->setAuthorInfo( "telephone", d->m_authorUi->lePhoneHome->text() );
    d->m_info->setAuthorInfo( "fax", d->m_authorUi->leFax->text() );
    d->m_info->setAuthorInfo( "country", d->m_authorUi->leCountry->text() );
    d->m_info->setAuthorInfo( "postal-code", d->m_authorUi->lePostal->text() );
    d->m_info->setAuthorInfo( "city", d->m_authorUi->leCity->text() );
    d->m_info->setAuthorInfo( "street", d->m_authorUi->leStreet->text() );
    d->m_info->setAuthorInfo( "position", d->m_authorUi->lePosition->text() );

    KConfig* config = KoGlobal::kofficeConfig();
    KConfigGroup cgs( config, "Author" );
    config->writeEntry("telephone", d->m_authorUi->lePhoneHome->text());
    config->writeEntry("telephone-work", d->m_authorUi->lePhoneWork->text());
    config->writeEntry("fax", d->m_authorUi->leFax->text());
    config->writeEntry("country",d->m_authorUi->leCountry->text());
    config->writeEntry("postal-code",d->m_authorUi->lePostal->text());
    config->writeEntry("city",  d->m_authorUi->leCity->text());
    config->writeEntry("street", d->m_authorUi->leStreet->text());
    config->sync();
}

void KoDocumentInfoDlg::slotResetMetaData()
{
    d->m_info->resetMetaData();

    if ( !d->m_info->aboutInfo( "creation-date" ).isEmpty() )
    {
        QDateTime t = QDateTime::fromString( d->m_info->aboutInfo( "creation-date" ),
                Qt::ISODate );
        QString s = KGlobal::locale()->formatDateTime( t );
        d->m_aboutUi->lblCreated->setText( s + ", " +
                d->m_info->aboutInfo( "initial-creator" ) );
    }

    if ( !d->m_info->aboutInfo( "date" ).isEmpty() )
    {
        QDateTime t = QDateTime::fromString( d->m_info->aboutInfo( "date" ), Qt::ISODate );
        QString s = KGlobal::locale()->formatDateTime( t );
        d->m_aboutUi->lblModified->setText( s + ", " + d->m_info->authorInfo( "creator" ) );
    }

    d->m_aboutUi->lblRevision->setText( d->m_info->aboutInfo( "editing-cycles" ) );
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
    KABC::StdAddressBook *ab = static_cast<KABC::StdAddressBook*>
        ( KABC::StdAddressBook::self() );
    if ( !ab )
        return;

    KABC::Addressee addr = ab->whoAmI();
    if ( addr.isEmpty() )
    {
        KMessageBox::sorry( 0L, i18n( "No personal contact data set, please use the option \
                    \"Set as Personal Contact Data\" from the \"Edit\"     menu in KAddressbook to set one." ) );
        return;
    }

    d->m_authorUi->leFullName->setText( addr.formattedName() );
    d->m_authorUi->leInitials->setText( addr.givenName()[ 0 ] + ". " +
            addr.familyName()[ 0 ] + "." );
    d->m_authorUi->leTitle->setText( addr.title() );
    d->m_authorUi->leCompany->setText( addr.organization() );
    d->m_authorUi->leEmail->setText( addr.preferredEmail() );

    KABC::PhoneNumber phone = addr.phoneNumber( KABC::PhoneNumber::Home );
    d->m_authorUi->lePhoneHome->setText( phone.number() );
    phone = addr.phoneNumber( KABC::PhoneNumber::Work );
    d->m_authorUi->lePhoneWork->setText( phone.number() );

    phone = addr.phoneNumber( KABC::PhoneNumber::Fax );
    d->m_authorUi->leFax->setText( phone.number() );

    KABC::Address a = addr.address( KABC::Address::Home );
    d->m_authorUi->leCountry->setText( a.country() );
    d->m_authorUi->lePostal->setText( a.postalCode() );
    d->m_authorUi->leCity->setText( a.locality() );
    d->m_authorUi->leStreet->setText( a.street() );
}

#include "KoDocumentInfoDlg.moc"
