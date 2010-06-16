/****************************************************************************
 ** Copyright (C) 2007 Klaralvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/

#ifndef KDCHARTLEVEYJENNINGSDIAGRAM_P_H
#define KDCHARTLEVEYJENNINGSDIAGRAM_P_H

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

#include <QDateTime>

#include "KDChartThreeDLineAttributes.h"
#include "KDChartLineDiagram_p.h"

#include <KDABLibFakes>

class QSvgRenderer;

namespace KDChart {

    class PaintContext;

/**
 * \internal
 */
    class LeveyJenningsDiagram::Private : public LineDiagram::Private
    {
        friend class LeveyJenningsDiagram;
    public:
        Private();
        Private( const Private& rhs );
        ~Private();

        void setYAxisRange() const;

        Qt::Alignment lotChangedPosition;
        Qt::Alignment fluidicsPackChangedPosition;
        Qt::Alignment sensorChangedPosition;

        QVector< QDateTime > fluidicsPackChanges;
        QVector< QDateTime > sensorChanges;

        QPen scanLinePen;

        QMap< LeveyJenningsDiagram::Symbol, QString >  icons;
        QMap< LeveyJenningsDiagram::Symbol, QSvgRenderer* > iconRenderer;

        QPair< QDateTime, QDateTime > timeRange;

        float expectedMeanValue;
        float expectedStandardDeviation;

        mutable float calculatedMeanValue;
        mutable float calculatedStandardDeviation;
    };

    KDCHART_IMPL_DERIVED_DIAGRAM( LeveyJenningsDiagram, LineDiagram, LeveyJenningsCoordinatePlane )
}

#endif /* KDCHARTLINEDIAGRAM_P_H */
