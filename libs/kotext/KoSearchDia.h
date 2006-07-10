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

#ifndef kosearchdia_h
#define kosearchdia_h

#include <kfind.h>
#include <kfinddialog.h>
#include <kreplace.h>
#include <kreplacedialog.h>
#include "KoTextIterator.h"
#include "KoTextFormat.h"

#include <QColor>
#include <QString>
#include <QStringList>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3ValueList>
#include <koffice_export.h>

class QPushButton;
class QRadioButton;
class Q3GridLayout;
class QCheckBox;
class QComboBox;
class QSpinBox;
class KColorButton;
class KMacroCommand;
class KoTextView;
class KoTextObject;
class KCommand;
class KoTextDocument;
class KoFindReplace;
class QFontComboBox;
class KoTextCursor;
//
// This class represents the KWord-specific search extension items, and also the
// corresponding replace items.
//
class KOTEXT_EXPORT KoSearchContext
{
public:

    // Options.

    typedef enum
    {
        Family = 1 * KFind::MinimumUserOption,
        Color = 2 * KFind::MinimumUserOption,
        Size = 4 * KFind::MinimumUserOption,
        Bold = 8 * KFind::MinimumUserOption,
        Italic = 16 * KFind::MinimumUserOption,
        Underline = 32 * KFind::MinimumUserOption,
        VertAlign = 64 * KFind::MinimumUserOption,
        StrikeOut = 128 * KFind::MinimumUserOption,
        BgColor = 256 * KFind::MinimumUserOption,
        Shadow = 512 * KFind::MinimumUserOption,
        WordByWord = 1024 * KFind::MinimumUserOption,
        Attribute = 2048 * KFind::MinimumUserOption,
        Language = 4096 * KFind::MinimumUserOption
    } Options;

    KoSearchContext();
    ~KoSearchContext();

    QString m_family;
    QColor m_color;
    QColor m_backGroundColor;
    int m_size;
    KoTextFormat::VerticalAlignment m_vertAlign;
    KoTextFormat::UnderlineType m_underline;
    KoTextFormat::StrikeOutType m_strikeOut;
    KoTextFormat::AttributeStyle m_attribute;

    QStringList m_strings; // history
    long m_optionsMask;
    long m_options;
    QString m_language;
};

//
// This class represents the GUI elements that correspond to KWSearchContext.
//
class KoSearchContextUI : public QObject
{
    Q_OBJECT
public:
    KoSearchContextUI( KoSearchContext *ctx, QWidget *parent );
    void setCtxOptions( long options );
    void setCtxHistory( const QStringList & history );
    KoSearchContext *context() const { return m_ctx;}
    bool optionSelected() const { return m_bOptionsShown;}
private slots:
    void slotShowOptions();

private:
    KoSearchContext *m_ctx;
    Q3GridLayout *m_grid;
    bool m_bOptionsShown;
    QPushButton *m_btnShowOptions;
    QWidget *m_parent;
};

//
// This class is the KWord search dialog.
//
class KOTEXT_EXPORT KoSearchDia:
    public KFindDialog
{
    Q_OBJECT

public:
    KoSearchDia( QWidget *parent, const char *name, KoSearchContext *find, bool hasSelection, bool hasCursor );
    KoSearchContext * searchContext() {
        return m_findUI->context();
    }
    bool optionSelected() const { return m_findUI->optionSelected();}

protected slots:
    void slotOk();

private:
    KoSearchContextUI *m_findUI;
};

//
// This class is the kotext replace dialog.
//
class KOTEXT_EXPORT KoReplaceDia:
    public KReplaceDialog
{
    Q_OBJECT

public:

    KoReplaceDia( QWidget *parent, const char *name, KoSearchContext *find, KoSearchContext *replace, bool hasSelection, bool hasCursor );
    KoSearchContext * searchContext() {
        return m_findUI->context();
    }
    KoSearchContext * replaceContext() {
        return m_replaceUI->context();
    }
    bool optionFindSelected() const { return m_findUI->optionSelected();}
    bool optionReplaceSelected() const { return m_replaceUI->optionSelected();}
protected slots:
    void slotOk();

private:

    KoSearchContextUI *m_findUI;
    KoSearchContextUI *m_replaceUI;
};

/**
 * Reimplement KFind to provide our own validateMatch - for the formatting options
 */
class KoTextFind : public KFind
{
    Q_OBJECT
public:
    KoTextFind(const QString &pattern, long options, KoFindReplace *_findReplace, QWidget *parent = 0);
    ~KoTextFind();
    virtual bool validateMatch( const QString &text, int index, int matchedlength );
private:
    KoFindReplace * m_findReplace;
};

/**
 * Reimplement KoReplace to provide our own validateMatch - for the formatting options
 */
