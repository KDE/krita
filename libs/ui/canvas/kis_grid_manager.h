/*
 * This file is part of Krita
 *
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
