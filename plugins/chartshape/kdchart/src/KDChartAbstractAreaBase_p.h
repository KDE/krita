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

#ifndef KDCHARTABSTRACTAREABASE_P_H
#define KDCHARTABSTRACTAREABASE_P_H

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

/** \file KDChartAbstractAreaBase_p.h
 *  \internal
 */

#include "KDChartAbstractAreaBase.h"
#include "KDChartTextAttributes.h"
#include "KDChartFrameAttributes.h"
#include "KDChartBackgroundAttributes.h"

#include <KDABLibFakes>


namespace KDChart {

/**
 * \internal
 */
    class AbstractAreaBase::Private
    {
        friend class AbstractAreaBase;
    public:
        explicit Private();
        virtual ~Private();

        Private( const Private& rhs ) :
            visible( rhs.visible ),
            frameAttributes( rhs.frameAttributes ),
            backgroundAttributes( rhs.backgroundAttributes )
            {
            }

    protected:
        void init();

        // These are set each time the area's sizeHint()
        // (or the maximumSize(), resp.) is calculated:
        // They store additional layout-information about
        // space needed around the area.
        // Other classes (e.g. KDChart::AutoSpacer) can use
        // these data to determine how much space has to
        // be added additionally ...
        mutable int amountOfLeftOverlap;
        mutable int amountOfRightOverlap;
        mutable int amountOfTopOverlap;
        mutable int amountOfBottomOverlap;

    private:
        bool visible;
        KDChart::FrameAttributes frameAttributes;
        KDChart::BackgroundAttributes backgroundAttributes;
    };

    inline AbstractAreaBase::AbstractAreaBase( AbstractAreaBase::Private * p ) :
        _d( p ) { init(); }

}
#endif /* KDCHARTABSTRACTAREABASE_P_H */

