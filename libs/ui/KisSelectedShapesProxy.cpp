/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

