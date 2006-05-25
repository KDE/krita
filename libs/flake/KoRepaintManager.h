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
#ifndef KOREPAINTMANAGER_H
#define KOREPAINTMANAGER_H

#include <QList>

class KoCanvasBase;
class KoSelection;
class KoShape;
class QRectF;

/**
 * This class gets repaint requests and will sent it to all registered canvas objects.
 * Each KoShape has a link to a repaint manager so any repaint requests on the shape
 * will be forwarded here.
 * There will be one repaintManager per canvas so in order to repaint the shape that may
 * be visible on more then one canvas there is a concept of chained repaintManagers.
 * The KoRepaintManager is refcounted and deletes itself.
 */
class KoRepaintManager {
public:
    /**
     * Constructor.
     * @param canvas the canvas this repaintManager is going to forward any requests to.
     * @param selection the selection object that manages the selections for the canvas
     */
    KoRepaintManager(KoCanvasBase *canvas, KoSelection *selection);

    /**
     * attach another repaint manager to this one so repaints from our children will
     * have effect on the other managers canvas as well.
     * @param manager the other manager.
     */
    void addChainedManager(KoRepaintManager *manager);

    /**
     * Since this object is refcounted you do not delete it but you dismantle it.
     */
    void dismantle(); // to deactivate this repaintManager for good.

    /**
     * For reference counting purposes, add a user to register that user has a reference.
     */
    void addUser() { m_refCount++; }
    /**
     * For reference counting purposes, remove a user to register that user no
     * longer has a reference.
     */
    void removeUser() { m_refCount--; }
    /**
     * return the amount of references.
     * @return the amount of references.
     */
    int refCount() { return m_refCount; }

    /**
     * Request a repaint to be queued.
     * The repaint will be restricted to the parameters rectangle, which is expected to be
     * in points (the internal coordinates system of KoShape) and it is expected to be
     * normalized and based in the global coordinates, not any local coordinates.
     * <p>This method will return immediately and only request a repaint. Successive calls
     * will be merged into an appropriate repaint action.
     * @param rect the rectangle (in pt) to queue for repaint.
     * @param shape the shape that is going to be redrawn; only needed when selectionHandles=true
     * @param selectionHandles if true; find out if the shape is selected and repaint its
     *   selection handles at the same time.
     */
    void repaint(QRectF &rect, const KoShape *shape = 0, bool selectionHandles=false);

    /**
     * Return if this manager is still actively supporting clients for a canvas.
     */
    bool isActive() { return m_active; }

private:
    KoCanvasBase *m_canvas;
    KoSelection *m_selection;
    bool m_active, m_locked;
    QList<KoRepaintManager*> m_otherManagers;
    int m_refCount;
};
#endif
