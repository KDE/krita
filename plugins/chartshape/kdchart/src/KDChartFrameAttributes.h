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

#ifndef KDCHARTFRAMEATTRIBUTES_H
#define KDCHARTFRAMEATTRIBUTES_H

#include <QDebug>
#include <QMetaType>
#include <QPen>
#include "KDChartGlobal.h"

namespace KDChart {

/**
  * @brief A set of attributes for frames around items
  */
class KDCHART_EXPORT FrameAttributes
{
public:
    FrameAttributes();
    FrameAttributes( const FrameAttributes& );
    FrameAttributes &operator= ( const FrameAttributes& );

    ~FrameAttributes();

    void setVisible( bool visible );
    bool isVisible() const;

    void setPen( const QPen & pen );
    QPen pen() const;

    void setPadding( int padding );
    int padding() const;

    bool operator==( const FrameAttributes& ) const;
    inline bool operator!=( const FrameAttributes& other ) const { return !operator==(other); }

private:

    KDCHART_DECLARE_PRIVATE_BASE_VALUE( FrameAttributes )
}; // End of class FrameAttributes

}

#if !defined(QT_NO_DEBUG_STREAM)
KDCHART_EXPORT QDebug operator<<(QDebug, const KDChart::FrameAttributes& );
#endif /* QT_NO_DEBUG_STREAM */

Q_DECLARE_METATYPE( KDChart::FrameAttributes )
KDCHART_DECLARE_SWAP_SPECIALISATION( KDChart::FrameAttributes )
Q_DECLARE_TYPEINFO( KDChart::FrameAttributes, Q_MOVABLE_TYPE );

#endif // KDCHARTFRAMEATTRIBUTES_H
