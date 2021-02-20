/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
