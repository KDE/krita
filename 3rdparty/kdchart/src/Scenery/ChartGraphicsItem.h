/****************************************************************************
** Copyright (C) 2001-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Chart library.
**
** Licensees holding valid commercial KD Chart licenses may use this file in
** accordance with the KD Chart Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#ifndef CHARTGRAPHICSITEM_H
#define CHARTGRAPHICSITEM_H

#include <QGraphicsPolygonItem>

namespace KDChart {

    /**
      * @brief Graphics item used inside of the ReverseMapper
      * \internal
      */
    class ChartGraphicsItem : public QGraphicsPolygonItem
    {
    public:
        enum { Type = UserType + 1 };

        ChartGraphicsItem();

        ChartGraphicsItem( int row,  int column );

        int row() const { return m_row; }
        int column() const { return m_column; }
        int type() const { return Type; }

    private:
        int m_row;
        int m_column;
    };

}

#endif
