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

