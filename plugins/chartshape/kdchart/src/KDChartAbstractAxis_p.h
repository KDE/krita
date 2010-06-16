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

#ifndef KDCHARTAXIS_P_H
#define KDCHARTAXIS_P_H

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

#include <QQueue>
#include <QTimer>

#include "KDChartAbstractArea_p.h"
#include "KDChartAbstractDiagram.h"
#include <KDChartTextAttributes.h>
#include <KDChartRulerAttributes.h>
#include <KDChartDiagramObserver.h>

#include <KDABLibFakes>


namespace KDChart {

/**
 * \internal
 */
class AbstractAxis::Private : public AbstractArea::Private
{
    friend class AbstractAxis;

public:
    Private( AbstractDiagram* diagram, AbstractAxis* axis );
    ~Private();

    bool setDiagram(   AbstractDiagram* diagram, bool delayedInit = false );
    void unsetDiagram( AbstractDiagram* diagram );
    const AbstractDiagram* diagram() const
    {
        return mDiagram;
    }
    bool hasDiagram( AbstractDiagram* diagram ) const;

    DiagramObserver* observer;

    TextAttributes textAttributes;
    RulerAttributes rulerAttributes;
    QStringList hardLabels;
    QStringList hardShortLabels;
    QQueue<AbstractDiagram*> secondaryDiagrams;

protected:
    AbstractDiagram* mDiagram;
    AbstractAxis*    mAxis;
};


inline AbstractAxis::AbstractAxis( Private * p, AbstractDiagram* diagram )
    :  AbstractArea( p )
{
    Q_UNUSED( diagram );
    init();
    QTimer::singleShot(0, this, SLOT(delayedInit()));
}

inline AbstractAxis::Private * AbstractAxis::d_func()
{ return static_cast<Private*>( AbstractArea::d_func() ); }
inline const AbstractAxis::Private * AbstractAxis::d_func() const
{ return static_cast<const Private*>( AbstractArea::d_func() ); }

}
#endif /* KDCHARTAREA_P_H */

