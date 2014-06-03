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

#ifndef __KDCHARTDIAGRAMOBSERVER_H_
#define __KDCHARTDIAGRAMOBSERVER_H_

#include "KDChartGlobal.h"

#include <QObject>
#include <QPointer>
#include <QModelIndex>

class QAbstractItemModel;

namespace KDChart {

    class AbstractDiagram;

    /**
     * \brief A DiagramObserver watches the associated diagram for
     * changes and deletion and emits corresponsing signals.
     */
    class KDCHART_EXPORT DiagramObserver : public QObject
    {
        Q_OBJECT
    public:
       /**
         * Constructs a new observer observing the given diagram.
         */
        explicit DiagramObserver( AbstractDiagram * diagram, QObject* parent = 0 );
        ~DiagramObserver();

        const AbstractDiagram* diagram() const;
        AbstractDiagram* diagram();

    Q_SIGNALS:
        /** This signal is emitted immediately before the diagram is
          * being destroyed. */
        void diagramDestroyed( AbstractDiagram* diagram );
        /** Emitted when a diagram is being destroyed, but before its data is invalidated **/
        void diagramAboutToBeDestroyed( AbstractDiagram* diagram );
        /** This signal is emitted whenever the data of the diagram changes. */
        void diagramDataChanged( AbstractDiagram* diagram );
        /** This signal is emitted whenever any of the data of the diagram was set (un)hidden. */
        void diagramDataHidden( AbstractDiagram* diagram );
        /** This signal is emitted whenever the attributes of the diagram change. */
        void diagramAttributesChanged( AbstractDiagram* diagram );

    private Q_SLOTS:
        void slotDestroyed(QObject*);
        void slotAboutToBeDestroyed();
        void slotHeaderDataChanged(Qt::Orientation,int,int);
        void slotDataChanged(QModelIndex,QModelIndex);
        void slotDataChanged();
        void slotDataHidden();
        void slotAttributesChanged();
        void slotAttributesChanged(QModelIndex,QModelIndex);
        void slotModelsChanged();

    private:
        void init();

        AbstractDiagram*    m_diagram;
        QPointer<QAbstractItemModel> m_model;
        QPointer<QAbstractItemModel> m_attributesmodel;
   };
}

#endif // KDChartDiagramObserver_H
