/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>

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




#include "KoRichText.h"

#include <kcolordialog.h>
#include <klocale.h>
#include <kdebug.h>

#include <q3groupbox.h>
#include <QPushButton>
#include <QTabWidget>
#include <QLabel>
#include <QComboBox>
#include <QRadioButton>
#include <QCheckBox>
#include <knuminput.h>
#include <KoGlobal.h>
#include <QGroupBox>


#include <QButtonGroup>
#include <kcolorbutton.h>
#include <kiconloader.h>
#include <kvbox.h>

KoFontDia::KoFontDia( const KoTextFormat& initialFormat,
    KSpell2::Loader::Ptr loader, QWidget* parent, const char* name )
    : KDialog( parent ),
    m_initialFormat(initialFormat),
    m_changedFlags(KoTextFormat::NoFlags)
{
    setCaption(i18n("Select Font") );
    setModal( true );
    setObjectName( name );
    setButtons( Ok|Cancel|User1|Apply );
    setDefaultButton( Ok );

    setButtonText( KDialog::User1, i18n("&Reset") );

    KVBox *mainWidget = new KVBox( this );
    KHBox *mainHBox = new KHBox( mainWidget );

    QTabWidget *fontTabWidget = new QTabWidget( mainHBox );

    // Font tab
    fontTab = new KoFontTab( KFontChooser::SmoothScalableFonts, this );
    fontTabWidget->addTab( fontTab, i18n( "Font" ) );

    connect( fontTab, SIGNAL( familyChanged() ), this, SLOT( slotFontFamilyChanged() ) );
    connect( fontTab, SIGNAL( boldChanged() ), this, SLOT( slotFontBoldChanged() ) );
    connect( fontTab, SIGNAL( italicChanged() ), this, SLOT( slotFontItalicChanged() ) );
    connect( fontTab, SIGNAL( sizeChanged() ), this, SLOT( slotFontSizeChanged() ) );

    //Highlighting tab
    highlightingTab = new KoHighlightingTab( this );
    fontTabWidget->addTab( highlightingTab, i18n( "Highlighting" ) );

    connect( highlightingTab, SIGNAL( underlineChanged( int ) ), this, SLOT( slotUnderlineChanged( int ) ) );
    connect( highlightingTab, SIGNAL( underlineStyleChanged( int ) ), this, SLOT( slotUnderlineStyleChanged( int ) ) );
    connect( highlightingTab, SIGNAL( underlineColorChanged( const QColor & ) ), this, SLOT( slotUnderlineColorChanged( const QColor & ) ) );
    connect( highlightingTab, SIGNAL( strikethroughChanged( int ) ), this, SLOT( slotStrikethroughChanged( int ) ) );
    connect( highlightingTab, SIGNAL( strikethroughStyleChanged( int ) ), this, SLOT( slotStrikethroughStyleChanged( int ) ) );
    connect( highlightingTab, SIGNAL( wordByWordChanged( bool ) ), this, SLOT( slotWordByWordChanged( bool ) ) );
    connect( highlightingTab, SIGNAL( capitalisationChanged( int ) ), this, SLOT( slotCapitalisationChanged( int ) ) );

    //Decoratio tab
    decorationTab = new KoDecorationTab( this );
    fontTabWidget->addTab( decorationTab, i18n( "Decoration" ) );

    connect( decorationTab, SIGNAL( fontColorChanged( const QColor& ) ), this, SLOT( slotFontColorChanged( const QColor& ) ) );
    connect( decorationTab, SIGNAL( backgroundColorChanged( const QColor& ) ), this, SLOT( slotBackgroundColorChanged( const QColor& ) ) );
    connect( decorationTab, SIGNAL( shadowColorChanged( const QColor& ) ), this, SLOT( slotShadowColorChanged( const QColor& ) ) );
    connect( decorationTab, SIGNAL( shadowDistanceChanged( double ) ), this, SLOT( slotShadowDistanceChanged( double ) ) );
    connect( decorationTab, SIGNAL( shadowDirectionChanged( int ) ), this, SLOT( slotShadowDirectionChanged( int ) ) );

    //Layout tab
    layoutTab = new KoLayoutTab( true, this );
    fontTabWidget->addTab( layoutTab, i18n( "Layout" ) );
    connect( layoutTab, SIGNAL( subSuperScriptChanged() ), this, SLOT( slotSubSuperChanged() ) );;
    connect( layoutTab, SIGNAL( offsetChanged( int ) ), this, SLOT( slotOffsetChanged( int ) ) );
    connect( layoutTab, SIGNAL( relativeSizeChanged( double ) ), this, SLOT( slotRelativeSizeChanged( double ) ) );
    connect( layoutTab, SIGNAL( hyphenationChanged( bool ) ), this, SLOT( slotHyphenationChanged( bool ) ) );

    //Language tab
    languageTab = new KoLanguageTab( loader, this );
    fontTabWidget->addTab( languageTab, i18n( "Language" ) );
    connect( languageTab, SIGNAL( languageChanged() ), this, SLOT( slotLanguageChanged() ) );

    //Related properties List View
    //relatedPropertiesListView = new K3ListView( mainHBox );

    //Preview
    fontDiaPreview = new KoFontDiaPreview( mainWidget );

    setMainWidget( mainWidget );

    init();
}

