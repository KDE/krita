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

#ifndef KIS_MINIMAL_SHADE_SELECTOR_H
#define KIS_MINIMAL_SHADE_SELECTOR_H

#include <QPointer>
#include <kis_canvas2.h>

#include "kis_color_selector_base.h"

class KisShadeSelectorLine;
class KisCanvas2;
class KisColorSelectorBaseProxy;

class KisMinimalShadeSelector : public KisColorSelectorBase
{
Q_OBJECT
public:
    explicit KisMinimalShadeSelector(QWidget *parent = 0);
    ~KisMinimalShadeSelector() override;
    void unsetCanvas() override;
    void setCanvas(KisCanvas2* canvas) override;

protected:
    void setColor(const KoColor& color) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;

public Q_SLOTS:
    void updateSettings() override;

protected Q_SLOTS:
    void canvasResourceChanged(int key, const QVariant& v) override;

protected:
    void paintEvent(QPaintEvent *) override;
    KisColorSelectorBase* createPopup() const override;

private:
    QList<KisShadeSelectorLine*> m_shadingLines;
    KoColor m_lastRealColor;
    QPointer<KisCanvas2> m_canvas;

    QScopedPointer<KisColorSelectorBaseProxy> m_proxy;
};

#endif
