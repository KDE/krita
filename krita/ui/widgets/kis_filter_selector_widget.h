/*
 *  Copyright (c) 2007-2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_FILTER_SELECTOR_WIDGET_H_
#define _KIS_FILTER_SELECTOR_WIDGET_H_

#include <QWidget>
#include <QTreeView>
#include <QHeaderView>
#include <QDebug>
#include <QResizeEvent>
#include <QSize>

#include <kis_types.h>

class QModelIndex;
class KisFilterConfiguration;
class KisViewManager;
class QAbstractItemModel;
class QHideEvent;
class QShowEvent;

/**
 * XXX
 */
class KisFilterSelectorWidget : public QWidget
{
    Q_OBJECT
public:
    KisFilterSelectorWidget(QWidget* parent);
    ~KisFilterSelectorWidget();
    void setFilter(KisFilterSP f);
    void setView(KisViewManager *view);
    void setPaintDevice(bool showAll, KisPaintDeviceSP);
    KisFilterConfiguration* configuration();
    bool isFilterGalleryVisible() const;
    KisFilterSP currentFilter() const;
public Q_SLOTS:
    void setVisible(bool visible);
    void showFilterGallery(bool visible);
protected Q_SLOTS:
    void slotBookmarkedFilterConfigurationSelected(int);
    void setFilterIndex(const QModelIndex&);
    void editConfigurations();
    void update();
Q_SIGNALS:
    void configurationChanged();
    void sigFilterGalleryToggled(bool visible);
    void sigSizeChanged();
private:
    struct Private;
    Private* const d;
};


class KisFilterTree: public QTreeView
{
    Q_OBJECT

public:

    KisFilterTree(QWidget *parent) : QTreeView(parent) {
        connect(this, SIGNAL(expanded(QModelIndex)), this, SLOT(update_scroll_area(QModelIndex)));
        connect(this, SIGNAL(collapsed(QModelIndex)), this, SLOT(update_scroll_area(QModelIndex)));
    }

    void setFilterModel(QAbstractItemModel * model);
    void activateFilter(QModelIndex idx);

    QSize minimumSizeHint() const
    {
        return QSize(200, QTreeView::sizeHint().height());
    }

    QSize sizeHint() const
    {
        return QSize(header()->width(), QTreeView::sizeHint().height());
    }

    void setModel(QAbstractItemModel *model)
    {
        QTreeView::setModel(model);
        header()->setResizeMode(0, QHeaderView::ResizeToContents);
    }

protected:

    void resizeEvent(QResizeEvent *event)
    {
        if (event->size().width() > 10) {
            setModel(m_model);

        }
        else {
            setModel(0);
        }
        QTreeView::resizeEvent(event);
    }

    void showEvent(QShowEvent * event)
    {
        setModel(m_model);
        QTreeView::showEvent(event);
    }

    void hideEvent(QHideEvent * event)
    {
        setModel(0);
        QTreeView::hideEvent(event);
    }

private slots:
    void update_scroll_area(const QModelIndex& i)
    {
        resizeColumnToContents(i.column());
    }

private:

    QAbstractItemModel *m_model;

};

#endif
