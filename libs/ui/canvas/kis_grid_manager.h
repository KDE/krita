/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_GRID_MANAGER_H
#define KIS_GRID_MANAGER_H

#include <QPainter>

#include "kis_types.h"
#include <kritaui_export.h>
#include "kis_action_manager.h"
#include "kis_action.h"

class KisGridDecoration;
class KisViewManager;
class KisGridConfig;


class KRITAUI_EXPORT KisGridManager : public QObject
{
    Q_OBJECT
public:
    KisGridManager(KisViewManager * parent);
    ~KisGridManager() override;
public:

    void setup(KisActionManager * actionManager);
    void setView(QPointer<KisView>imageView);

    void setGridConfig(const KisGridConfig &config);

Q_SIGNALS:
    void sigRequestUpdateGridConfig(const KisGridConfig &config);

public Q_SLOTS:

    void updateGUI();

private Q_SLOTS:

    void slotChangeGridVisibilityTriggered(bool value);
    void slotSnapToGridTriggered(bool value);

private:
    void setGridConfigImpl(const KisGridConfig &config, bool emitModified);

private:
    void setFastConfig(int size);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif
