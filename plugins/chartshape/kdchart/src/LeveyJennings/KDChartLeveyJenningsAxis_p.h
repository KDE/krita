/****************************************************************************
 ** Copyright (C) 2007 Klarälvdalens Datakonsult AB.  All rights reserved.
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

#ifndef KDCHARTLEVEYJENNINGSAXIS_P_H
#define KDCHARTLEVEYJENNINGSAXIS_P_H

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

#include "KDChartLeveyJenningsAxis.h"
#include "KDChartLeveyJenningsDiagram.h"
#include "KDChartCartesianAxis_p.h"

#include <KDABLibFakes>


namespace KDChart {

/**
  * \internal
  */
class LeveyJenningsAxis::Private : public CartesianAxis::Private
{
    friend class LeveyJenningsAxis;

public:
    Private( LeveyJenningsDiagram* diagram, LeveyJenningsAxis* axis )
        : CartesianAxis::Private( diagram, axis )
    {}
    ~Private() {}

private:
    LeveyJenningsGridAttributes::GridType type;
    Qt::DateFormat format;
};

inline LeveyJenningsAxis::LeveyJenningsAxis( Private * p, AbstractDiagram* diagram )
    : CartesianAxis( p, diagram )
{
    init();
}
inline LeveyJenningsAxis::Private * LeveyJenningsAxis::d_func()
{ return static_cast<Private*>( CartesianAxis::d_func() ); }
inline const LeveyJenningsAxis::Private * LeveyJenningsAxis::d_func() const
{ return static_cast<const Private*>( CartesianAxis::d_func() ); }

}

#endif
