/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2001, S.R.Haque <srhaque@iee.org>
   Copyright (C) 2001, David Faure <faure@kde.org>

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

#include "KoSearchDia.h"
#include "KoTextParag.h"

#include <KoGlobal.h>
#include <KoTextObject.h>
#include <KoTextView.h>

#include <kcolorbutton.h>
#include <kcommand.h>
#include <kdebug.h>
#include <qfontcombobox.h>
#include <klocale.h>
#include <kseparator.h>

#include <q3buttongroup.h>
#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>
#include <QRegExp>
#include <QSpinBox>
#include <QLayout>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3ValueList>

KoSearchContext::KoSearchContext()
{
    m_family = "times";
    m_color = Qt::black;
    m_backGroundColor = Qt::black;

    m_size = 12;
    m_vertAlign = KoTextFormat::AlignNormal;
    m_optionsMask = 0;
    m_options = KFind::FromCursor | KReplaceDialog::PromptOnReplace;
    m_underline = KoTextFormat::U_NONE;
    m_strikeOut = KoTextFormat::S_NONE;
    m_attribute = KoTextFormat::ATT_NONE;
    m_language = QString::null;
}

KoSearchContext::~KoSearchContext()
{
}


KoSearchContextUI::KoSearchContextUI( KoSearchContext *ctx, QWidget *parent )
    : QObject(parent), m_ctx(ctx), m_parent(parent)
{
    m_bOptionsShown = false;
    m_btnShowOptions = new QPushButton( i18n("Show Formatting Options"), parent );
    connect( m_btnShowOptions, SIGNAL( clicked() ), SLOT( slotShowOptions() ) );

    m_grid = new Q3GridLayout( m_parent, 1, 1, 0, 6 );
    m_grid->addWidget( m_btnShowOptions, 0, 0 );
    m_btnShowOptions->setEnabled( true );
}

void KoSearchContextUI::slotShowOptions()
{
    KoFormatDia * dlg = new KoFormatDia( m_parent, i18n("Formatting Options"), m_ctx );
    if ( dlg->exec())
    {
        dlg->ctxOptions( );
        m_bOptionsShown = true;
    }

    delete dlg;
}

void KoSearchContextUI::setCtxOptions( long options )
{
    if ( m_bOptionsShown )
    {
        options |= m_ctx->m_options;
    }
    m_ctx->m_options = options;
}

void KoSearchContextUI::setCtxHistory( const QStringList & history )
{
    m_ctx->m_strings = history;
}

KoSearchDia::KoSearchDia( QWidget * parent,const char *name, KoSearchContext *find, bool hasSelection, bool hasCursor )
    : KFindDialog( parent, name, find->m_options, find->m_strings )
{
    // The dialog extension.
    m_findUI = new KoSearchContextUI( find, findExtension() );
    setHasSelection(hasSelection);
    setHasCursor(hasCursor);
}

void KoSearchDia::slotOk()
{
    KFindDialog::slotOk();

    // Save the current state back into the context required.
    if ( optionSelected() )
        m_findUI->setCtxOptions( options() );
    m_findUI->setCtxHistory( findHistory() );
}

KoReplaceDia::KoReplaceDia( QWidget *parent, const char *name, KoSearchContext *find, KoSearchContext *replace, bool hasSelection, bool hasCursor )
    : KReplaceDialog( parent, name, find->m_options, find->m_strings, replace->m_strings )
{
    // The dialog extension.
    m_findUI = new KoSearchContextUI( find, findExtension() );
    m_replaceUI = new KoSearchContextUI( replace, replaceExtension() );
    // Look whether we have a selection, and/or a cursor
    setHasSelection(hasSelection);
    setHasCursor(hasCursor);
}

void KoReplaceDia::slotOk()
{
    KReplaceDialog::slotOk();

    // Save the current state back into the context required.
    m_findUI->setCtxHistory( findHistory() );
    if ( optionFindSelected() )
        m_findUI->setCtxOptions( KReplaceDialog::options() );

    m_replaceUI->setCtxHistory( replacementHistory() );
    if ( optionReplaceSelected() )
        m_replaceUI->setCtxOptions( KReplaceDialog::options() );
}



KoFindReplace::KoFindReplace( QWidget * parent, KoSearchDia * dialog, const Q3ValueList<KoTextObject *> & lstObject, KoTextView* textView )
    : m_find( new KoTextFind( dialog->pattern(), dialog->options(), this, parent ) ),
      m_replace( 0L ),
      m_searchContext( *dialog->searchContext() ),
      m_replaceContext(),
      m_searchContextEnabled( dialog->optionSelected() ),
      m_doCounting( true ),
      m_macroCmd( 0L ),
      m_offset( 0 ),
      m_textIterator( lstObject, textView, dialog->options() ),
      m_lastTextObjectHighlighted( 0 )
{
    connectFind( m_find );
}

