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

#ifndef KDCHARTTERNARYPOINTDIAGRAM_P_H
#define KDCHARTTERNARYPOINTDIAGRAM_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KD Chart API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtDebug>

#include "KDChartAbstractTernaryDiagram_p.h"

#include <KDABLibFakes>

namespace KDChart {

/**
 * \internal
 */
    class TernaryPointDiagram::Private : public AbstractTernaryDiagram::Private
    {
        friend class TernaryPointDiagram;
    public:
        Private();
        ~Private() {}

        Private( const Private& rhs )
            : AbstractTernaryDiagram::Private( rhs )
        {
        }

    };

KDCHART_IMPL_DERIVED_DIAGRAM( TernaryPointDiagram, AbstractTernaryDiagram, TernaryCoordinatePlane )
/*
inline TernaryPointDiagram::TernaryPointDiagram( Private * p, TernaryCoordinatePlane* plane )
  : AbstractTernaryDiagram( p, plane ) { init(); }
inline TernaryPointDiagram::Private * TernaryPointDiagram::d_func()
{ return static_cast<Private*>( AbstractTernaryDiagram::d_func() ); }
inline const TernaryPointDiagram::Private * TernaryPointDiagram::d_func() const
{ return static_cast<const Private*>( AbstractTernaryDiagram::d_func() ); }
*/

}

#endif /* KDCHARTTERNARYPOINTDIAGRAM_P_H */

