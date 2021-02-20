/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LIQUIFY_PAINT_HELPER_H
#define __KIS_LIQUIFY_PAINT_HELPER_H

#include <QScopedPointer>

class KisLiquifyTransformWorker;
class KisCoordinatesConverter;
class KoPointerEvent;
class KisLiquifyProperties;
class QPainterPath;
class KoCanvasResourceProvider;

class KisLiquifyPaintHelper
{
public:
    KisLiquifyPaintHelper(const KisCoordinatesConverter *converter);
    ~KisLiquifyPaintHelper();

    void configurePaintOp(const KisLiquifyProperties &_props,
                          KisLiquifyTransformWorker *worker);

    void startPaint(KoPointerEvent *event, const KoCanvasResourceProvider *manager);
    void continuePaint(KoPointerEvent *event);
    bool endPaint(KoPointerEvent *event);

    void hoverPaint(KoPointerEvent *event);

    QPainterPath brushOutline(const KisLiquifyProperties &props);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_LIQUIFY_PAINT_HELPER_H */
