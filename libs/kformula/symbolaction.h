/* This file is part of the KDE project
   Copyright (C) 2002 Heinrich Kuettler <heinrich.kuettler@gmx.de>

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

#ifndef _SYMBOLACTION_H_
#define _SYMBOLACTION_H_

#include <kaction.h>
#include <q3valuelist.h>
#include <qstringlist.h>
#include <q3memarray.h>

#include "kformuladefs.h"

KFORMULA_NAMESPACE_BEGIN

class SymbolAction : public KSelectAction
{
public:
    SymbolAction( KActionCollection* parent = 0, const char* name = 0 );
    SymbolAction( const QString& text, const KShortcut& cut,
                 const QObject* receiver, const char* slot, KActionCollection* parent,
                 const char* name = 0 );

    int plug( QWidget*, int index = -1 );
    void setSymbols( const QStringList&, const Q3ValueList<QFont>&, const Q3MemArray<QChar>& );
    void updateItems( int );

private:
    Q3ValueList<QFont> m_fonts;
    Q3MemArray<QChar> m_chars;
};

KFORMULA_NAMESPACE_END

#endif // _SYMBOLACTION_H_