KoFindReplace::KoFindReplace( QWidget * parent, KoReplaceDia * dialog, const Q3ValueList<KoTextObject *> & lstObject, KoTextView* textView )
    : m_find( 0L ),
      m_replace( new KoTextReplace( dialog->pattern(), dialog->replacement(), dialog->options(), this, parent ) ),
      m_searchContext( *dialog->searchContext() ),
      m_replaceContext( *dialog->replaceContext() ),
      m_searchContextEnabled( dialog->optionFindSelected() ),
      m_doCounting( true ),
      m_macroCmd( 0L ),
      m_offset( 0 ),
      m_textIterator( lstObject, textView, dialog->options() ),
      m_lastTextObjectHighlighted( 0 )
{
    connectFind( m_replace );
    connect( m_replace, SIGNAL( replace( const QString &, int , int, int ) ),
             this, SLOT( replace( const QString &, int , int,int ) ) );
}

void KoFindReplace::connectFind( KFind* find )
{
    connect( find, SIGNAL( optionsChanged() ),
             this, SLOT( optionsChanged() ) );
    connect( find, SIGNAL( dialogClosed() ),
             this, SLOT( dialogClosed() ) );
    // Connect highlight signal to code which handles highlighting
    // of found text.
    connect( find, SIGNAL( highlight( const QString &, int, int ) ),
             this, SLOT( highlight( const QString &, int, int ) ) );
    // Connect findNext signal - called when pressing the button in the dialog
    connect( find, SIGNAL( findNext() ),
             this, SLOT( slotFindNext() ) );
    m_bInit = true;
    m_currentParagraphModified = false;
    m_matchingIndex = -1;

    // Also connect to the textiterator
    connect( &m_textIterator, SIGNAL( currentParagraphModified( int, int, int ) ),
             this, SLOT( slotCurrentParagraphModified( int, int, int ) ) );
}

KoFindReplace::~KoFindReplace()
{
    removeHighlight();
    //kDebug(32500) << "KoFindReplace::~KoFindReplace" << endl;
    delete m_find;
    delete m_replace;
}

void KoFindReplace::optionsChanged()
{
    m_textIterator.setOptions( options() );
}

void KoFindReplace::dialogClosed()
{
    removeHighlight();
    emitUndoRedo();
    // we have to stop the match counting when closing the dialog,
    // because Shift-F3 (find previous) would keep increasing that count, wrongly.
    m_doCounting = false;
}

void KoFindReplace::removeHighlight()
{
    if ( m_lastTextObjectHighlighted )
        m_lastTextObjectHighlighted->removeHighlight(true);
    m_lastTextObjectHighlighted = 0L;
}

void KoFindReplace::emitUndoRedo()
{
    // Emit the command containing the replace operations
    // #### We allow editing text during a replace operation, so we need
    // to call this more often... so that 'undo' undoes the last
    // replace operations. TODO!
    if(m_macroCmd)
        emitNewCommand(m_macroCmd);
    m_macroCmd = 0L;
}

/**
*
* This whole code should be rewritten at some point:
* if we want to allow matches across paragraph borders,
* then we need to send the whole text of a textobject at once,
* instead of paragraph-after-paragraph.
*
*/

bool KoFindReplace::findNext()
{
    KFind::Result res = KFind::NoMatch;
    while ( res == KFind::NoMatch && !m_textIterator.atEnd() ) {
        //kDebug(32500) << "findNext loop. m_bInit=" << m_bInit << " needData=" << needData() << " m_currentParagraphModified=" << m_currentParagraphModified << endl;
        if ( needData() || m_currentParagraphModified ) {
            if ( !m_bInit && !m_currentParagraphModified ) {
                ++m_textIterator;
                if ( m_textIterator.atEnd() )
                    break;
            }
            m_bInit = false;
            QPair<int, QString> c = m_textIterator.currentTextAndIndex();
            m_offset = c.first;
            if ( !m_currentParagraphModified )
                setData( c.second );
            else
                setData( c.second, m_matchingIndex );
            m_currentParagraphModified = false;
        }

        if ( m_find )
            // Let KFind inspect the text fragment, and display a dialog if a match is found
            res = m_find->find();
        else
            res = m_replace->replace();
    }

    //kDebug(32500) << k_funcinfo << "we're done. res=" << res << endl;
    if ( res == KFind::NoMatch ) // i.e. at end
    {
        emitUndoRedo();
        removeHighlight();
        if ( shouldRestart() ) {
            m_textIterator.setOptions( m_textIterator.options() & ~KFind::FromCursor );
            m_textIterator.restart();
            m_bInit = true;
            if ( m_find )
                m_find->resetCounts();
            else
                m_replace->resetCounts();
            return findNext();
        }
        else { // done, close the 'find next' dialog
            if ( m_find )
                m_find->closeFindNextDialog();
            else
                m_replace->closeReplaceNextDialog();
        }
        return false;
    }
    return true;
}

