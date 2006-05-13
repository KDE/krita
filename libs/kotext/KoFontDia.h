/* This file is part of the KDE project
   Copyright (C)  2001,2002,2003 Montel Laurent <lmontel@mandrakesoft.com>

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

#include <kfontdialog.h>
#include <QTabWidget>
#include <KoTextFormat.h>
#include <QCheckBox>
#include <koffice_export.h>

#include "KoFontTab.h"
#include "KoHighlightingTab.h"
#include "KoDecorationTab.h"
#include "KoLayoutTab.h"
#include "KoLanguageTab.h"

#include "KoFontDiaPreview.h"

#include <kspell2/broker.h>

class QComboBox;


class KOTEXT_EXPORT KoFontDia : public KDialogBase
{
    Q_OBJECT
public:

    /// If your application supports spell-checking, pass here the KSpell2 Broker
    /// so that the font dialog can show which languages are supported for spellchecking.
    KoFontDia( const KoTextFormat& initialFormat,
               KSpell2::Broker::Ptr broker = KSpell2::Broker::Ptr(),
               QWidget* parent = 0, const char* name = 0 );

    int changedFlags() const { return m_changedFlags; }

    KoTextFormat newFormat() const;

protected slots:
    void slotReset();
    virtual void slotApply();
    virtual void slotOk();
    void slotFontFamilyChanged();
    void slotFontBoldChanged();
    void slotFontItalicChanged();
    void slotFontSizeChanged();
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
    void slotLanguageChanged( int );

signals:
    void applyFont();

private:
    void init();

    KoTextFormat m_initialFormat;
    KoFontTab *fontTab;
    KoHighlightingTab *highlightingTab;
    KoDecorationTab *decorationTab;
    KoLayoutTab *layoutTab;
    KoLanguageTab *languageTab;
    KoFontDiaPreview *fontDiaPreview;

    int m_changedFlags;

};

#endif
