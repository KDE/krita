/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisSelectedShapesProxy.h"

#include "kis_signal_auto_connection.h"
#include <KoShapeManager.h>
#include <KoSelection.h>


struct KisSelectedShapesProxy::Private
{
    KoShapeManager *globalShapeManager;
    QPointer<KoShapeManager> shapeManager;
    KisSignalAutoConnectionsStore shapeManagerConnections;
};

KisSelectedShapesProxy::KisSelectedShapesProxy(KoShapeManager *globalShapeManager)
    : m_d(new Private())
{
    m_d->globalShapeManager = globalShapeManager;

    connect(m_d->globalShapeManager->selection(),
            SIGNAL(currentLayerChanged(const KoShapeLayer*)),
            SIGNAL(currentLayerChanged(const KoShapeLayer*)));
}

KisSelectedShapesProxy::~KisSelectedShapesProxy()
{

}

void KisSelectedShapesProxy::setShapeManager(KoShapeManager *shapeManager)
{
    if (shapeManager != m_d->shapeManager) {
        m_d->shapeManager = shapeManager;

        m_d->shapeManagerConnections.clear();

        if (m_d->shapeManager) {
            m_d->shapeManagerConnections.addConnection(
                m_d->shapeManager, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
            m_d->shapeManagerConnections.addConnection(
                m_d->shapeManager, SIGNAL(selectionContentChanged()), this, SIGNAL(selectionContentChanged()));
        }

        emit selectionChanged();
    }
}

KoSelection *KisSelectedShapesProxy::selection()
{
    return m_d->shapeManager ?
        m_d->shapeManager->selection() :
        m_d->globalShapeManager->selection();
}

