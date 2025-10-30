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
    ~KisAsyncColorSamplerHelper() override;

    bool isActive() const;

    void activate(bool sampleCurrentLayer, bool pickFgColor);
    void deactivate();

    void startAction(const QPointF &docPoint, int radius, int blend);
    void continueAction(const QPointF &docPoint);
    void endAction();

    QRectF colorPreviewDocRect(const QPointF &docPoint);
    void paint(QPainter &gc, const KoViewConverter &converter);

    void updateCursor(bool sampleCurrentLayer, bool pickFgColor);

    void setUpdateGlobalColor(bool value);
    bool updateGlobalColor() const;

Q_SIGNALS:
    void sigRequestUpdateOutline();
    void sigRequestCursor(const QCursor &cursor);
    void sigRequestCursorReset();
    /**
     * Notifies about the raw color picked from the layer,
     * including its alpha channel.
     */
    void sigRawColorSelected(const KoColor &color);

    /**
     * Notifies about the "palette" color picked from the layer,
     * that is, with the alpha channel set to OPACITY_OPAQUE.
     */
    void sigColorSelected(const KoColor &color);

    /**
     * Notifies about the "palette" color picked from the layer,
     * that is, with the alpha channel set to OPACITY_OPAQUE.
     *
     * This notification is emitted only once at the very end
     * of the color picking stroke.
     */
    void sigFinalColorSelected(const KoColor &color);

private Q_SLOTS:
    void activateDelayedPreview();
    void slotAddSamplingJob(const QPointF &docPoint);
    void slotColorSamplingFinished(const KoColor &rawColor);

private:
    void activatePreview();
    void paintRectangle(QPainter &gc, const QRectF &viewRectF, const QColor &currentColor, const QColor &baseColor);
    void paintCircle(QPainter &gc, const QRectF &viewRectF, const QColor &currentColor, const QColor &baseColor);

    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISASYNCCOLORSAMPLERHELPER_H
