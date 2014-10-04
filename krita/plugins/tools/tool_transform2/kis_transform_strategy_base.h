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

class QImage;
class QPointF;
class QTransform;
class QPainter;
class QCursor;
class KoPointerEvent;
class KisCoordinatesConverter;


class KisTransformStrategyBase : public QObject
{
public:
    KisTransformStrategyBase(const KisCoordinatesConverter *_converter);
    ~KisTransformStrategyBase();

    QImage originalImage() const;
    QTransform thumbToImageTransform() const;

    void setThumbnailImage(const QImage &image, QTransform thumbToImageTransform);

public:

    virtual bool acceptsClicks() const;

    virtual void setTransformFunction(const QPointF &mousePos, bool perspectiveModifierActive) = 0;
    virtual void paint(QPainter &gc) = 0;
    virtual QCursor getCurrentCursor() const = 0;

    virtual void externalConfigChanged() = 0;

    virtual bool beginPrimaryAction(KoPointerEvent *event);
    virtual void continuePrimaryAction(KoPointerEvent *event, bool specialModifierActve);
    virtual bool endPrimaryAction(KoPointerEvent *event);

protected:

    virtual bool beginPrimaryAction(const QPointF &pt);
    virtual void continuePrimaryAction(const QPointF &pt, bool specialModifierActve);
    virtual bool endPrimaryAction();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_TRANSFORM_STRATEGY_BASE_H */