void KoFindReplace::slotFindNext() // called by the button in the small "find next?" dialog
{
    bool ret = findNext();
    Q_UNUSED(ret);
}

bool KoFindReplace::findPrevious()
{
    int opt = options();
    bool forw = ! ( options() & KFind::FindBackwards );
    if ( forw )
        setOptions( opt | KFind::FindBackwards );
    else
        setOptions( opt & ~KFind::FindBackwards );

    bool ret = findNext();

    setOptions( opt ); // restore initial options

    return ret;
}

long KoFindReplace::options() const
{
    return m_find ? m_find->options() : m_replace->options();
}

void KoFindReplace::setOptions(long opt)
{
    if ( m_find )
        m_find->setOptions(opt);
    else
        m_replace->setOptions(opt);
    m_textIterator.setOptions( opt );
}

void KoFindReplace::slotCurrentParagraphModified( int, int pos, int )
{
    if ( pos >= m_offset )
        m_currentParagraphModified = true;
    // (this bool forces the next findNext() to call setData again)
}

// slot connected to the 'highlight' signal
void KoFindReplace::highlight( const QString &, int matchingIndex, int matchingLength )
{
    m_matchingIndex = matchingIndex;
    if ( m_lastTextObjectHighlighted )
        m_lastTextObjectHighlighted->removeHighlight(true);
    m_lastTextObjectHighlighted = m_textIterator.currentTextObject();
    //kDebug(32500) << "KoFindReplace::highlight " << matchingIndex << "," << matchingLength << endl;
    KDialog* dialog = m_find ? m_find->findNextDialog() : m_replace->replaceNextDialog();
    highlightPortion(m_textIterator.currentParag(), m_offset + matchingIndex, matchingLength, m_lastTextObjectHighlighted->textDocument(), dialog );
}

// slot connected to the 'replace' signal
void KoFindReplace::replace( const QString &text, int matchingIndex,
                             int replacementLength, int matchedLength )
{
    //kDebug(32500) << "KoFindReplace::replace m_offset=" << m_offset << " matchingIndex=" << matchingIndex << " matchedLength=" << matchedLength << " options=" << options() << endl;
    m_matchingIndex = matchingIndex;
    int index = m_offset + matchingIndex;

    // highlight might not have happened (if 'prompt on replace' is off)
    if ( (options() & KReplaceDialog::PromptOnReplace) == 0 )
        highlight( text, matchingIndex, matchedLength );

    KoTextObject* currentTextObj = m_textIterator.currentTextObject();
    KoTextDocument * textdoc = currentTextObj->textDocument();
    KoTextCursor cursor( textdoc );
    cursor.setParag( m_textIterator.currentParag() );
    cursor.setIndex( index );

    //reactive spellchecking
    currentTextObj->setNeedSpellCheck(true);
    if ( m_replaceContext.m_optionsMask )
    {
        replaceWithAttribut( &cursor, index );
    }
    // Don't repaint if we're doing batch changes
    bool repaint = options() & KReplaceDialog::PromptOnReplace;

    // Grab replacement string
    QString rep = text.mid( matchingIndex, replacementLength );

    // Don't let the replacement set the paragraph to "modified by user"
    disconnect( &m_textIterator, SIGNAL( currentParagraphModified( int, int, int ) ),
                this, SLOT( slotCurrentParagraphModified( int, int, int ) ) );

    KCommand *cmd = currentTextObj->replaceSelectionCommand(
        &cursor, rep, QString::null,
        KoTextDocument::HighlightSelection,
        repaint ? KoTextObject::DefaultInsertFlags : KoTextObject::DoNotRepaint );

    connect( &m_textIterator, SIGNAL( currentParagraphModified( int, int, int ) ),
             this, SLOT( slotCurrentParagraphModified( int, int, int ) ) );

    if( cmd )
        macroCommand()->addCommand(cmd);
}

