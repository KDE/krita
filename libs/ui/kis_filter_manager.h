/*
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    //! Cancel current running stroke
    void cancelRunningStroke();
    //! Call `cancelRunningStroke()` and then delete dialog `d`
    void cancelDialog();
    bool isStrokeRunning() const;

    bool isIdle() const;

    void setFilterAllSelectedFrames(bool filterAllSelectedFrames);
    bool filterAllSelectedFrames();

private Q_SLOTS:

    void insertFilter(const QString &name);
    void showFilterDialog(const QString &filterId, KisFilterConfigurationSP overrideDefaultConfig = nullptr);
    void reapplyLastFilter();
    void reapplyLastFilterReprompt();

    void slotStrokeEndRequested();
    void slotStrokeCancelRequested();

    //! Clean up after filter dialog has been accepted / rejected / closed
    void filterDialogHasFinished(int);

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif
