/*
*  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
*
*  SPDX-License-Identifier: GPL-2.0-or-later
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

    void setFilter(KisFilterSP f, KisFilterConfigurationSP overrideDefaultConfig);

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


private Q_SLOTS:
    void slotFilterWidgetSizeChanged();
    void updatePreview();

private:
    struct Private;
    KisDlgFilter::Private* const d;
};

#endif
