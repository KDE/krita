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

#include "KoFontTab.h"

KoFontTab::KoFontTab( uint fontListCriteria, QWidget* parent, const char* name, Qt::WFlags fl ) 
        : KoFontTabBase( parent/*, name, fl*/ ) //FIXME apply fl argument
{
    QStringList list;
    KFontChooser_local::getFontList(list, fontListCriteria);

    characterFont->setSampleBoxVisible( false );
    characterFont->setFamilyList( list );
    comparisonFont = characterFont->font();
    connect( characterFont, SIGNAL( fontSelected( const QFont & ) ), this, SLOT( slotFontChanged( const QFont & ) ) );
}

KoFontTab::~KoFontTab()
{
}

QFont KoFontTab::getSelection()
{
    return characterFont->font();
}

void KoFontTab::setSelection( const QFont &font )
{
    characterFont->setFont( font );
}

void KoFontTab::slotFontChanged( const QFont &font )
{
    if ( comparisonFont.family() != font.family() ) emit familyChanged();
    if ( comparisonFont.bold() != font.bold() ) emit boldChanged();
    if ( comparisonFont.italic() != font.italic() ) emit italicChanged();
    if ( comparisonFont.pointSize() != font.pointSize() ) emit sizeChanged();
    comparisonFont = font;
}

#include "KoFontTab.moc"
