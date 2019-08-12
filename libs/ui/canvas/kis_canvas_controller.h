/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KIS_CANVAS_CONTROLLER_H
#define KIS_CANVAS_CONTROLLER_H

#include <KoCanvasControllerWidget.h>
#include <libs/flake/KoCanvasSupervisor.h>

#include "kritaui_export.h"
#include "kis_types.h"

class KConfigGroup;
class KisView;

class KRITAUI_EXPORT KisCanvasController : public KoCanvasControllerWidget
{
    Q_OBJECT

public:
    KisCanvasController(QPointer<KisView>parent, KoCanvasSupervisor *observerProvider, KActionCollection * actionCollection);
    ~KisCanvasController() override;

    void setCanvas(KoCanvasBase *canvas) override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void updateDocumentSize(const QSizeF &sz, bool recalculateCenter) override;
    void activate() override;

    QPointF currentCursorPosition() const override;

public:
    using KoCanvasController::documentSize;
    bool wrapAroundMode() const;
    bool levelOfDetailMode() const;

    void saveCanvasState(KisPropertiesConfiguration &config) const;
    void restoreCanvasState(const KisPropertiesConfiguration &config);

    void resetScrollBars() override;

public Q_SLOTS:
    void mirrorCanvas(bool enable);
    void rotateCanvas(qreal angle, const QPointF &center);
    void rotateCanvas(qreal angle);
    void rotateCanvasRight15();
    void rotateCanvasLeft15();
    qreal rotation() const;
    void resetCanvasRotation();
    void slotToggleWrapAroundMode(bool value);
    void slotTogglePixelGrid(bool value);
    void slotToggleLevelOfDetailMode(bool value);

Q_SIGNALS:
    void documentSizeChanged();

private:
    struct Private;
    Private * const m_d;
};

#endif /* KIS_CANVAS_CONTROLLER_H */
