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

#ifndef KDCHARTHEADERFOOTER_P_H
#define KDCHARTHEADERFOOTER_P_H

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

#include "KDChartHeaderFooter.h"
#include "KDChartTextArea_p.h"
#include "KDChartChart.h"
#include <QMap>

#include <KDABLibFakes>


class KDTextDocument;

namespace KDChart {
    class Chart;
    class TextAttributes;
}

/**
 * \internal
 */
class KDChart::HeaderFooter::Private : public KDChart::TextArea::Private
{
    friend class KDChart::HeaderFooter;
public:
    explicit Private();
    ~Private();

    Private( const Private& rhs ) :
        TextArea::Private( rhs ),
        type( rhs.type ),
        position( rhs.position )
        {
        }

    void updateTextDoc();

private:
    // user-settable
    HeaderFooterType type;
    Position position;
};

inline KDChart::HeaderFooter::HeaderFooter( Private* d, KDChart::Chart* parent )
    : TextArea( d )
{
    setParent( parent );
    init();
}
inline KDChart::HeaderFooter::Private * KDChart::HeaderFooter::d_func()
{
    return static_cast<Private*>( TextArea::d_func() );
}
inline const KDChart::HeaderFooter::Private * KDChart::HeaderFooter::d_func() const
{
    return static_cast<const Private*>( TextArea::d_func() );
}


#endif /* KDCHARTHEADERFOOTER_P_H */
