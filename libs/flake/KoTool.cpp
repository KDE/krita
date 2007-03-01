/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoTool.h"
#include "KoCanvasBase.h"
#include "KoPointerEvent.h"

#include <kactioncollection.h>
#include <QWidget>

class KoToolPrivate {
public:
    KoToolPrivate()
        : optionWidget(0),
        previousCursor(Qt::ArrowCursor) { }

    QWidget *optionWidget; ///< the optionwidget this tool will show in the option widget palette
    QCursor previousCursor;
    QHash<QString, QAction*> actionCollection;
    QString toolId;
};

KoTool::KoTool(KoCanvasBase *canvas )
    : m_canvas(canvas),
    d(new KoToolPrivate())
{
    if(m_canvas) { // in the case of KoToolManagers dummytool it can be zero :(
        KoCanvasResourceProvider * crp = m_canvas->resourceProvider();
        Q_ASSERT_X(crp, "KoTool::KoTool", "No KoCanvasResourceProvider");
        if (crp)
            connect( m_canvas->resourceProvider(),
                 SIGNAL( sigResourceChanged(int, const QVariant & ) ),
                 this,
                 SLOT( resourceChanged( int, const QVariant &  ) ) );
    }
}

KoTool::~KoTool()
{
    if (d->optionWidget && !d->optionWidget->parentWidget())
        delete d->optionWidget;
}

void KoTool::activate(bool temporary) {
    Q_UNUSED(temporary);
}

void KoTool::deactivate() {
}

void KoTool::resourceChanged( int key, const QVariant & res )
{
    Q_UNUSED( key );
    Q_UNUSED( res );
}

bool KoTool::wantsAutoScroll() {
    return true;
}

void KoTool::mouseDoubleClickEvent( KoPointerEvent *event ) {
    event->ignore();
}

void KoTool::keyPressEvent(QKeyEvent *e) {
    e->ignore();
}

void KoTool::keyReleaseEvent(QKeyEvent *e) {
    e->ignore();
}

void KoTool::wheelEvent( KoPointerEvent * e ) {
    e->ignore();
}


void KoTool::useCursor(QCursor cursor, bool force) {
    if(!force && cursor.shape() == d->previousCursor.shape())
        return;
    d->previousCursor = cursor;
    emit sigCursorChanged(d->previousCursor);
}

QWidget * KoTool::optionWidget() {
    if (d->optionWidget == 0) {
        d->optionWidget = createOptionWidget();
    }
    return d->optionWidget;
}

void KoTool::addAction(const QString &name, QAction *action) {
    d->actionCollection.insert(name, action);
}

QHash<QString, QAction*> KoTool::actions() const {
    return d->actionCollection;
}

QAction *KoTool::action(const QString &name) const {
    return d->actionCollection[name];
}

QWidget * KoTool::createOptionWidget()
{
    return 0;
}

void KoTool::setToolId(const QString &id) {
    d->toolId = id;
}

QString KoTool::toolId() const {
    return d->toolId;
}

QCursor KoTool::cursor() const {
    return d->previousCursor;
}

#include "KoTool.moc"
