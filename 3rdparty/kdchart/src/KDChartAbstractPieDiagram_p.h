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

#ifndef KDCHARTABSTRACTPIEDIAGRAM_P_H
#define KDCHARTABSTRACTPIEDIAGRAM_P_H

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

#include "KDChartAbstractPolarDiagram_p.h"
#include <KDChartAbstractThreeDAttributes.h>

#include <KDABLibFakes>


namespace KDChart {

class PolarCoordinatePlane;

/**
 * \internal
 */
class AbstractPieDiagram::Private : public AbstractPolarDiagram::Private
{
    friend class AbstractPieDiagram;
public:
    Private();
    ~Private();

    Private( const Private& rhs ) :
        AbstractPolarDiagram::Private( rhs ),
        granularity( rhs.granularity )
        {
        }

private:
    double granularity;
};

KDCHART_IMPL_DERIVED_DIAGRAM( AbstractPieDiagram, AbstractPolarDiagram, PolarCoordinatePlane )

}
#endif /* KDCHARTABSTRACTPIEDIAGRAM_P_H */

