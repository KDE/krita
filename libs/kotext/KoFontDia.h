/* This file is part of the KDE project
   Copyright (C)  2001,2002,2003 Montel Laurent <lmontel@mandrakesoft.com>
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

#ifndef __kofontdia_h__
#define __kofontdia_h__

// local lib
#include "KoFontTab.h"
#include "KoHighlightingTab.h"
#include "KoDecorationTab.h"
#include "KoLayoutTab.h"
#include "KoLanguageTab.h"

// koffice
//#include <koffice_export.h>

// kde + Qt
#include <QTabWidget>
#include <kdialog.h>
#include <sonnet/loader.h>

#include <QTextCharFormat>

//   #include <kfontdialog.h>
//   #include <QCheckBox>
//
//
//   #include "KoFontDiaPreview.h"
//

//class QComboBox;


class KoFontDia : public KDialog
{
    Q_OBJECT
public:

    /// If your application supports spell-checking, pass here the KSpell2 Loader
    /// so that the font dialog can show which languages are supported for spellchecking.
    KoFontDia( const QTextCharFormat &format, KSpell2::Loader::Ptr loader = KSpell2::Loader::Ptr(), QWidget* parent = 0);

    QTextCharFormat format() { return m_format; }

protected slots:
    void slotReset();
    void slotApply();
    void slotOk();
    //void slotFontFamilyChanged();
    //void slotFontBoldChanged();
    //void slotFontItalicChanged();
    //void slotFontSizeChanged();
    void slotFontColorChanged( const QColor& color );
    void slotBackgroundColorChanged( const QColor& color );
    void slotCapitalisationChanged( int item );
    void slotUnderlineChanged( int item );
    void slotUnderlineStyleChanged( int item );
    void slotUnderlineColorChanged( const QColor &color );
    void slotStrikethroughChanged( int item );
    void slotStrikethroughStyleChanged( int item );
    void slotWordByWordChanged( bool state );
    void slotShadowDistanceChanged( double distance );
    void slotShadowDirectionChanged( int direction );
    void slotShadowColorChanged( const QColor &color );
    void slotSubSuperChanged();
    void slotOffsetChanged( int offset );
    void slotRelativeSizeChanged( double relativeSize );
    void slotHyphenationChanged( bool state );
    void slotLanguageChanged();

    void fontChanged(const QFont &font);

private:
    QTextCharFormat m_format;
    KoFontTab *fontTab;
    KoHighlightingTab *highlightingTab;
    KoDecorationTab *decorationTab;
    KoLayoutTab *layoutTab;
    KoLanguageTab *languageTab;
    //KoFontDiaPreview *fontDiaPreview;
};

#endif
