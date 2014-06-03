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

#ifndef KDCHARTABSTRACTTHREEDATTRIBUTES_H
#define KDCHARTABSTRACTTHREEDATTRIBUTES_H

#include <QMetaType>
#include "KDChartGlobal.h"

namespace KDChart {

/**
  * @brief Base class for 3D attributes
  */
class KDCHART_EXPORT AbstractThreeDAttributes
{
public:
    AbstractThreeDAttributes();
    AbstractThreeDAttributes( const AbstractThreeDAttributes& );
    AbstractThreeDAttributes &operator= ( const AbstractThreeDAttributes& );

    virtual ~AbstractThreeDAttributes() = 0;

    void setEnabled( bool enabled );
    bool isEnabled() const;

    void setDepth( double depth );
    double depth() const;

    // returns the depth(), if is isEnabled() is true, otherwise returns 0.0
    double validDepth() const;

    bool isThreeDBrushEnabled() const;
    void setThreeDBrushEnabled( bool enabled );
    virtual QBrush threeDBrush( const QBrush& brush, const QRectF& rect ) const;

    bool operator==( const AbstractThreeDAttributes& ) const;
    inline bool operator!=( const AbstractThreeDAttributes& other ) const { return !operator==(other); }

    KDCHART_DECLARE_PRIVATE_BASE_POLYMORPHIC(AbstractThreeDAttributes)

    KDCHART_DECLARE_SWAP_BASE(AbstractThreeDAttributes)

}; // End of class AbstractThreeDAttributes

}

#if !defined(QT_NO_DEBUG_STREAM)
KDCHART_EXPORT QDebug operator<<(QDebug, const KDChart::AbstractThreeDAttributes& );
#endif /* QT_NO_DEBUG_STREAM */



#endif // KDCHARTABSTRACTTHREEDATTRIBUTES_H