void KoFontDia::init()
{
    connect( this, SIGNAL( user1Clicked() ), this, SLOT(slotReset()) );

    slotReset();
}

KoTextFormat KoFontDia::newFormat() const
{
    return KoTextFormat( fontTab->getSelection(),
                         layoutTab->getSubSuperScript(),
                         decorationTab->getTextColor(),
                         decorationTab->getBackgroundColor(),
                         highlightingTab->getUnderlineColor(),
                         highlightingTab->getUnderline(),
                         highlightingTab->getUnderlineStyle(),
                         highlightingTab->getStrikethrough(),
                         highlightingTab->getStrikethroughStyle(),
                         highlightingTab->getCapitalisation(),
                         languageTab->getLanguage(),
                         layoutTab->getRelativeTextSize(),
                         layoutTab->getOffsetFromBaseline(),
                         highlightingTab->getWordByWord(),
                         layoutTab->getAutoHyphenation(),
                         decorationTab->getShadowDistanceX(),
                         decorationTab->getShadowDistanceY(),
                         decorationTab->getShadowColor()
			);
}

void KoFontDia::slotApply()
{
    emit applyFont();
}

void KoFontDia::slotOk()
{
    slotApply();
    slotButtonClicked( Ok );
}

void KoFontDia::slotReset()
{
    fontTab->setSelection( m_initialFormat.font() );
    highlightingTab->setUnderline( m_initialFormat.underlineType() );
    highlightingTab->setUnderlineStyle( m_initialFormat.underlineStyle() );
    highlightingTab->setUnderlineColor( m_initialFormat.textUnderlineColor() );
    highlightingTab->setStrikethrough( m_initialFormat.strikeOutType() );
    highlightingTab->setStrikethroughStyle( m_initialFormat.strikeOutStyle() );
    highlightingTab->setWordByWord( m_initialFormat.wordByWord() );
    highlightingTab->setCapitalisation( m_initialFormat.attributeFont() );
    decorationTab->setTextColor( m_initialFormat.color() );
    decorationTab->setBackgroundColor( m_initialFormat.textBackgroundColor() );
    decorationTab->setShadow( m_initialFormat.shadowDistanceX(), m_initialFormat.shadowDistanceY(), m_initialFormat.shadowColor() );
    layoutTab->setSubSuperScript( m_initialFormat.vAlign(), m_initialFormat.offsetFromBaseLine(), m_initialFormat.relativeTextSize() );
    layoutTab->setAutoHyphenation( m_initialFormat.hyphenation() );
    languageTab->setLanguage( m_initialFormat.language() );
}

void KoFontDia::slotFontFamilyChanged()
{
    m_changedFlags |= KoTextFormat::Family;
    fontDiaPreview->setFont( fontTab->getSelection() );
}

void KoFontDia::slotFontBoldChanged()
{
    m_changedFlags |= KoTextFormat::Bold;
    fontDiaPreview->setFont( fontTab->getSelection() );
}

void KoFontDia::slotFontItalicChanged()
{
    m_changedFlags |= KoTextFormat::Italic;
    fontDiaPreview->setFont( fontTab->getSelection() );
}

void KoFontDia::slotFontSizeChanged()
{
    m_changedFlags |= KoTextFormat::Size;
    fontDiaPreview->setFont( fontTab->getSelection() );
}

void KoFontDia::slotFontColorChanged( const QColor& color )
{
    m_changedFlags |= KoTextFormat::Color;
    fontDiaPreview->setFontColor( color );
}

void KoFontDia::slotBackgroundColorChanged( const QColor& color )
{
    m_changedFlags |= KoTextFormat::TextBackgroundColor;
    fontDiaPreview->setBackgroundColor( color );
}

