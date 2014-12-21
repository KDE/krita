/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_TRANSFORM_STRATEGY_BASE_H
#define __KIS_TRANSFORM_STRATEGY_BASE_H

#include <QObject>
#include <QScopedPointer>

#include "kis_tool.h"


class QImage;
class QPointF;
class QTransform;
class QPainter;
class QCursor;
class KoPointerEvent;
class QPainterPath;


class KisTransformStrategyBase : public QObject
{
public:
    KisTransformStrategyBase();
    ~KisTransformStrategyBase();

    QImage originalImage() const;
    QTransform thumbToImageTransform() const;

    void setThumbnailImage(const QImage &image, QTransform thumbToImageTransform);

public:

    virtual bool acceptsClicks() const;

    virtual void paint(QPainter &gc) = 0;
    virtual QCursor getCurrentCursor() const = 0;
    virtual QPainterPath getCursorOutline() const;

    virtual void externalConfigChanged() = 0;

    virtual bool beginPrimaryAction(KoPointerEvent *event) = 0;
    virtual void continuePrimaryAction(KoPointerEvent *event) = 0;
    virtual bool endPrimaryAction(KoPointerEvent *event) = 0;
    virtual void hoverActionCommon(KoPointerEvent *event) = 0;

    virtual void activateAlternateAction(KisTool::AlternateAction action);
    virtual void deactivateAlternateAction(KisTool::AlternateAction action);

    virtual bool beginAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action);
    virtual void continueAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action);
    virtual bool endAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_TRANSFORM_STRATEGY_BASE_H */
