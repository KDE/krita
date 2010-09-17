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

#ifndef KOCHART_INTERFACE
#define KOCHART_INTERFACE


#include <QtPlugin>
#include <Qt>


#define ChartShapeId "ChartShape"

class QAbstractItemModel;
class QString;


namespace KoChart
{

/**
 * Interface for ChartShape to embed it into a spreadsheet.
 */
class ChartInterface
{
public:
    virtual ~ChartInterface() {}

    /**
     * Sets the SheetAccessModel to be used by this chart. Use this method if
     * you want to embed the ChartShape into a spreadsheet.
     *
     * See kspread/SheetAccessModel.h for details.
     */
    virtual void setSheetAccessModel(QAbstractItemModel* model) = 0;

    /**
     * Re-initializes the chart with data from an arbitrary region.
     *
     * @param region             Name of region to use, e.g. "Table1.A1:B3"
     * @param firstRowIsLabel    Whether to interpret the first row as labels
     * @param firstColumnIsLabel Whether to interpret the first column as labels
     * @param dataDirection      orientation of a data set. Qt::Horizontal means a row is
     *                           to be interpreted as one data set, columns with Qt::Vertical.
     */
    virtual void reset(const QString& region,
                       bool firstRowIsLabel,
                       bool firstColumnIsLabel,
                       Qt::Orientation dataDirection) = 0;
};

} // namespace KoChart

Q_DECLARE_INTERFACE(KoChart::ChartInterface, "org.koffice.KoChart.ChartInterface:1.0")

#endif // KOCHART_INTERFACE

