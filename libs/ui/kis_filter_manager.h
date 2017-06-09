/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_FILTER_MANAGER_
#define _KIS_FILTER_MANAGER_

#include <QObject>
#include <kritaui_export.h>
#include <kis_types.h>

class KisViewManager;
class KActionCollection;
class KisActionManager;
class KisView;

/**
 * Create all the filter actions for the specified view and implement re-apply filter
 */
class KRITAUI_EXPORT KisFilterManager : public QObject
{

    Q_OBJECT

public:

    KisFilterManager(KisViewManager * parent);
    ~KisFilterManager() override;
    void setView(QPointer<KisView>imageView);

    void setup(KActionCollection * ac, KisActionManager *actionManager);
    void updateGUI();

    void apply(KisFilterConfigurationSP filterConfig);
    void finish();
    void cancel();
    bool isStrokeRunning() const;

private Q_SLOTS:

    void insertFilter(const QString &name);
    void showFilterDialog(const QString &filterId);
    void reapplyLastFilter();

    void slotStrokeEndRequested();
    void slotStrokeCancelRequested();

private:
    struct Private;
    Private * const d;
};

#endif