void KoFindReplace::replaceWithAttribut( KoTextCursor * cursor, int index )
{
    KoTextFormat * lastFormat = m_textIterator.currentParag()->at( index )->format();
    KoTextFormat * newFormat = new KoTextFormat(*lastFormat);
    int flags = 0;
    if (m_replaceContext.m_optionsMask & KoSearchContext::Bold)
    {
        flags |= KoTextFormat::Bold;
        newFormat->setBold( (bool)(m_replaceContext.m_options & KoSearchContext::Bold) );
    }
    if (m_replaceContext.m_optionsMask & KoSearchContext::Size)
    {
        flags |= KoTextFormat::Size;
        newFormat->setPointSize( m_replaceContext.m_size );

    }
    if ( m_replaceContext.m_optionsMask & KoSearchContext::Family)
    {
        flags |= KoTextFormat::Family;
        newFormat->setFamily( m_replaceContext.m_family );
    }
    if ( m_replaceContext.m_optionsMask & KoSearchContext::Color)
    {
        flags |= KoTextFormat::Color;
        newFormat->setColor( m_replaceContext.m_color );
    }
    if ( m_replaceContext.m_optionsMask & KoSearchContext::Italic)
    {
        flags |= KoTextFormat::Italic;
        newFormat->setItalic( (bool)(m_replaceContext.m_options & KoSearchContext::Italic) );
    }
    if ( m_replaceContext.m_optionsMask & KoSearchContext::Underline)
    {
        flags |= KoTextFormat::ExtendUnderLine;
        newFormat->setUnderlineType( m_replaceContext.m_underline );

    }
    if ( m_replaceContext.m_optionsMask & KoSearchContext::VertAlign)
    {
        flags |= KoTextFormat::VAlign;
        newFormat->setVAlign( m_replaceContext.m_vertAlign);
    }
    if ( m_replaceContext.m_optionsMask & KoSearchContext::StrikeOut)
    {
        flags |= KoTextFormat::StrikeOut;
        newFormat->setStrikeOutType( m_replaceContext.m_strikeOut);
    }
    if ( m_replaceContext.m_optionsMask & KoSearchContext::BgColor)
    {
        newFormat->setTextBackgroundColor(m_replaceContext.m_backGroundColor);
        flags |= KoTextFormat::TextBackgroundColor;
    }
    if (m_replaceContext.m_optionsMask & KoSearchContext::Shadow)
    {
        flags |= KoTextFormat::ShadowText;
        // If shadow has been selected, we set a shadow (any shadow) in the new format
        if ( m_replaceContext.m_options & KoSearchContext::Shadow )
            newFormat->setShadow( 1, 1, Qt::gray );
        else
            newFormat->setShadow( 0, 0, QColor() );
    }
    if (m_replaceContext.m_optionsMask & KoSearchContext::WordByWord)
    {
        flags |= KoTextFormat::WordByWord;
        newFormat->setWordByWord( (bool)(m_replaceContext.m_options & KoSearchContext::WordByWord) );
    }
    if (m_replaceContext.m_optionsMask & KoSearchContext::Language)
    {
        flags |= KoTextFormat::Language;
        newFormat->setLanguage( m_replaceContext.m_language );
    }


    KCommand *cmd = m_textIterator.currentTextObject()->setFormatCommand( cursor, &lastFormat ,newFormat,flags , false, KoTextDocument::HighlightSelection );

    if( cmd )
        macroCommand()->addCommand(cmd);
}

KMacroCommand* KoFindReplace::macroCommand()
{
    // Create on demand, to avoid making an empty command
    if(!m_macroCmd)
        m_macroCmd = new KMacroCommand(i18n("Replace Text"));
    return m_macroCmd;
}

void KoFindReplace::setActiveWindow()
{
    KDialog* dialog = m_find ? m_find->findNextDialog() : m_replace->replaceNextDialog();
    if ( dialog )
        dialog->activateWindow();
}

/*int KoFindReplace::numMatches() const
{
    return m_find->numMatches();
}

int KoFindReplace::numReplacements() const
{
    return m_replace->numReplacements();
}*/

////

KoTextFind::KoTextFind( const QString &pattern, long options, KoFindReplace *_findReplace, QWidget *parent )
    : KFind( pattern, options, parent),
      m_findReplace( _findReplace)
{
}

KoTextFind::~KoTextFind()
{
}

bool KoTextFind::validateMatch( const QString &text, int index, int matchedlength )
{
    return m_findReplace->validateMatch( text, index, matchedlength );
}

KoTextReplace::KoTextReplace(const QString &pattern, const QString &replacement, long options, KoFindReplace *_findReplace, QWidget *parent )
    : KReplace( pattern, replacement, options, parent),
      m_findReplace( _findReplace)
{
}

KoTextReplace::~KoTextReplace()
{
}

bool KoTextReplace::validateMatch( const QString &text, int index, int matchedlength )
{
    return m_findReplace->validateMatch( text, index, matchedlength );
}

