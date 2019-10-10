/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_COMMON_COLORS_H
#define KIS_COMMON_COLORS_H

#include <QMutex>
#include <QTimer>
#include "kis_color_patches.h"
#include <kis_types.h>

class QPushButton;

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
    QPushButton* m_reloadButton;
    QList<KoColor> m_calculatedColors;
    KisImageWSP m_image;
};

#endif
