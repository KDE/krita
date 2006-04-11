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

#ifndef __kolanguagetab_h__
#define __kolanguagetab_h__

#include <kolanguagetabbase.h>

#include <kspell2/broker.h>

class KoLanguageTab : public KoLanguageTabBase
{
    Q_OBJECT

public:
    KoLanguageTab( KSpell2::Broker::Ptr broker = KSpell2::Broker::Ptr(), QWidget* parent=0, const char* name=0, Qt::WFlags fl=0 );
    ~KoLanguageTab();

    QString getLanguage() const;
    void setLanguage( const QString &item );

signals:
    void languageChanged( int item );
};

#endif
