/* This file is part of the KDE project

   Copyright 2008 Johannes Simon <johannes.simon@gmail.com>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KO_CHART_MODEL
#define KO_CHART_MODEL

#include "kochart_export.h"

#include <QVector>
#include <QObject>

class QAbstractItemModel;
class QRect;

namespace KoChart {

/**
* Item data role used to retrieve the string representing the data area
* of a row or a column.
*
* Example:
* The area string of the 3rd row would be retrieved with this line of code:
* QString area = model->headerData( 2, Qt::Vertical, SECTION_AREA_ROLE );
* ("$Table2.$D9:$D13", for instance)
*/
const int SECTION_AREA_ROLE = 32;

/**
* Item data role used to retrieve the string representing the data cell
* of a header. The header data usually is a name for the dataset.
*
* Example:
* The area string of the name of the data series that has its y-values
* in the 5th column could be retrieved with this line of code:
* QString cell = model->headerData( 4, Qt::Horizontal, HEADER_AREA_ROLE );
* ("$Table1.$C8", for instance)
*/
const int HEADER_AREA_ROLE  = 33;

/**
* The ChartModel class implements a model that can be filled and
* passed on to KChart to provide the data used within the chart.
*/
class KOCHART_EXPORT ChartModel : public QObject
{
public:
    ChartModel(QObject *parent = 0);
    virtual ~ChartModel();

    virtual QString regionToString( const QVector<QRect> &region ) const = 0;
    virtual QVector<QRect> stringToRegion( const QString &string ) const = 0;
    virtual QAbstractItemModel * model() = 0;
};

} // Namespace KoChart

#endif // KO_CHART_MODEL

