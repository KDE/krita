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
#include <QtGui/QListView>
#include <QtGui/QHeaderView>
#include <QtGui/QHeaderView>
#include <QtGui/QMouseEvent>
#include <QtGui/QStyledItemDelegate>

#include <KDebug>

class KoResourceSelector::Private
{
public:
    Private() : displayMode(ImageMode) {}
    DisplayMode displayMode;

    void updateIndex( KoResourceSelector * me )
    {
        KoResourceModel * resourceModel = qobject_cast<KoResourceModel*>(me->model());
        if (!resourceModel)
            return;
        if (!resourceModel->rowCount())
            return;

        int currentIndex = me->currentIndex();
        QModelIndex currentModelIndex = me->view()->currentIndex();

        if (currentIndex < 0 || !currentModelIndex.isValid()) {
            me->blockSignals(true);
            me->view()->setCurrentIndex( resourceModel->index( 0, 0 ) );
            me->setCurrentIndex(0);
            me->blockSignals(false);
            me->update();
        }
    }
};

KoResourceSelector::KoResourceSelector(QWidget * parent)
    : QComboBox( parent ), d( new Private() )
{
    connect( this, SIGNAL(currentIndexChanged(int)),
             this, SLOT(indexChanged(int)) );

    setMouseTracking(true);
}

KoResourceSelector::KoResourceSelector( KoAbstractResourceServerAdapter * resourceAdapter, QWidget * parent )
    : QComboBox( parent ), d( new Private() )
{
    Q_ASSERT(resourceAdapter);

    setView( new KoResourceItemView(this) );
    setModel( new KoResourceModel(resourceAdapter, this) );
    setItemDelegate( new KoResourceItemDelegate( this ) );
    setMouseTracking(true);
    d->updateIndex(this);

    connect( this, SIGNAL(currentIndexChanged(int)),
             this, SLOT(indexChanged(int)) );

    connect(resourceAdapter, SIGNAL(resourceAdded(KoResource*)),
            this, SLOT(resourceAdded(KoResource*)));
    connect(resourceAdapter, SIGNAL(removingResource(KoResource*)),
            this, SLOT(resourceRemoved(KoResource*)));
}

KoResourceSelector::~KoResourceSelector()
{
    delete d;
}

void KoResourceSelector::paintEvent( QPaintEvent *pe )
{
    QComboBox::paintEvent( pe );

    if (d->displayMode == ImageMode) {
        QStyleOptionComboBox option;
        option.initFrom( this );
        QRect r = style()->subControlRect( QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField, this );

        QStyleOptionViewItem viewOption;
        viewOption.initFrom( this );
        viewOption.rect = r;

        QPainter painter( this );
        itemDelegate()->paint( &painter, viewOption, view()->currentIndex() );
    }
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

void KoResourceSelector::setResourceAdapter(KoAbstractResourceServerAdapter *resourceAdapter)
{
    Q_ASSERT(resourceAdapter);
    setModel(new KoResourceModel(resourceAdapter, this));
    d->updateIndex(this);

    connect(resourceAdapter, SIGNAL(resourceAdded(KoResource*)),
            this, SLOT(resourceAdded(KoResource*)));
    connect(resourceAdapter, SIGNAL(removingResource(KoResource*)),
            this, SLOT(resourceRemoved(KoResource*)));
}

void KoResourceSelector::setDisplayMode(DisplayMode mode)
{
    if (mode == d->displayMode)
        return;

    switch(mode) {
    case ImageMode:
        setItemDelegate(new KoResourceItemDelegate(this));
        setView( new KoResourceItemView(this) );
        break;
    case TextMode:
        setItemDelegate(new QStyledItemDelegate(this));
        setView(new QListView(this));
        break;
    }

    d->displayMode = mode;
    d->updateIndex(this);
}

void KoResourceSelector::setColumnCount( int columnCount )
{
    KoResourceModel * resourceModel = qobject_cast<KoResourceModel*>(model());
    if (resourceModel)
        resourceModel->setColumnCount( columnCount );
}

void KoResourceSelector::setRowHeight( int rowHeight )
{
    QTableView * tableView = qobject_cast<QTableView*>(view());
    if (tableView)
        tableView->verticalHeader()->setDefaultSectionSize( rowHeight );
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

void KoResourceSelector::resourceAdded(KoResource*)
{
    d->updateIndex(this);
}

void KoResourceSelector::resourceRemoved(KoResource* r)
{
    d->updateIndex(this);
}

#include <KoResourceSelector.moc>
