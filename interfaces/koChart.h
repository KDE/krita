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

#ifndef __koffice_kchart_h__
#define __koffice_kchart_h__

#include <qvariant.h>

#include <KoDocument.h>
#include <KDChartTable.h>

#include <koffice_export.h>

namespace KoChart
{
    class Part;

    // KoChart::Value is either:
    //  - a double (interpreted as a value)
    //  - a QString (interpreted as a label)
    //  - a QDateTime (interpreted as a date/time value)
    //  - Invalid (interpreted as empty)
    typedef QVariant         Value;
    //typedef KDChartTableData Data;

    class KOCHARTINTERFACE_EXPORT WizardExtension : public QObject
    {
        Q_OBJECT
    public:
        WizardExtension( Part *part, const char *name = 0 );
        virtual ~WizardExtension();

        Part *part() const { return m_part; }

        virtual bool show( QString &area ) = 0;
        // XXX add more?

    private:
        Part *m_part;
        class WizardExtensionPrivate;
        WizardExtensionPrivate *d;
    };

    class KOCHARTINTERFACE_EXPORT Part : public KoDocument
    {
        Q_OBJECT
    public:
        Part( QWidget *parentWidget, const char *widgetName,
              QObject *parent, const char *name,
              bool singleViewMode = false );

        virtual ~Part();

#if 0
	// The old interface.
        virtual void setData( const Data &d ) = 0;
#else
	// The new interface.
	virtual void resizeData( int rows, int columns ) = 0 ;
	virtual void setCellData( int row, int column, const QVariant &) = 0;
	virtual void analyzeHeaders( ) = 0;
#endif
        virtual void setCanChangeValue(bool b )=0;

        virtual WizardExtension *wizardExtension();
    private:
        class PartPrivate;
        PartPrivate *d;
    };
}

#endif
