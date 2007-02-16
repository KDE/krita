/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C)  2006 Thomas Zander <zander@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#include "KoFontDia.h"

#include <klocale.h>
#include <kvbox.h>
#include <kfontdialog.h>

#include <QWidget>

KoFontDia::KoFontDia( const QTextCharFormat &format, KSpell2::Loader::Ptr loader, QWidget* parent)
    : KDialog(parent),
    m_format(format)
{
    setCaption(i18n("Select Font") );
    setModal( true );
    setButtons( Ok|Cancel|Default|Apply );
    setDefaultButton( Ok );

    KVBox *mainWidget = new KVBox( this );
    KHBox *mainHBox = new KHBox( mainWidget );

    QTabWidget *fontTabWidget = new QTabWidget( mainHBox );

    // Font tab
    fontTab = new KoFontTab( KFontChooser::SmoothScalableFonts, this );
    fontTabWidget->addTab( fontTab, i18n( "Font" ) );

/*  connect( fontTab, SIGNAL( familyChanged() ), this, SLOT( slotFontFamilyChanged() ) );
    connect( fontTab, SIGNAL( boldChanged() ), this, SLOT( slotFontBoldChanged() ) );
    connect( fontTab, SIGNAL( italicChanged() ), this, SLOT( slotFontItalicChanged() ) );
    connect( fontTab, SIGNAL( sizeChanged() ), this, SLOT( slotFontSizeChanged() ) );
*/

    //Highlighting tab
    m_highlightingTab = new KoHighlightingTab( this );
    fontTabWidget->addTab( m_highlightingTab, i18n( "Highlighting" ) );

    //Decoration tab
    m_decorationTab = new KoDecorationTab( this );
    fontTabWidget->addTab( m_decorationTab, i18n( "Decoration" ) );

    //Layout tab
    m_layoutTab = new KoLayoutTab( true, this );
    fontTabWidget->addTab( m_layoutTab, i18n( "Layout" ) );

    //Language tab
    languageTab = new KoLanguageTab( loader, this );
    fontTabWidget->addTab( languageTab, i18n( "Language" ) );
    connect( languageTab, SIGNAL( languageChanged() ), this, SLOT( slotLanguageChanged() ) );

    //Related properties List View
    //relatedPropertiesListView = new K3ListView( mainHBox );

    //Preview
    //fontDiaPreview = new KoFontDiaPreview( mainWidget );

    setMainWidget( mainWidget );

    connect( this, SIGNAL( applyClicked() ), this, SLOT( slotApply() ) );
    connect( this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );
    connect( this, SIGNAL( defaultClicked() ), this, SLOT( slotReset() ) );
    slotReset();
}

void KoFontDia::slotApply()
{
    m_format.setFont(fontTab->font());
    m_highlightingTab->save( m_format );
    m_decorationTab->save( m_format );
    m_layoutTab->save( m_format );
}

void KoFontDia::slotOk()
{
    slotApply();
    QDialog::accept();
}

void KoFontDia::slotReset()
{
    fontTab->setFont( m_format.font());
    m_highlightingTab->open( m_format );
    m_decorationTab->open( m_format );
    m_layoutTab->open( m_format );
}

#include "KoFontDia.moc"

