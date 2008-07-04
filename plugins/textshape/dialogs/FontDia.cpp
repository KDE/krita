/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C)  2006-2007 Thomas Zander <zander@kde.org>
   Copyright (C)  2008 Girish Ramakrishnan <girish@forwardbias.in>

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

#include "FontDia.h"
#include "FontTab.h"
#include "CharacterHighlighting.h"
#include "FontDecorations.h"
#include "FontLayoutTab.h"
#include "LanguageTab.h"

#include <klocale.h>
#include <kvbox.h>
#include <kfontdialog.h>

FontDia::FontDia(const QTextCursor &cursor, QWidget* parent)
    : KDialog(parent),
      m_cursor(cursor),
      m_initialFormat(cursor.charFormat()),
      m_style(m_initialFormat)
{
    setCaption(i18n("Select Font") );
    setModal( true );
    setButtons(Ok|Cancel|Reset|Apply);
    setDefaultButton( Ok );

    KVBox *mainWidget = new KVBox( this );
    KHBox *mainHBox = new KHBox( mainWidget );

    QTabWidget *fontTabWidget = new QTabWidget( mainHBox );

    // Font tab
    m_fontTab = new FontTab(this);
    fontTabWidget->addTab(m_fontTab, i18n("Font"));

/*  connect( fontTab, SIGNAL( familyChanged() ), this, SLOT( slotFontFamilyChanged() ) );
    connect( fontTab, SIGNAL( boldChanged() ), this, SLOT( slotFontBoldChanged() ) );
    connect( fontTab, SIGNAL( italicChanged() ), this, SLOT( slotFontItalicChanged() ) );
    connect( fontTab, SIGNAL( sizeChanged() ), this, SLOT( slotFontSizeChanged() ) );
*/

    //Highlighting tab
    m_highlightingTab = new CharacterHighlighting( this );
    fontTabWidget->addTab( m_highlightingTab, i18n( "Highlighting" ) );

    //Decoration tab
    m_decorationTab = new FontDecorations( this );
    fontTabWidget->addTab( m_decorationTab, i18n( "Decoration" ) );

    //Layout tab
    m_layoutTab = new FontLayoutTab( true, this );
    fontTabWidget->addTab( m_layoutTab, i18n( "Layout" ) );

    //Language tab
    m_languageTab = new LanguageTab(this);
    fontTabWidget->addTab(m_languageTab, i18n("Language"));

    //Related properties List View
    //relatedPropertiesListView = new K3ListView( mainHBox );

    //Preview
    //fontDiaPreview = new KoFontDiaPreview( mainWidget );

    setMainWidget( mainWidget );

    connect( this, SIGNAL( applyClicked() ), this, SLOT( slotApply() ) );
    connect( this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );
    connect( this, SIGNAL( resetClicked() ), this, SLOT( slotReset() ) );
    initTabs();
}

void FontDia::initTabs()
{
    m_fontTab->setFont(m_style.font());
    m_highlightingTab->open(&m_style);
    m_decorationTab->open(&m_style);
    m_layoutTab->open(&m_style);
    m_languageTab->setLanguage(m_style.language());
}

void FontDia::slotApply()
{
    QFont font = m_fontTab->font();
    m_style.setFontFamily(font.family());
    m_style.setFontPointSize(font.pointSize());
    m_style.setFontWeight(font.weight());
    m_style.setFontItalic(font.italic());
    m_highlightingTab->save();
    m_decorationTab->save();
    m_layoutTab->save();
    m_style.setLanguage(m_languageTab->language());
    m_style.applyStyle(&m_cursor);
}

void FontDia::slotOk()
{
    slotApply();
    KDialog::accept();
}

void FontDia::slotReset()
{
    m_style.copyProperties(m_initialFormat);
    initTabs();
    slotApply(); // ### Should reset() apply?
}

#include "FontDia.moc"

