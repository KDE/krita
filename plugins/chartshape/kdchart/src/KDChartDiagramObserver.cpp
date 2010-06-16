/* -*- Mode: C++ -*-
   KDChart - a multi-platform charting engine
   */

/****************************************************************************
 ** Copyright (C) 2001-2007 Klarävdalens Datakonsult AB.  All rights reserved.
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

#include <KDChartDiagramObserver.h>
#include <KDChartAbstractDiagram.h>
#include <KDChartAttributesModel.h>

#include <KDABLibFakes>

#include <QDebug>

using namespace KDChart;

DiagramObserver::DiagramObserver( AbstractDiagram * diagram, QObject* parent )
    : QObject( parent ), m_diagram( diagram )
{
    if ( m_diagram ) {
        connect( m_diagram, SIGNAL(destroyed(QObject*)), SLOT(slotDestroyed(QObject*)));
        connect( m_diagram, SIGNAL(modelsChanged()), SLOT(slotModelsChanged()));
    }
    init();
}

DiagramObserver::~DiagramObserver()
{
}

const AbstractDiagram* DiagramObserver::diagram() const
{
    return m_diagram;
}

AbstractDiagram* DiagramObserver::diagram()
{
    return m_diagram;
}


void DiagramObserver::init()
{
    if ( !m_diagram )
        return;

    if ( m_model )
        disconnect(m_model);

    if ( m_attributesmodel )
        disconnect(m_attributesmodel);

    connect( m_diagram, SIGNAL(dataHidden()), SLOT(slotDataHidden()) );

    if( m_diagram->model() ){
        connect( m_diagram->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                 SLOT(slotDataChanged(QModelIndex,QModelIndex)));
        connect( m_diagram->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                 SLOT(slotDataChanged()));
        connect( m_diagram->model(), SIGNAL(columnsInserted(QModelIndex,int,int)),
                 SLOT(slotDataChanged()));
        connect( m_diagram->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                 SLOT(slotDataChanged()));
        connect( m_diagram->model(), SIGNAL(columnsRemoved(QModelIndex,int,int)),
                 SLOT(slotDataChanged()));
        connect( m_diagram->model(), SIGNAL(modelReset()),
                 SLOT(slotDataChanged()));
        connect( m_diagram->model(), SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                 SLOT(slotHeaderDataChanged(Qt::Orientation,int,int)));
    }

    if( m_diagram->attributesModel() )
        connect( m_diagram->attributesModel(), SIGNAL(attributesChanged(QModelIndex,QModelIndex)),
                 SLOT(slotAttributesChanged(QModelIndex,QModelIndex)));
    m_model = m_diagram->model();
    m_attributesmodel = m_diagram->attributesModel();
}


void DiagramObserver::slotDestroyed(QObject*)
{
    //qDebug() << this << "emits signal\n"
    //        "    emit diagramDestroyed(" <<  m_diagram << ")";
    AbstractDiagram* diag = m_diagram;
    disconnect( m_diagram, 0, this, 0);
    m_diagram = 0;
    emit diagramDestroyed( diag );
}

void DiagramObserver::slotModelsChanged()
{
    init();
    slotDataChanged();
    slotAttributesChanged();
}

void DiagramObserver::slotHeaderDataChanged(Qt::Orientation,int,int)
{
    //qDebug() << "DiagramObserver::slotHeaderDataChanged()";
    emit diagramDataChanged( m_diagram );
}

void DiagramObserver::slotDataChanged(QModelIndex,QModelIndex)
{
    slotDataChanged();
}

void DiagramObserver::slotDataChanged()
{
    //qDebug() << "DiagramObserver::slotDataChanged()";
    emit diagramDataChanged( m_diagram );
}

void DiagramObserver::slotDataHidden()
{
    //qDebug() << "DiagramObserver::slotDataHidden()";
    emit diagramDataHidden( m_diagram );
}

void DiagramObserver::slotAttributesChanged(QModelIndex,QModelIndex)
{
    slotAttributesChanged();
}

void DiagramObserver::slotAttributesChanged()
{
    //qDebug() << "DiagramObserver::slotAttributesChanged()";
    emit diagramAttributesChanged( m_diagram );
}

