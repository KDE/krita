/*
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
*  Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#ifndef _KIS_DLG_FILTER_H_
#define _KIS_DLG_FILTER_H_

#include <QDialog>

#include <kis_types.h>

class KisViewManager;
class KisFilterManager;

class KisDlgFilter : public QDialog
{

    Q_OBJECT

public:

    KisDlgFilter(KisViewManager *view, KisNodeSP node, KisFilterManager *filterManager, QWidget *parent = 0);

    ~KisDlgFilter() override;

    void setFilter(KisFilterSP f);

protected Q_SLOTS:

    void slotOnAccept();
    void slotOnReject();

    void createMask();

    void enablePreviewToggled(bool state);

    void filterSelectionChanged();

    void resizeEvent(QResizeEvent* ) override;

public Q_SLOTS:
    void adjustSize();

private:
    void startApplyingFilter(KisFilterConfigurationSP config);
    void setDialogTitle(KisFilterSP f);
    void updatePreview();

private Q_SLOTS:
    void slotFilterWidgetSizeChanged();

private:
    struct Private;
    KisDlgFilter::Private* const d;
};

#endif
