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

#ifndef PAINTCONTEXT_H
#define PAINTCONTEXT_H

class QPainter;

#include <QRectF>
#include "KDChartGlobal.h"

namespace KDChart {

    class AbstractCoordinatePlane;

    /**
      * @brief Stores information about painting diagrams
      * \internal
      */
    class KDCHART_EXPORT PaintContext
    {
    public:
        PaintContext();
        ~PaintContext();

        const QRectF rectangle () const;
        void setRectangle( const QRectF& rect );

        QPainter* painter() const;
        void setPainter( QPainter* painter );

        AbstractCoordinatePlane* coordinatePlane() const;
        void setCoordinatePlane( AbstractCoordinatePlane* plane );

    private:
        class Private;
        Private * _d;
        Private * d_func() { return _d; }
        const Private * d_func() const { return _d; }
    };

}

#endif /* PAINTCONTEXT_H */

