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

#ifndef KDCHARTTHREEDLINEATTRIBUTES_H
#define KDCHARTTHREEDLINEATTRIBUTES_H

#include <QMetaType>
#include "KDChartAbstractThreeDAttributes.h"
#include "KDChartGlobal.h"

namespace KDChart {

  /**
    * @brief A set of 3D line attributes
    */
  class KDCHART_EXPORT ThreeDLineAttributes : public AbstractThreeDAttributes
  {
  public:
    ThreeDLineAttributes();
    ThreeDLineAttributes( const ThreeDLineAttributes& );
    ThreeDLineAttributes &operator= ( const ThreeDLineAttributes& );

    ~ThreeDLineAttributes();

    /* threeD lines specific */
    void setLineXRotation( const uint degrees );
    uint lineXRotation() const;
    void setLineYRotation( const uint degrees );
    uint lineYRotation() const;

    bool operator==( const ThreeDLineAttributes& ) const;
    inline bool operator!=( const ThreeDLineAttributes& other ) const { return !operator==(other); }

    KDCHART_DECLARE_SWAP_DERIVED(ThreeDLineAttributes)

    KDCHART_DECLARE_PRIVATE_DERIVED(ThreeDLineAttributes)

  }; // End of class ThreeDLineAttributes

}

#if !defined(QT_NO_DEBUG_STREAM)
KDCHART_EXPORT QDebug operator<<(QDebug, const KDChart::ThreeDLineAttributes& );
#endif /* QT_NO_DEBUG_STREAM */

Q_DECLARE_METATYPE( KDChart::ThreeDLineAttributes )
Q_DECLARE_TYPEINFO( KDChart::ThreeDLineAttributes, Q_MOVABLE_TYPE );
KDCHART_DECLARE_SWAP_SPECIALISATION_DERIVED( KDChart::ThreeDLineAttributes )


#endif // KDCHARTTHREEDLINEATTRIBUTES_H
