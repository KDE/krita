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
#include <QResizeEvent>
#include <QSize>

#include <kis_debug.h>
#include <KisKineticScroller.h>
#include <kis_types.h>

class QModelIndex;
class KisFilterConfiguration;
class KisViewManager;
class QAbstractItemModel;
class QHideEvent;
class QShowEvent;

/**
 * Widget for selecting the filter. This shows the widget if there is any.
 */
class KisFilterSelectorWidget : public QWidget
{
    Q_OBJECT
public:
    KisFilterSelectorWidget(QWidget* parent);
    ~KisFilterSelectorWidget() override;
    void setFilter(KisFilterSP f);
    void setView(KisViewManager *view);
    void setPaintDevice(bool showAll, KisPaintDeviceSP);
    KisFilterConfigurationSP configuration();
    bool isFilterGalleryVisible() const;
    KisFilterSP currentFilter() const;
public Q_SLOTS:
    void setVisible(bool visible) override;
    void showFilterGallery(bool visible);
protected Q_SLOTS:
    void slotBookmarkedFilterConfigurationSelected(int);
    void slotBookMarkCurrentFilter();
    void setFilterIndex(const QModelIndex&);
    void editConfigurations();
    void update();
    void showXMLdialog();
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
        QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(this);
        if (scroller) {
            connect(scroller, SIGNAL(stateChanged(QScroller::State)),
                    this, SLOT(slotScrollerStateChanged(QScroller::State)));
        }

        connect(this, SIGNAL(expanded(QModelIndex)), this, SLOT(update_scroll_area(QModelIndex)));
        connect(this, SIGNAL(collapsed(QModelIndex)), this, SLOT(update_scroll_area(QModelIndex)));
    }

    void setFilterModel(QAbstractItemModel * model);
    void activateFilter(QModelIndex idx);

    QSize minimumSizeHint() const override
    {
        return QSize(200, QTreeView::sizeHint().height());
    }

    QSize sizeHint() const override
    {
        return QSize(header()->width(), QTreeView::sizeHint().height());
    }

    void setModel(QAbstractItemModel *model) override
    {
        QTreeView::setModel(model);
        if (header()->visualIndex(0) != -1) {
            header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        }
    }

protected:

    void resizeEvent(QResizeEvent *event) override
    {
        if (event->size().width() > 10) {
            setModel(m_model);

        }
        else {
            setModel(0);
        }
        QTreeView::resizeEvent(event);
    }

    void showEvent(QShowEvent * event) override
    {
        setModel(m_model);
        QTreeView::showEvent(event);
    }

    void hideEvent(QHideEvent * event) override
    {
        setModel(0);
        QTreeView::hideEvent(event);
    }

private Q_SLOTS:
    void update_scroll_area(const QModelIndex& i)
    {
        resizeColumnToContents(i.column());
    }

public Q_SLOTS:
    void slotScrollerStateChanged(QScroller::State state){ KisKineticScroller::updateCursor(this, state); }

private:

    QAbstractItemModel *m_model;

};

#endif
