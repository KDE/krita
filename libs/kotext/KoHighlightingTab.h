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

#ifndef __kohighlightingtab_h__
#define __kohighlightingtab_h__

#include <kohighlightingtabbase.h>
#include <KoTextFormat.h>

#include <QColor>

class KoHighlightingTab : public KoHighlightingTabBase
{
    Q_OBJECT

public:
    KoHighlightingTab( QWidget* parent=0, const char* name=0, Qt::WFlags fl=0 );
    ~KoHighlightingTab();

    KoTextFormat::UnderlineType getUnderline() const;
    KoTextFormat::UnderlineStyle getUnderlineStyle() const;
    QColor getUnderlineColor() const;
    KoTextFormat::StrikeOutType getStrikethrough() const;
    KoTextFormat::StrikeOutStyle getStrikethroughStyle() const;
    bool getWordByWord() const;
    KoTextFormat::AttributeStyle getCapitalisation() const;

    void setUnderline( KoTextFormat::UnderlineType item );
    void setUnderlineStyle( KoTextFormat::UnderlineStyle item );
    void setUnderlineColor( const QColor &color );
    void setStrikethrough( int item );
    void setStrikethroughStyle( int item );
    void setWordByWord( bool state );
    void setCapitalisation( int item );

signals:
    void underlineChanged( int item );
    void underlineStyleChanged( int item );
    void underlineColorChanged( const QColor & );
    void strikethroughChanged( int item );
    void strikethroughStyleChanged( int item );
    void wordByWordChanged( bool state );
    void capitalisationChanged( int item );

protected slots:
    void slotUnderlineChanged( int item );
    void slotStrikethroughChanged( int item );
};

#endif

