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

