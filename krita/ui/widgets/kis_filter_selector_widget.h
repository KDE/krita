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
#include <QDebug>
#include <QResizeEvent>
#include <QSize>

#include <kis_types.h>

class QModelIndex;
class KisFilterConfiguration;
class KisView2;
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
    void setView(KisView2 *view);
    void setPaintDevice(KisPaintDeviceSP);
    KisFilterConfiguration* configuration();
    void showFilterGallery(bool visible);
    bool isFilterGalleryVisible() const;
protected slots:
    void slotBookmarkedFilterConfigurationSelected(int);
    void setFilterIndex(const QModelIndex&);
    void editConfigurations();
signals:
    void configurationChanged();
private:
    struct Private;
    Private* const d;
};


class KisFilterTree: public QTreeView {

public:

    KisFilterTree(QWidget *parent) : QTreeView(parent) {}

    void setFilterModel(QAbstractItemModel * model)
    {
        m_model = model;
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

private:

    QAbstractItemModel *m_model;

};

#endif
