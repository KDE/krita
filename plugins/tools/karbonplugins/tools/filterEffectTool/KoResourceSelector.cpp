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

#include <KisResourceModel.h>
#include <KisResourceModelProvider.h>
#include <KisResourceItemListView.h>
#include <KisResourceItemDelegate.h>
#include <QPainter>
#include <QTableView>
#include <QListView>
#include <QHeaderView>
#include <QMouseEvent>
#include <QStyledItemDelegate>

#include <WidgetsDebug.h>

class Q_DECL_HIDDEN KoResourceSelector::Private
{
public:
    Private() : displayMode(ImageMode) {}
    DisplayMode displayMode;
    KisResourceModel *model;

    void updateIndex( KoResourceSelector * me )
    {
        KisResourceModel *resourceModel = qobject_cast<KisResourceModel*>(me->model());
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

KoResourceSelector::KoResourceSelector(QWidget * parent )
    : QComboBox(parent), d(new Private())
{
    setView(new KisResourceItemListView(this));
    d->model = KisResourceModelProvider::resourceModel(ResourceType::FilterEffects);
    setModel(d->model);
    setItemDelegate(new KisResourceItemDelegate(this));
    setMouseTracking(true);
    d->updateIndex(this);

    connect( this, SIGNAL(currentIndexChanged(int)),
             this, SLOT(indexChanged(int)) );
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
    opt.initFrom( this );
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

        KoResourceSP resource = d->model->resourceForIndex(index);

        if (resource) {
            emit resourceApplied(resource);
        }
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


void KoResourceSelector::setDisplayMode(DisplayMode mode)
{
    if (mode == d->displayMode)
        return;

    switch(mode) {
    case ImageMode:
        setItemDelegate(new KisResourceItemDelegate(this));
        setView( new KisResourceItemListView(this) );
        break;
    case TextMode:
        setItemDelegate(new QStyledItemDelegate(this));
        setView(new QListView(this));
        break;
    }

    d->displayMode = mode;
    d->updateIndex(this);
}

void KoResourceSelector::setSingleColumn(bool singleColumn) {
    qDebug() << "setting single column" << singleColumn;
    KisResourceItemListView * listView = qobject_cast<KisResourceItemListView*>(view());
    if (!listView) return;
    if (singleColumn) {
        qDebug() << "resource item view exists and is set single column";
        listView->setViewMode(QListView::ListMode);
        listView->setItemSize(QSize(listView->viewport()->width(),listView->iconSize().height()));
    } else {
        listView->setViewMode(QListView::IconMode);
        listView->setItemSize(QSize(listView->iconSize().height(), listView->iconSize().height()));
    }
}

void KoResourceSelector::setRowHeight( int rowHeight )
{
    KisResourceItemListView * listView = qobject_cast<KisResourceItemListView*>(view());
    if (listView) {
        listView->setItemSize(QSize(listView->iconSize().width(), rowHeight));
    }
}

void KoResourceSelector::indexChanged( int )
{
    QModelIndex index = view()->currentIndex();
    if(!index.isValid()) {
        return;
    }
    KoResourceSP resource = d->model->resourceForIndex(index);
    if (resource) {
        emit resourceSelected( resource );
    }
}
