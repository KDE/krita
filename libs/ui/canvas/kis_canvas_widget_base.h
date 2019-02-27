/*
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>, (C)
 * Copyright (C) 2010 Adrian Page <adrian@pagenet.plus.com>
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