void KoFontDia::slotCapitalisationChanged( int item )
{
    m_changedFlags |= KoTextFormat::Attribute;
    fontDiaPreview->setCapitalisation( item );
}

void KoFontDia::slotUnderlineChanged( int item )
{
    m_changedFlags |= KoTextFormat::ExtendUnderLine;
    if ( !item ) fontDiaPreview->setUnderlining( item, 0, Qt::black, false );
    else fontDiaPreview->setUnderlining( item, highlightingTab->getUnderlineStyle(), highlightingTab->getUnderlineColor(), highlightingTab->getWordByWord() );
}

void KoFontDia::slotUnderlineStyleChanged( int item )
{
    m_changedFlags |= KoTextFormat::ExtendUnderLine;
    if ( !highlightingTab->getUnderline() ) fontDiaPreview->setUnderlining( 0, 0, Qt::black, false );
    else fontDiaPreview->setUnderlining( highlightingTab->getUnderline(), item, highlightingTab->getUnderlineColor(), highlightingTab->getWordByWord() );
}

void KoFontDia::slotUnderlineColorChanged( const QColor &color )
{
    m_changedFlags |= KoTextFormat::ExtendUnderLine;
    if ( !highlightingTab->getUnderline() ) fontDiaPreview->setUnderlining( 0, 0, Qt::black, false );
    else fontDiaPreview->setUnderlining( highlightingTab->getUnderline(), highlightingTab->getUnderlineStyle(), color, highlightingTab->getWordByWord() );
}

void KoFontDia::slotWordByWordChanged( bool state )
{
    m_changedFlags |= KoTextFormat::WordByWord;
    fontDiaPreview->setWordByWord( state );
}

void KoFontDia::slotStrikethroughChanged( int item )
{
    m_changedFlags |= KoTextFormat::StrikeOut;
    if ( !item ) fontDiaPreview->setStrikethrough( item, 0, false );
    else fontDiaPreview->setStrikethrough( item, highlightingTab->getStrikethroughStyle(), highlightingTab->getWordByWord() );
}

void KoFontDia::slotStrikethroughStyleChanged( int item )
{
    m_changedFlags |= KoTextFormat::StrikeOut;
    if ( !highlightingTab->getStrikethrough() ) fontDiaPreview->setStrikethrough( 0, 0, false );
    else fontDiaPreview->setStrikethrough( highlightingTab->getStrikethrough(), item, highlightingTab->getWordByWord() );
}

void KoFontDia::slotShadowDistanceChanged( double )
{
    m_changedFlags |= KoTextFormat::ShadowText;
    fontDiaPreview->setShadow( decorationTab->getShadowDistanceX(), decorationTab->getShadowDistanceY(), decorationTab->getShadowColor() );
}

void KoFontDia::slotShadowDirectionChanged( int )
{
    m_changedFlags |= KoTextFormat::ShadowText;
    fontDiaPreview->setShadow( decorationTab->getShadowDistanceX(), decorationTab->getShadowDistanceY(), decorationTab->getShadowColor() );
}

void KoFontDia::slotShadowColorChanged( const QColor & )
{
    m_changedFlags |= KoTextFormat::ShadowText;
    fontDiaPreview->setShadow( decorationTab->getShadowDistanceX(), decorationTab->getShadowDistanceY(), decorationTab->getShadowColor() );
}

void KoFontDia::slotSubSuperChanged()
{
    m_changedFlags |= KoTextFormat::VAlign;
    fontDiaPreview->setSubSuperscript( layoutTab->getSubSuperScript(), layoutTab->getOffsetFromBaseline(), layoutTab->getRelativeTextSize() );
}

void KoFontDia::slotOffsetChanged( int offset )
{
    m_changedFlags |= KoTextFormat::OffsetFromBaseLine;
    fontDiaPreview->setSubSuperscript( layoutTab->getSubSuperScript(), offset, layoutTab->getRelativeTextSize() );
}

void KoFontDia::slotRelativeSizeChanged( double relativeSize )
{
    m_changedFlags |= KoTextFormat::VAlign;
    fontDiaPreview->setSubSuperscript( layoutTab->getSubSuperScript(), layoutTab->getOffsetFromBaseline(), relativeSize );
}

void KoFontDia::slotHyphenationChanged( bool )
{
    m_changedFlags |= KoTextFormat::Hyphenation;
}

void KoFontDia::slotLanguageChanged()
{
    m_changedFlags |= KoTextFormat::Language;
}

#include "KoFontDia.moc"

