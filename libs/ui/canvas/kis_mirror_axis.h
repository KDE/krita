/*
 * Copyright (c) 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef KISMIRRORAXIS_H
#define KISMIRRORAXIS_H

#include "kis_canvas_decoration.h"

class KisView;
class KisCanvasResourceProvider;
class KisMirrorAxisConfig;

class KisMirrorAxis : public KisCanvasDecoration
{
    Q_OBJECT
    Q_PROPERTY(float handleSize READ handleSize WRITE setHandleSize NOTIFY handleSizeChanged)

public:
    KisMirrorAxis(KisCanvasResourceProvider* provider, QPointer<KisView> parent);
    ~KisMirrorAxis() override;

    float handleSize() const;
    void setHandleSize(float newSize);
    void setVisible(bool v) override;

    void setMirrorAxisConfig(const KisMirrorAxisConfig& config);
    const KisMirrorAxisConfig& mirrorAxisConfig() const;

Q_SIGNALS:
    void handleSizeChanged();
    void sigConfigChanged();

protected:
    void drawDecoration(QPainter& gc, const QRectF& updateArea, const KisCoordinatesConverter* converter, KisCanvas2* canvas) override;
    bool eventFilter(QObject* target, QEvent* event) override;
    void toggleMirrorActions();

private:
    class Private;
    Private * const d;

private Q_SLOTS:
    void mirrorModeChanged();
    void moveHorizontalAxisToCenter();
    void moveVerticalAxisToCenter();
};

#endif // KISMIRRORAXIS_H
