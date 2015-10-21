/* This file is part of the KDE project
 * Copyright 2014  Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "CurveEditorItem.h"

#include "kis_curve_widget.h"
#include <QPainter>
#include <QMouseEvent>
#include <kis_debug.h>

class CurveEditorItem::Private
{
public:
    Private(CurveEditorItem* qq)
        : q(qq)
    {
        curveWidget = new KisCurveWidget();
    }
    ~Private()
    {
        delete curveWidget;
    }
    void repaint();

    CurveEditorItem* q;
    KisCurveWidget* curveWidget;
    QImage contents;

};

CurveEditorItem::CurveEditorItem(QQuickItem* parent)
    : QQuickPaintedItem(parent)
    , d(new Private(this))
{
    setFlag(QQuickItem::ItemHasContents, true);
    setAcceptedMouseButtons( Qt::LeftButton | Qt::RightButton | Qt::MidButton );
    connect(d->curveWidget, SIGNAL(pointSelectedChanged()), SIGNAL(pointSelectedChanged()));
    connect(d->curveWidget, SIGNAL(modified()), SIGNAL(curveChanged()));
    qRegisterMetaType<KisCubicCurve>();
}

CurveEditorItem::~CurveEditorItem()
{
    delete d;
}

void CurveEditorItem::paint(QPainter* p)
{
    p->drawImage(boundingRect(), d->contents);
}

KisCubicCurve CurveEditorItem::curve() const
{
    return d->curveWidget->curve();
}

void CurveEditorItem::setCurve(KisCubicCurve curve)
{
    d->curveWidget->setCurve(curve);
    emit curveChanged();
}

bool CurveEditorItem::pointSelected() const
{
    return d->curveWidget->pointSelected();
}

void CurveEditorItem::deleteSelectedPoint()
{
    if(pointSelected()) {
        QKeyEvent* event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
        d->curveWidget->keyPressEvent(event);
        d->repaint();
    }
}

void CurveEditorItem::Private::repaint()
{
    curveWidget->resize(q->boundingRect().size().toSize());
    contents = QImage(q->boundingRect().size().toSize(), QImage::Format_ARGB32_Premultiplied);
    contents.fill(Qt::blue);
    curveWidget->render(&contents);
    q->update();
}

void CurveEditorItem::geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    d->repaint();
    QQuickPaintedItem::geometryChanged(newGeometry, oldGeometry);
}

void CurveEditorItem::mousePressEvent(QMouseEvent* event)
{
    QMouseEvent* mouseEvent = new QMouseEvent(event->type(), event->pos(), event->button(), event->buttons(), event->modifiers());
    d->curveWidget->mousePressEvent(mouseEvent);
    if(mouseEvent->isAccepted()) {
        event->accept();
    }
    d->repaint();
}

void CurveEditorItem::mouseMoveEvent(QMouseEvent* event)
{
    QMouseEvent* mouseEvent = new QMouseEvent(event->type(), event->pos(), event->button(), event->buttons(), event->modifiers());
    d->curveWidget->mouseMoveEvent(mouseEvent);
    if(mouseEvent->isAccepted()) {
        event->accept();
    }
    d->repaint();
}

void CurveEditorItem::mouseReleaseEvent(QMouseEvent* event)
{
    QMouseEvent* mouseEvent = new QMouseEvent(event->type(), event->pos(), event->button(), event->buttons(), event->modifiers());
    d->curveWidget->mouseReleaseEvent(mouseEvent);
    if(mouseEvent->isAccepted()) {
        event->accept();
    }
    d->repaint();
}

