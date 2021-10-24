/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_COMMON_COLORS_H
#define KIS_COMMON_COLORS_H

#include <QToolButton>

#include <QMutex>
#include <QTimer>
#include "kis_color_patches.h"
#include <kis_types.h>

class KisCommonColors : public KisColorPatches
{
Q_OBJECT
public:
    explicit KisCommonColors(QWidget *parent = 0);
    void setCanvas(KisCanvas2 *canvas) override;
    void unsetCanvas() override {}
    KisColorSelectorBase* createPopup() const override;

public Q_SLOTS:
    void setColors(QList<KoColor> colors);
    void updateSettings() override;
    void recalculate();

private:
    QMutex m_mutex;
    QTimer m_recalculationTimer;
    QToolButton* m_reloadButton;
    QList<KoColor> m_calculatedColors;
    KisImageWSP m_image;
};

#endif