KoFormatDia::KoFormatDia( QWidget* parent, const QString & _caption, KoSearchContext *_ctx ,  const char* name)
    : KDialog( parent ),
      m_ctx(_ctx)
{
    setCaption( _caption );
    setModal( true );
    setObjectName( name );
    setButtons( Ok|Cancel|User1 |User2 );
    setModal( true );

    QWidget *page = new QWidget( this );
    setMainWidget(page);
    setButtonText( KDialog::User1, i18n("Reset") );
    setButtonText( KDialog::User2, i18n("Clear") );

    connect( this, SIGNAL( user1Clicked() ), this, SLOT(slotReset()));
    connect( this, SIGNAL( user2Clicked() ), this, SLOT(slotClear()));

    Q3GridLayout *m_grid = new Q3GridLayout( page, 15, 2, 0, 6 );
    m_checkFamily = new QCheckBox( i18n( "Family:" ),page  );
    m_checkSize = new QCheckBox( i18n( "Size:" ), page );
    m_checkColor = new QCheckBox( i18n( "Color:" ), page );
    m_checkBgColor = new QCheckBox( i18n( "Background color:" ), page );
    m_checkBold = new QCheckBox( i18n( "Bold:" ), page );
    m_checkItalic = new QCheckBox( i18n( "Italic:" ),page );
    m_checkShadow = new QCheckBox( i18n( "Shadow:" ), page );
    m_checkWordByWord = new QCheckBox( i18n( "Word by word:" ), page );

    m_checkUnderline = new QCheckBox( i18n( "Underline:" ), page);
    m_underlineItem = new QComboBox( page );
    // This has to be the type list, not the style list (we need the "no underline" case).
    // Of course we could even have both...
    m_underlineItem->addItems( KoTextFormat::underlineTypeList() );
    m_underlineItem->setCurrentIndex( (int)m_ctx->m_underline );

    m_checkStrikeOut= new QCheckBox( i18n( "Strikeout:" ), page);

    m_strikeOutItem = new QComboBox( page );
    m_strikeOutItem->addItems( KoTextFormat::strikeOutTypeList() );
    m_strikeOutItem->setCurrentIndex( (int)m_ctx->m_strikeOut );


    m_checkFontAttribute = new QCheckBox( i18n( "Capitalization:" ), page);
    m_fontAttributeItem = new QComboBox( page );
    m_fontAttributeItem->addItems( KoTextFormat::fontAttributeList() );
    m_fontAttributeItem->setCurrentIndex( (int)m_ctx->m_attribute );

    m_checkLanguage = new QCheckBox( i18n( "Language:" ), page);
    m_languageItem = new QComboBox( page );
    m_languageItem->addItems( KoGlobal::listOfLanguages() );
    m_languageItem->setItemText( 0, KoGlobal::languageFromTag( m_ctx->m_language ) );


    m_checkVertAlign = new QCheckBox( i18n( "Vertical alignment:" ), page );

    m_familyItem = new QFontComboBox(page);
    m_familyItem->setCurrentFont(m_ctx->m_family);

    m_sizeItem = new QSpinBox( page );
    m_sizeItem->setMaximum( 100 );
    m_sizeItem->setMinimum( 4 );
    m_sizeItem->setValue( m_ctx->m_size );

    m_colorItem = new KColorButton( page );
    m_colorItem->setColor( m_ctx->m_color );

    m_bgColorItem = new KColorButton( page );
    m_bgColorItem->setColor( m_ctx->m_backGroundColor);



    Q3ButtonGroup *grpBold = new Q3ButtonGroup( 1, Qt::Vertical, page );
    grpBold->setRadioButtonExclusive( TRUE );
    grpBold->layout();
    m_boldYes=new QRadioButton( i18n("Yes"), grpBold );
    m_boldNo=new QRadioButton( i18n("No"), grpBold );

    Q3ButtonGroup *grpItalic = new Q3ButtonGroup( 1, Qt::Vertical, page );
    grpItalic->setRadioButtonExclusive( TRUE );
    grpItalic->layout();
    m_italicYes=new QRadioButton( i18n("Yes"), grpItalic );
    m_italicNo=new QRadioButton( i18n("No"), grpItalic );

    Q3ButtonGroup *grpShadow = new Q3ButtonGroup( 1, Qt::Vertical, page );
    grpShadow->setRadioButtonExclusive( TRUE );
    grpShadow->layout();
    m_shadowYes=new QRadioButton( i18n("Yes"), grpShadow );
    m_shadowNo=new QRadioButton( i18n("No"), grpShadow );

    Q3ButtonGroup *grpWordByWord = new Q3ButtonGroup( 1, Qt::Vertical, page );
    grpWordByWord->setRadioButtonExclusive( TRUE );
    grpWordByWord->layout();
    m_wordByWordYes=new QRadioButton( i18n("Yes"), grpWordByWord );
    m_wordByWordNo=new QRadioButton( i18n("No"), grpWordByWord );


    m_vertAlignItem = new QComboBox( page );
    m_vertAlignItem->setEditable( false );
    m_vertAlignItem->addItem( i18n( "Normal" ) );
    m_vertAlignItem->addItem( i18n( "Subscript" ) );
    m_vertAlignItem->addItem( i18n( "Superscript" ) );
    m_vertAlignItem->setCurrentIndex( (int)m_ctx->m_vertAlign );

    m_grid->addWidget( m_checkFamily, 1, 0 );
    m_grid->addWidget( m_checkSize, 2, 0 );
    m_grid->addWidget( m_checkColor, 3, 0 );
    m_grid->addWidget( m_checkBgColor, 4, 0);
    m_grid->addWidget( m_checkBold, 5, 0 );
    m_grid->addWidget( m_checkItalic, 6, 0 );
    m_grid->addWidget( m_checkStrikeOut, 7, 0 );
    m_grid->addWidget( m_checkUnderline, 8, 0 );
    m_grid->addWidget( m_checkVertAlign, 9, 0 );
    m_grid->addWidget( m_checkShadow, 10, 0 );
    m_grid->addWidget( m_checkWordByWord, 11, 0 );
    m_grid->addWidget( m_checkFontAttribute, 12, 0 );

    m_grid->addWidget( m_familyItem, 1, 1 );
    m_grid->addWidget( m_sizeItem, 2, 1 );
    m_grid->addWidget( m_colorItem, 3, 1 );
    m_grid->addWidget( m_bgColorItem, 4, 1);
    m_grid->addWidget( grpBold, 5, 1 );
    m_grid->addWidget( grpItalic, 6, 1 );

    m_grid->addWidget( m_strikeOutItem, 7, 1 );
    m_grid->addWidget( m_underlineItem, 8, 1 );

    m_grid->addWidget( m_vertAlignItem, 9, 1 );
    m_grid->addWidget( grpShadow, 10, 1 );
    m_grid->addWidget( grpWordByWord, 11, 1 );

    m_grid->addWidget( m_fontAttributeItem, 12, 1);

    m_grid->addWidget( m_checkLanguage, 13, 0);
    m_grid->addWidget( m_languageItem, 13, 1);

    KSeparator *tmpSep = new KSeparator( page );
    m_grid->addMultiCellWidget( tmpSep, 14, 14, 0, 1 );

    // signals and slots connections
    QObject::connect( m_checkFamily, SIGNAL( toggled( bool ) ), m_familyItem, SLOT( setEnabled( bool ) ) );
    QObject::connect( m_checkSize, SIGNAL( toggled( bool ) ), m_sizeItem, SLOT( setEnabled( bool ) ) );
    QObject::connect( m_checkColor, SIGNAL( toggled( bool ) ), m_colorItem, SLOT( setEnabled( bool ) ) );
    QObject::connect( m_checkBgColor, SIGNAL( toggled( bool ) ), m_bgColorItem, SLOT( setEnabled( bool ) ) );

    QObject::connect( m_checkBold, SIGNAL( toggled( bool ) ), m_boldYes, SLOT( setEnabled( bool ) ) );
    QObject::connect( m_checkItalic, SIGNAL( toggled( bool ) ), m_italicYes, SLOT( setEnabled( bool ) ) );
    QObject::connect( m_checkStrikeOut, SIGNAL( toggled( bool ) ), m_strikeOutItem, SLOT( setEnabled( bool ) ) );
    QObject::connect( m_checkShadow, SIGNAL( toggled( bool ) ), m_shadowYes, SLOT( setEnabled( bool ) ) );
    QObject::connect( m_checkWordByWord, SIGNAL( toggled( bool ) ), m_wordByWordYes, SLOT( setEnabled( bool ) ) );
    QObject::connect( m_checkFontAttribute, SIGNAL( toggled( bool ) ), m_fontAttributeItem, SLOT( setEnabled( bool ) ) );
    QObject::connect( m_checkLanguage, SIGNAL( toggled( bool ) ), m_languageItem, SLOT( setEnabled( bool ) ) );


    QObject::connect( m_checkBold, SIGNAL( toggled( bool ) ), m_boldNo, SLOT( setEnabled( bool ) ) );
    QObject::connect( m_checkItalic, SIGNAL( toggled( bool ) ), m_italicNo, SLOT( setEnabled( bool ) ) );
    QObject::connect( m_checkShadow, SIGNAL( toggled( bool ) ), m_shadowNo, SLOT( setEnabled( bool ) ) );
    QObject::connect( m_checkWordByWord, SIGNAL( toggled( bool ) ), m_wordByWordNo, SLOT( setEnabled( bool ) ) );


    QObject::connect( m_checkVertAlign, SIGNAL( toggled( bool ) ), m_vertAlignItem, SLOT( setEnabled( bool ) ) );

    QObject::connect( m_checkUnderline, SIGNAL( toggled( bool ) ), m_underlineItem, SLOT( setEnabled( bool ) ) );

    slotReset();
}