class KoTextReplace : public KReplace
{
    Q_OBJECT
public:
    KoTextReplace(const QString &pattern, const QString &replacement, long options, KoFindReplace *_findReplace, QWidget *parent = 0);
    ~KoTextReplace();
    virtual bool validateMatch( const QString &text, int index, int matchedlength );
private:
    KoFindReplace * m_findReplace;
};

/**
 * This class implements the 'find' functionality ( the "search next, prompt" loop )
 * and the 'replace' functionality. Same class, to allow centralizing the findNext() code.
 */
class KOTEXT_EXPORT KoFindReplace : public QObject
{
    Q_OBJECT
public:
    KoFindReplace( QWidget * parent, KoSearchDia * dialog, const Q3ValueList<KoTextObject *> & lstObject, KoTextView* textView );
    KoFindReplace( QWidget * parent, KoReplaceDia * dialog, const Q3ValueList<KoTextObject *> & lstObject, KoTextView* textView );
    ~KoFindReplace();

    KoTextParag *currentParag() {
        return m_textIterator.currentParag();
    }

    bool isReplace() const { return m_replace != 0L; }

    bool shouldRestart();

    //int numMatches() const;
    //int numReplacements() const;

    /** Look for the next match. Returns false if we're finished. */
    bool findNext();

    /** Look for the previous match. Returns false if we're finished. */
    bool findPrevious();

    /** Bring to front (e.g. when menuitem called twice) */
    void setActiveWindow();

    /** Emit undo/redo command for the last replacements made. */
    void emitUndoRedo();

    virtual void emitNewCommand(KCommand *) = 0;

    /**
     * Highlight a match.
     */
    virtual void highlightPortion(KoTextParag * parag, int index, int length, KoTextDocument *textdoc, KDialog* dialog) = 0;

    /** For KoTextFind and KoTextReplace */
    bool validateMatch( const QString &text, int index, int matchedlength );

protected:
    void replaceWithAttribut( KoTextCursor * cursor, int index );
    KMacroCommand* macroCommand();
    long options() const;
    void setOptions(long opt);
    void removeHighlight();
    bool needData() const { return m_find ? m_find->needData() : m_replace->needData(); }
    void setData( const QString& data, int startPos = -1 ) {
        if ( m_find ) m_find->setData( data, startPos );
        else m_replace->setData( data, startPos );
    }

protected slots:
    void slotFindNext();
    void optionsChanged();
    void dialogClosed();
    void highlight( const QString &text, int matchingIndex, int matchingLength );
    void replace( const QString &text, int replacementIndex, int replacedLength, int searchLength );
    void slotCurrentParagraphModified( int, int, int );

private:
    void connectFind( KFind* find );

    // Only one of those two will be set
    KoTextFind * m_find;
    KoTextReplace * m_replace;

    KoSearchContext m_searchContext;
    KoSearchContext m_replaceContext;
    bool m_searchContextEnabled;
    bool m_doCounting;
    bool m_bInit;
    bool m_currentParagraphModified;

    KMacroCommand *m_macroCmd;
    int m_offset;
    int m_matchingIndex; // can be removed once we use kde-3.2 (for kfind::index())

    KoTextIterator m_textIterator;

    // For removing the last highlight
    KoTextObject* m_lastTextObjectHighlighted;
};

/**
 * The separate dialog that pops up to ask for which formatting options
 * should be used to match text, or when replacing text.
 */
class KoFormatDia: public KDialog
{
    Q_OBJECT
public:
    KoFormatDia( QWidget* parent, const QString & _caption, KoSearchContext *_ctx, const char* name=0L);
    //apply to parameter to context !
    void ctxOptions( );

protected slots:
    void slotReset();
    void slotClear();
private:
    QCheckBox *m_checkFamily;
    QCheckBox *m_checkSize;
    QCheckBox *m_checkColor;
    QCheckBox *m_checkBgColor;
    QCheckBox *m_checkBold;
    QCheckBox *m_checkItalic;
    QCheckBox *m_checkShadow;
    QCheckBox *m_checkWordByWord;
    QCheckBox *m_checkUnderline;
    QCheckBox *m_checkVertAlign;
    QCheckBox *m_checkStrikeOut;
    QCheckBox *m_checkFontAttribute;
    QCheckBox *m_checkLanguage;

    QFontComboBox *m_familyItem;
    QSpinBox *m_sizeItem;
    KColorButton *m_colorItem;
    KColorButton *m_bgColorItem;
    QRadioButton *m_boldYes;
    QRadioButton *m_boldNo;
    QRadioButton *m_italicYes;
    QRadioButton *m_italicNo;
    QRadioButton *m_shadowYes;
    QRadioButton *m_shadowNo;
    QRadioButton *m_wordByWordYes;
    QRadioButton *m_wordByWordNo;

    QComboBox *m_vertAlignItem;
    QComboBox *m_underlineItem;
    QComboBox *m_strikeOutItem;
    QComboBox *m_fontAttributeItem;
    QComboBox *m_languageItem;
    KoSearchContext *m_ctx;
};

#endif
