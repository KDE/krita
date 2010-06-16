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

#ifndef KDCHART_TEXT_AREA_P_H
#define KDCHART_TEXT_AREA_P_H

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

/** \file KDChartTextArea_p.h
 *  \internal
 */

#include "KDChartTextArea.h"
#include "KDChartAbstractAreaBase_p.h"

#include <KDABLibFakes>


namespace KDChart {

/**
 * \internal
 */
    class TextArea::Private : public AbstractAreaBase::Private
    {
        friend class TextArea;
    public:
        explicit Private();
        virtual ~Private();

        Private( const Private& rhs ) :
            AbstractAreaBase::Private( rhs )
            {
                // Just for consistency
            }
    };


    inline TextArea::TextArea( Private * p )
        :  QObject(), AbstractAreaBase( p ), TextLayoutItem()
    {
        init();
    }
    inline TextArea::Private * TextArea::d_func()
    {
        return static_cast<Private*>( AbstractAreaBase::d_func() );
    }
    inline const TextArea::Private * TextArea::d_func() const
    {
        return static_cast<const Private*>( AbstractAreaBase::d_func() );
    }

}

#endif /* KDCHART_TEXT_AREA_P_H */

