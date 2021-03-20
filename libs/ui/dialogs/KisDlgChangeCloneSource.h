/*
 *  SPDX-FileCopyrightText: 2019 Tusooa Zhu <tusooa@vista.aero>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_DLG_CHANGE_CLONE_SOURCE_H_
#define KIS_DLG_CHANGE_CLONE_SOURCE_H_

#include "kis_types.h"
#include <KoDialog.h>

#include "ui_wdgchangeclonesource.h"

class QWidget;
class KisViewManager;

class KisDlgChangeCloneSource : public KoDialog
{
    Q_OBJECT

public:
    KisDlgChangeCloneSource(QList<KisCloneLayerSP> layers, KisViewManager *view, QWidget *parent = 0);

    ~KisDlgChangeCloneSource() override;

private:
    void updateTargetLayerList();

public Q_SLOTS:
    void slotCancelChangesAndSetNewTarget();

private:
    struct Private;
    const QScopedPointer<Private> d;
};

#endif // KIS_DLG_CHANGE_CLONE_SOURCE_H_