void KoFormatDia::slotClear()
{
    m_ctx->m_optionsMask = 0;
    m_ctx->m_options = 0;
    slotReset();
}

void KoFormatDia::slotReset()
{
    m_checkFamily->setChecked( m_ctx->m_optionsMask & KoSearchContext::Family );
    m_familyItem->setEnabled(m_checkFamily->isChecked());

    m_checkSize->setChecked( m_ctx->m_optionsMask & KoSearchContext::Size );
    m_sizeItem->setEnabled(m_checkSize->isChecked());

    m_checkColor->setChecked( m_ctx->m_optionsMask & KoSearchContext::Color );
    m_colorItem->setEnabled(m_checkColor->isChecked());

    m_checkBgColor->setChecked( m_ctx->m_optionsMask & KoSearchContext::BgColor );
    m_bgColorItem->setEnabled(m_checkBgColor->isChecked());


    m_checkBold->setChecked( m_ctx->m_optionsMask & KoSearchContext::Bold );
    m_boldYes->setEnabled(m_checkBold->isChecked());
    m_boldNo->setEnabled(m_checkBold->isChecked());

    m_checkShadow->setChecked( m_ctx->m_optionsMask & KoSearchContext::Shadow );
    m_shadowYes->setEnabled(m_checkShadow->isChecked());
    m_shadowNo->setEnabled(m_checkShadow->isChecked());

    m_checkWordByWord->setChecked( m_ctx->m_optionsMask & KoSearchContext::WordByWord );
    m_wordByWordYes->setEnabled(m_checkWordByWord->isChecked());
    m_wordByWordNo->setEnabled(m_checkWordByWord->isChecked());


    m_checkStrikeOut->setChecked( m_ctx->m_optionsMask & KoSearchContext::StrikeOut );
    m_strikeOutItem->setEnabled( m_checkStrikeOut->isChecked());


    m_checkItalic->setChecked( m_ctx->m_optionsMask & KoSearchContext::Italic );
    m_italicNo->setEnabled(m_checkItalic->isChecked());
    m_italicYes->setEnabled(m_checkItalic->isChecked());

    m_checkUnderline->setChecked( m_ctx->m_optionsMask & KoSearchContext::Underline );
    m_underlineItem->setEnabled(m_checkUnderline->isChecked());

    m_checkVertAlign->setChecked( m_ctx->m_optionsMask & KoSearchContext::VertAlign );
    m_vertAlignItem->setEnabled(m_checkVertAlign->isChecked());

    m_checkFontAttribute->setChecked( m_ctx->m_optionsMask & KoSearchContext::Attribute );
    m_fontAttributeItem->setEnabled(m_checkFontAttribute->isChecked());


    m_checkLanguage->setChecked( m_ctx->m_optionsMask & KoSearchContext::Language );
    m_languageItem->setEnabled(m_checkLanguage->isChecked());


    if (m_ctx->m_options & KoSearchContext::Bold)
        m_boldYes->setChecked( true );
    else
        m_boldNo->setChecked( true );

    if (m_ctx->m_options & KoSearchContext::Italic)
        m_italicYes->setChecked( true );
    else
        m_italicNo->setChecked( true );

    if (m_ctx->m_options & KoSearchContext::Shadow)
        m_shadowYes->setChecked( true );
    else
        m_shadowNo->setChecked( true );

    if (m_ctx->m_options & KoSearchContext::WordByWord)
        m_wordByWordYes->setChecked( true );
    else
        m_wordByWordNo->setChecked( true );

}

void KoFormatDia::ctxOptions( )
{
    long optionsMask = 0;
    long options = 0;
    if ( m_checkFamily->isChecked() )
        optionsMask |= KoSearchContext::Family;
    if ( m_checkSize->isChecked() )
        optionsMask |= KoSearchContext::Size;
    if ( m_checkColor->isChecked() )
        optionsMask |= KoSearchContext::Color;
    if ( m_checkBgColor->isChecked() )
        optionsMask |= KoSearchContext::BgColor;
    if ( m_checkBold->isChecked() )
        optionsMask |= KoSearchContext::Bold;
    if ( m_checkItalic->isChecked() )
        optionsMask |= KoSearchContext::Italic;
    if ( m_checkUnderline->isChecked() )
        optionsMask |= KoSearchContext::Underline;
    if ( m_checkVertAlign->isChecked() )
        optionsMask |= KoSearchContext::VertAlign;
    if ( m_checkStrikeOut->isChecked() )
        optionsMask |= KoSearchContext::StrikeOut;
    if ( m_checkShadow->isChecked() )
        optionsMask |= KoSearchContext::Shadow;
    if ( m_checkWordByWord->isChecked() )
        optionsMask |= KoSearchContext::WordByWord;
    if ( m_checkLanguage->isChecked() )
        optionsMask |= KoSearchContext::Language;


    if ( m_boldYes->isChecked() )
        options |= KoSearchContext::Bold;
    if ( m_italicYes->isChecked() )
        options |= KoSearchContext::Italic;
    if ( m_shadowYes->isChecked() )
        options |= KoSearchContext::Shadow;
    if ( m_wordByWordYes->isChecked() )
        options |= KoSearchContext::WordByWord;


    m_ctx->m_optionsMask = optionsMask;
    m_ctx->m_family = m_familyItem->currentText();
    m_ctx->m_size = m_sizeItem->cleanText().toInt();
    m_ctx->m_color = m_colorItem->color();
    m_ctx->m_backGroundColor = m_bgColorItem->color();
    m_ctx->m_vertAlign = (KoTextFormat::VerticalAlignment)m_vertAlignItem->currentIndex();
    m_ctx->m_underline = (KoTextFormat::UnderlineType)m_underlineItem->currentIndex();
    m_ctx->m_strikeOut = (KoTextFormat::StrikeOutType)m_strikeOutItem->currentIndex();
    m_ctx->m_attribute = (KoTextFormat::AttributeStyle)m_fontAttributeItem->currentIndex();
    m_ctx->m_language = KoGlobal::listTagOfLanguages()[m_languageItem->currentIndex()];

    m_ctx->m_options = options;
}


