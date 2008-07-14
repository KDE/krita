/* This file is part of the KDE project
   Copyright (C) 2000-2002 Kalle Dalheimer <kalle@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "koChart.h"

#include <qobject.h>

using namespace KoChart;

WizardExtension::WizardExtension( Part *part )
    : QObject( part )
{
    m_part = part;
}

WizardExtension::~WizardExtension()
{
}

Part::Part( QWidget *parentWidget,
            QObject *parent,
            bool singleViewMode )
    : KoDocument( parentWidget, parent, singleViewMode )
{
}

Part::~Part()
{
}

WizardExtension *Part::wizardExtension()
{
    QObjectList::ConstIterator end( QObject::children().end() );
    for (QObjectList::ConstIterator it( QObject::children().begin() ); it != end; ++it ) {
      WizardExtension* we = ::qobject_cast<WizardExtension *>( *it );
      if ( we )
          return we;
    }

    return 0;
}

#include "koChart.moc"
