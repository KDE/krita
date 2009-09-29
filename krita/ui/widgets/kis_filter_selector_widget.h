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

#include <kis_types.h>

class QModelIndex;
class KisFilterConfiguration;

/**
 *
 */
class KisFilterSelectorWidget : public QWidget
{
    Q_OBJECT
public:
    KisFilterSelectorWidget(QWidget* parent);
    ~KisFilterSelectorWidget();
    void setFilter(KisFilterSP f);
    void setPaintDevice(KisPaintDeviceSP);
    void setImage(KisImageWSP);
    KisFilterConfiguration* configuration();
    void showSelector(bool visible);
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

#endif
