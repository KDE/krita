/*
 *  Copyright (c) 2019 Tusooa Zhu <tusooa@vista.aero>
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
