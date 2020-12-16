/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TRANSFORM_STRATEGY_BASE_H
#define __KIS_TRANSFORM_STRATEGY_BASE_H

#include <QObject>
#include <QScopedPointer>

#include "kis_tool.h"


class QImage;
class QTransform;
class QPainter;
class QCursor;
class KoPointerEvent;
class QPainterPath;


class KisTransformStrategyBase : public QObject
{
public:
    KisTransformStrategyBase();
    ~KisTransformStrategyBase() override;

    QImage originalImage() const;
    QTransform thumbToImageTransform() const;

    void setThumbnailImage(const QImage &image, QTransform thumbToImageTransform);

public:

    virtual bool acceptsClicks() const;

    virtual void paint(QPainter &gc) = 0;
    virtual QCursor getCurrentCursor() const = 0;
    virtual QPainterPath getCursorOutline() const;

    virtual void externalConfigChanged() = 0;

    virtual void activatePrimaryAction();
    virtual void deactivatePrimaryAction();

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
