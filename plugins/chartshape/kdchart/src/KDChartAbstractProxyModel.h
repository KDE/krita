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

#ifndef KDCHARTABSTRACTPROXYMODEL_H
#define KDCHARTABSTRACTPROXYMODEL_H

#include <QAbstractProxyModel>

#include "KDChartGlobal.h"

namespace KDChart
{
    /**
      * @brief Base class for all proxy models used inside KD Chart
      * \internal
      */
    class KDCHART_EXPORT AbstractProxyModel : public QAbstractProxyModel
    {
        Q_OBJECT
    public:
        explicit AbstractProxyModel( QObject* parent = 0 );

        /*! \reimpl */ 
        QModelIndex mapFromSource( const QModelIndex & sourceIndex ) const;
        /*! \reimpl */ 
        QModelIndex mapToSource( const QModelIndex &proxyIndex ) const;

        /*! \reimpl */
        QModelIndex index( int row, int col, const QModelIndex& index ) const;
        /*! \reimpl */ 
        QModelIndex parent( const QModelIndex& index ) const;
    };
}

#endif /* KDCHARTABSTRACTPROXYMODEL_H */
