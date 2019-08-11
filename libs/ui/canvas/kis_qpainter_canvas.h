/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
 * Copyright (C) Lukáš Tvrdý <lukast.dev@gmail.com>, (C) 2009
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
#ifndef KIS_QPAINTER_CANVAS_H
#define KIS_QPAINTER_CANVAS_H

#include <QWidget>

#include "kis_canvas_widget_base.h"
#include "kis_ui_types.h"

class QPaintEvent;
class KisCanvas2;
class KisDisplayColorConverter;

/**
 *
 * KisQPainterCanvas is the widget that shows the actual image using arthur.
 *
 * NOTE: if you change something in the event handling here, also change it
 * in the opengl canvas.
 *
 * @author Boudewijn Rempt <boud@valdyas.org>
*/
class KisQPainterCanvas : public QWidget, public KisCanvasWidgetBase
{

    Q_OBJECT

public:

    KisQPainterCanvas(KisCanvas2 * canvas, KisCoordinatesConverter *coordinatesConverter, QWidget * parent);

    ~KisQPainterCanvas() override;

    void setPrescaledProjection(KisPrescaledProjectionSP prescaledProjection);

public: // QWidget overrides

    void paintEvent(QPaintEvent * ev) override;

    void resizeEvent(QResizeEvent *e) override;

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

    void inputMethodEvent(QInputMethodEvent *event) override;

public: // Implement kis_abstract_canvas_widget interface
    void setDisplayFilter(QSharedPointer<KisDisplayFilter> displayFilter) override;
    void notifyImageColorSpaceChanged(const KoColorSpace *cs) override;
    void setWrapAroundViewingMode(bool value) override;
    void channelSelectionChanged(const QBitArray &channelFlags) override;
    void setDisplayColorConverter(KisDisplayColorConverter *colorConverter) override;
    void finishResizingImage(qint32 w, qint32 h) override;
    KisUpdateInfoSP startUpdateCanvasProjection(const QRect & rc, const QBitArray &channelFlags) override;
    QRect updateCanvasProjection(KisUpdateInfoSP info) override;
    using KisCanvasWidgetBase::updateCanvasProjection;

    QWidget * widget() override {
        return this;
    }

    bool isBusy() const override {
        return false;
    }

    void setLodResetInProgress(bool value) override {
        Q_UNUSED(value);
    }

protected: // KisCanvasWidgetBase

    bool callFocusNextPrevChild(bool next) override;

protected:
    virtual void drawImage(QPainter & gc, const QRect &updateWidgetRect) const;

private Q_SLOTS:
    void slotConfigChanged();

private:
    class Private;
    Private * const m_d;
};

#endif
