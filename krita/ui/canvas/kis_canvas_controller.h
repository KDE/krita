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

#include "krita_export.h"
#include "kis_types.h"

class KisView;

class KRITAUI_EXPORT KisCanvasController : public KoCanvasControllerWidget
{
    Q_OBJECT

public:
    KisCanvasController(QPointer<KisView>parent, KActionCollection * actionCollection);
    ~KisCanvasController();

    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void changeCanvasWidget(QWidget *widget);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual bool eventFilter(QObject *watched, QEvent *event);
    virtual void updateDocumentSize(const QSize &sz, bool recalculateCenter);
    virtual void activate();

public:
    using KoCanvasController::documentSize;
    bool wrapAroundMode() const;
    bool levelOfDetailMode() const;

public Q_SLOTS:
    void mirrorCanvas(bool enable);
    void rotateCanvas(qreal angle);
    void rotateCanvasRight15();
    void rotateCanvasLeft15();
    void resetCanvasRotation();
    void slotToggleWrapAroundMode(bool value);
    void slotToggleLevelOfDetailMode(bool value);

Q_SIGNALS:
    void documentSizeChanged();

private:
    struct Private;
    Private * const m_d;
};

#endif /* KIS_CANVAS_CONTROLLER_H */
