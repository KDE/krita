/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISASYNCCOLORSAMPLERHELPER_H
#define KISASYNCCOLORSAMPLERHELPER_H

#include "kritaui_export.h"

#include <QScopedPointer>
#include <QObject>

#include "kis_types.h"

class QPainter;
class KoViewConverter;
class KisStrokesFacade;
class KisCanvas2;
class KoColor;

class KRITAUI_EXPORT KisAsyncColorSamplerHelper : public QObject
{
    Q_OBJECT
public:
    KisAsyncColorSamplerHelper(KisCanvas2 *canvas);
    ~KisAsyncColorSamplerHelper();

    bool isActive() const;

    void activate(bool sampleCurrentLayer, bool pickFgColor);
    void deactivate();

    void startAction(const QPointF &docPoint);
    void continueAction(const QPointF &docPoint);
    void endAction();

    QRectF colorPreviewDocRect(const QPointF &docPoint);
    void paint(QPainter &gc, const KoViewConverter &converter);

Q_SIGNALS:
    void sigRequestUpdateOutline();
    void sigRequestCursor(const QCursor &cursor);
    void sigRequestCursorReset();

private Q_SLOTS:
    void activateDelayedPreview();
    void slotAddSamplingJob(const QPointF &docPoint);
    void slotColorSamplingFinished(const KoColor &color);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISASYNCCOLORSAMPLERHELPER_H
