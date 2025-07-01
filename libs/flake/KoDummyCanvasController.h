/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006, 2008 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2007-2010 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2007-2008 C. Boemann <cbo@boemann.dk>
 * SPDX-FileCopyrightText: 2006-2007 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2009 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KODUMMYCANVASCONTROLLER_H
#define KODUMMYCANVASCONTROLLER_H

#include "KoCanvasController.h"

/**
 * A dummy implementation of KoCanvasController that provides empty or default
 * implementations for all virtual methods. This can be used as a base class
 * or for testing purposes.
 */
class KRITAFLAKE_EXPORT KoDummyCanvasController : public KoCanvasController {

public:
    explicit KoDummyCanvasController(KisKActionCollection* actionCollection)
        : KoCanvasController(actionCollection)
    {}

    ~KoDummyCanvasController() override = default;

    void setCanvas(KoCanvasBase *canvas) override {Q_UNUSED(canvas)}
    KoCanvasBase *canvas() const override {return 0;}
    void ensureVisibleDoc(const QRectF &, bool ) override {}
    void zoomIn(const QPoint &/*center*/) override {}
    void zoomIn() override {}
    void zoomOut(const QPoint &/*center*/) override {}
    void zoomOut() override {}
    void zoomTo(const QRect &/*rect*/) override {}
    void setZoom(KoZoomMode::Mode /*mode*/, qreal /*zoom*/) override {}
    void setPreferredCenter(const QPointF &/*viewPoint*/) override {}
    QPointF preferredCenter() const override {return QPointF();}
    void pan(const QPoint &/*distance*/) override {}
    void panUp() override {}
    void panDown() override {}
    void panLeft() override {}
    void panRight() override {}
    QPoint scrollBarValue() const override {return QPoint();}
    void setScrollBarValue(const QPoint &/*value*/) override {}
    void resetScrollBars() override {}
    QPointF currentCursorPosition() const override { return QPointF(); }
    KoZoomState zoomState() const override { return {}; };
};

#endif // KODUMMYCANVASCONTROLLER_H