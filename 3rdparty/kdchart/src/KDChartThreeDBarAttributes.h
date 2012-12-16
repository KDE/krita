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

#ifndef KDCHARTTHREEDBARATTRIBUTES_H
#define KDCHARTTHREEDBARATTRIBUTES_H

#include <QMetaType>
#include "KDChartAbstractThreeDAttributes.h"
#include "KDChartGlobal.h"

namespace KDChart {

  /**
    * @brief A set of 3D bar attributes
    */
  class KDCHART_EXPORT ThreeDBarAttributes : public AbstractThreeDAttributes
  {
  public:
    ThreeDBarAttributes();
    ThreeDBarAttributes( const ThreeDBarAttributes& );
    ThreeDBarAttributes &operator= ( const ThreeDBarAttributes& );

    ~ThreeDBarAttributes();

    /* threeD Bars specific */
    void setUseShadowColors( bool useShadowColors );
    bool useShadowColors() const;

    //Pending Michel I am not sure this will be used
    void setAngle( uint threeDAngle );
    uint angle() const;

    bool operator==( const ThreeDBarAttributes& ) const;
    inline bool operator!=( const ThreeDBarAttributes& other ) const { return !operator==(other); }

    KDCHART_DECLARE_SWAP_DERIVED(ThreeDBarAttributes)

    KDCHART_DECLARE_PRIVATE_DERIVED(ThreeDBarAttributes)

  }; // End of class ThreeDBarAttributes

}

#if !defined(QT_NO_DEBUG_STREAM)
KDCHART_EXPORT QDebug operator<<(QDebug, const KDChart::ThreeDBarAttributes& );
#endif /* QT_NO_DEBUG_STREAM */

Q_DECLARE_METATYPE( KDChart::ThreeDBarAttributes )
Q_DECLARE_TYPEINFO( KDChart::ThreeDBarAttributes, Q_MOVABLE_TYPE );
KDCHART_DECLARE_SWAP_SPECIALISATION_DERIVED( KDChart::ThreeDBarAttributes )

#endif // KDCHARTTHREEDBARATTRIBUTES_H
