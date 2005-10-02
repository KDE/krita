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

#include <qobjectlist.h>

using namespace KoChart;

WizardExtension::WizardExtension( Part *part, const char *name )
    : QObject( part, name )
{
    m_part = part;
}

WizardExtension::~WizardExtension()
{
}

Part::Part( QWidget *parentWidget, const char *widgetName,
            QObject *parent, const char *name,
            bool singleViewMode )
    : KoDocument( parentWidget, widgetName, parent, name, singleViewMode )
{
}

Part::~Part()
{
}

WizardExtension *Part::wizardExtension()
{
    QObjectListIt it( *QObject::children() );
    for (; it.current(); ++it )
        if ( it.current()->inherits( "KoChart::WizardExtension" ) )
            return static_cast<WizardExtension *>( it.current() );

    return 0;
}

#include "koChart.moc"
