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


#include <QtCore/QtPlugin>


#define ChartShapeId "ChartShape"

class QAbstractItemModel;
class QRect;


namespace KoChart
{

class ChartInterface
{
public:
    virtual ~ChartInterface() {}

    virtual void setModel(QAbstractItemModel* model, bool takeOwnershipOfModel = false) = 0;
    virtual void setModel(QAbstractItemModel* model, const QVector<QRect> &selection) = 0;
    virtual void setFirstRowIsLabel(bool isLabel) = 0;
    virtual void setFirstColumnIsLabel(bool isLabel) = 0;
    virtual void setDataDirection(Qt::Orientation orientation) = 0;
};

} // namespace KoChart

Q_DECLARE_INTERFACE(KoChart::ChartInterface, "org.koffice.KoChart.ChartInterface:1.0")

#endif // KOCHART_INTERFACE
