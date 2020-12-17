/*
 * SPDX-FileCopyrightText: 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#ifndef KISMIRRORAXIS_H
#define KISMIRRORAXIS_H

#include <QScopedPointer>

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
    const QScopedPointer<Private> d;

private Q_SLOTS:
    void mirrorModeChanged();
    void moveHorizontalAxisToCenter();
    void moveVerticalAxisToCenter();
};

#endif // KISMIRRORAXIS_H