bool KoFindReplace::validateMatch( const QString & /*text*/, int index, int matchedlength )
{
    if ( !m_searchContextEnabled || !m_searchContext.m_optionsMask )
        return true;
    KoTextString * s = currentParag()->string();
    for ( int i = index ; i < index+matchedlength ; ++i )
    {
        KoTextStringChar & ch = s->at(i);
        KoTextFormat *format = ch.format();
        if (m_searchContext.m_optionsMask & KoSearchContext::Bold)
        {
            if ( (!format->font().bold() && (m_searchContext.m_options & KoSearchContext::Bold)) || (format->font().bold() && ((m_searchContext.m_options & KoSearchContext::Bold)==0)))
                return false;
        }
        if (m_searchContext.m_optionsMask & KoSearchContext::Shadow)
        {
            bool hasShadow = format->shadowDistanceX() != 0 || format->shadowDistanceY() != 0;
            if ( (!hasShadow && (m_searchContext.m_options & KoSearchContext::Shadow))
                 || (hasShadow && ((m_searchContext.m_options & KoSearchContext::Shadow)==0)) )
                return false;
        }

        if (m_searchContext.m_optionsMask & KoSearchContext::WordByWord)
        {
            if ( (!format->wordByWord() && (m_searchContext.m_options & KoSearchContext::WordByWord)) || (format->wordByWord() && ((m_searchContext.m_options & KoSearchContext::WordByWord)==0)))
                return false;
        }


        if (m_searchContext.m_optionsMask & KoSearchContext::Size)
        {
            if ( format->font().pointSize() != m_searchContext.m_size )
                return false;
        }
        if ( m_searchContext.m_optionsMask & KoSearchContext::Family)
        {
            if (format->font().family() != m_searchContext.m_family)
                return false;
        }
        if ( m_searchContext.m_optionsMask & KoSearchContext::Color)
        {
            if (format->color() != m_searchContext.m_color)
                return false;
        }
        if ( m_searchContext.m_optionsMask & KoSearchContext::BgColor)
        {
            if (format->textBackgroundColor() != m_searchContext.m_backGroundColor)
                return false;
        }

        if ( m_searchContext.m_optionsMask & KoSearchContext::Italic)
        {
            if ( (!format->font().italic() && (m_searchContext.m_options & KoSearchContext::Italic)) || (format->font().italic() && ((m_searchContext.m_options & KoSearchContext::Italic)==0)))
                return false;

        }
        if ( m_searchContext.m_optionsMask & KoSearchContext::Underline)
        {
            if ( format->underlineType() != m_searchContext.m_underline )
                return false;
        }
        if ( m_searchContext.m_optionsMask & KoSearchContext::StrikeOut)
        {
            if ( format->strikeOutType() != m_searchContext.m_strikeOut )
                return false;
        }

        if ( m_searchContext.m_optionsMask & KoSearchContext::VertAlign)
        {
            if ( format->vAlign() != m_searchContext.m_vertAlign )
                return false;
        }
        if ( m_searchContext.m_optionsMask & KoSearchContext::Language)
        {
            if ( format->language() != m_searchContext.m_language )
                return false;
        }

        if ( m_searchContext.m_optionsMask & KoSearchContext::Attribute)
        {
            if ( format->attributeFont() != m_searchContext.m_attribute )
                return false;
        }

    }
    return true;
}

bool KoFindReplace::shouldRestart()
{
    if ( m_find )
        return m_find->shouldRestart( true /*since text is editable*/, m_doCounting );
    else
        return m_replace->shouldRestart( true /*since text is editable*/, m_doCounting );
}

#include "KoSearchDia.moc"
