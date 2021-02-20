/*
 * This file is part of Krita
 *
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRID_DECORATION_H
#define KIS_GRID_DECORATION_H

#include <QScopedPointer>
#include <kis_canvas_decoration.h>

class KisGridConfig;


class KisGridDecoration : public KisCanvasDecoration
{
    Q_OBJECT
public:
    KisGridDecoration(KisView* parent);
    ~KisGridDecoration() override;

    void setGridConfig(const KisGridConfig &config);

protected:
    void drawDecoration(QPainter& gc, const QRectF& updateArea, const KisCoordinatesConverter* converter, KisCanvas2* canvas) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_GRID_DECORATION_H
