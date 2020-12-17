/*
 * SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>, (C)
 * SPDX-FileCopyrightText: 2010 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_CANVAS_WIDGET_BASE_
#define _KIS_CANVAS_WIDGET_BASE_

#include <QList>
#include <Qt>

#include "kis_abstract_canvas_widget.h"

class QColor;
class QImage;
class QInputMethodEvent;
class QVariant;

class KisCoordinatesConverter;
class KisDisplayFilter;
class KisCanvas2;

#include "kritaui_export.h"

class KRITAUI_EXPORT KisCanvasWidgetBase : public KisAbstractCanvasWidget
{
public:
    KisCanvasWidgetBase(KisCanvas2 * canvas, KisCoordinatesConverter *coordinatesConverter);

    ~KisCanvasWidgetBase() override;

public: // KisAbstractCanvasWidget

    KoToolProxy *toolProxy() const override;

    /**
     * Draw the specified decorations on the view.
     */
    void drawDecorations(QPainter & gc, const QRect &updateWidgetRect) const override;

    void addDecoration(KisCanvasDecorationSP deco) override;
    void removeDecoration(const QString& id) override;
    KisCanvasDecorationSP decoration(const QString& id) const override;

    void setDecorations(const QList<KisCanvasDecorationSP > &) override;
    QList<KisCanvasDecorationSP > decorations() const override;

    void setWrapAroundViewingMode(bool value) override;

    /**
     * Returns the color of the border, i.e. the part of the canvas
     * outside the image contents.
     *
     */
    QColor borderColor() const;

    /**
     * Returns one check of the background checkerboard pattern.
     */
    static QImage createCheckersImage(qint32 checkSize = -1);


    KisCoordinatesConverter* coordinatesConverter() const;

    QVector<QRect> updateCanvasProjection(const QVector<KisUpdateInfoSP> &infoObjects) override;
    using KisAbstractCanvasWidget::updateCanvasProjection;

protected:
    KisCanvas2 *canvas() const;

    /**
     * Event handlers to be called by derived canvas event handlers.
     * All common event processing is carried out by these
     * functions.
     */
    QVariant processInputMethodQuery(Qt::InputMethodQuery query) const;
    void processInputMethodEvent(QInputMethodEvent *event);
    void notifyConfigChanged();

    /// To be implemented by the derived canvas
    virtual bool callFocusNextPrevChild(bool next) = 0;

private:
    struct Private;
    Private * const m_d;

};

#endif // _KIS_CANVAS_WIDGET_BASE_
