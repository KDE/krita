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

#ifndef KDCHARTABSTRACTAREAWIDGET_P_H
#define KDCHARTABSTRACTAREAWIDGET_P_H

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

/** \file KDChartAbstractAreaWidget_p.h
 *  \internal
 */

#include "KDChartAbstractAreaWidget.h"
#include "KDChartAbstractAreaBase_p.h"

#include <KDABLibFakes>


namespace KDChart {

/**
 * \internal
 */
class AbstractAreaWidget::Private : public AbstractAreaBase::Private
{
    friend class AbstractAreaWidget;
public:
    explicit Private();
    virtual ~Private();

    Private( const Private& rhs ) :
        AbstractAreaBase::Private( rhs )
        {
            // Just for consistency
        }

    QSize currentLayoutSize;

    // non-virtual method, calling widget->resizeLayout( size )
    void resizeLayout( AbstractAreaWidget* widget, const QSize& sz ); 
};


inline AbstractAreaWidget::AbstractAreaWidget( AbstractAreaWidget::Private * p, QWidget* parent )
  : QWidget( parent ), AbstractAreaBase( p )
{
    init();
}
inline AbstractAreaWidget::Private * AbstractAreaWidget::d_func()
{
    return static_cast<Private*>( AbstractAreaBase::d_func() );
}
inline const AbstractAreaWidget::Private * AbstractAreaWidget::d_func() const
{
    return static_cast<const Private*>( AbstractAreaBase::d_func() );
}


}

#endif /* KDCHARTABSTRACTAREAWIDGET_P_H */

