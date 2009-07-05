/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoResourceSelector.h"
#include <KoResourceServerAdapter.h>
#include <KoResourceModel.h>
#include <KoResourceItemView.h>
#include <KoResourceItemDelegate.h>
#include <QtGui/QPainter>
#include <QtGui/QTableView>
#include <QtGui/QHeaderView>
#include <QtGui/QHeaderView>
#include <QtGui/QMouseEvent>

#include <KDebug>

class KoResourceSelector::Private
{
public:
    Private() : model(0), view(0) {}
    KoResourceModel * model;
    KoResourceItemView * view;
};

KoResourceSelector::KoResourceSelector( KoAbstractResourceServerAdapter * resourceAdapter, QWidget * parent )
    : QComboBox( parent ), d( new Private() )
{
    d->model = new KoResourceModel(resourceAdapter, this); 
    d->view = new KoResourceItemView(this); 
    connect( this, SIGNAL(currentIndexChanged(int)),
             this, SLOT(indexChanged(int)) );
    Q_ASSERT(resourceAdapter);
    setView( d->view );
    setModel( d->model );
    setItemDelegate( new KoResourceItemDelegate( this ) );
    setMouseTracking(true);

    d->view->setCurrentIndex( d->model->index( 0, 0 ) );
}

KoResourceSelector::~KoResourceSelector()
{
    delete d;
}

void KoResourceSelector::paintEvent( QPaintEvent *pe )
{
    QComboBox::paintEvent( pe );

    QStyleOptionComboBox option;
    option.initFrom( this );
    QRect r = style()->subControlRect( QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField, this );

    QStyleOptionViewItem viewOption;
    viewOption.initFrom( this );
    viewOption.rect = r;
    
    QPainter painter( this );
    itemDelegate()->paint( &painter, viewOption, view()->currentIndex() );
}

void KoResourceSelector::mousePressEvent( QMouseEvent * event )
{
    QStyleOptionComboBox opt;
    opt.init( this );
    opt.subControls = QStyle::SC_All;
    opt.activeSubControls = QStyle::SC_ComboBoxArrow;
    QStyle::SubControl sc = style()->hitTestComplexControl(QStyle::CC_ComboBox, &opt,
                                                           mapFromGlobal(event->globalPos()),
                                                           this);
    // only clicking on combobox arrow shows popup,
    // otherwise the resourceApplied signal is send with the current resource
    if (sc == QStyle::SC_ComboBoxArrow)
        QComboBox::mousePressEvent( event );
    else {
        QModelIndex index = view()->currentIndex();
        if( ! index.isValid() )
            return;

        KoResource * resource = static_cast<KoResource*>( index.internalPointer() );
        if( resource )
            emit resourceApplied( resource );
    }
}

void KoResourceSelector::mouseMoveEvent( QMouseEvent * event )
{
    QStyleOptionComboBox option;
    option.initFrom( this );
    QRect r = style()->subControlRect( QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField, this );
    if (r.contains(event->pos()))
        setCursor(Qt::PointingHandCursor);
    else
        unsetCursor();
}

void KoResourceSelector::setColumnCount( int columnCount )
{
    d->model->setColumnCount( columnCount );
}

void KoResourceSelector::setRowHeight( int rowHeight )
{
    d->view->verticalHeader()->setDefaultSectionSize( rowHeight );
}

void KoResourceSelector::indexChanged( int )
{
    QModelIndex index = view()->currentIndex();
    if( ! index.isValid() )
        return;

    KoResource * resource = static_cast<KoResource*>( index.internalPointer() );
    if( resource )
        emit resourceSelected( resource );
}

#include "KoResourceSelector.moc"
