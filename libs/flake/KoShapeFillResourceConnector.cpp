/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KoShapeFillResourceConnector.h"

#include <KoCanvasResourceManager.h>
#include <KoSelectedShapesProxy.h>

#include "kis_assert.h"
#include "kis_signal_auto_connection.h"

#include <KoColor.h>
#include <KoFlake.h>
#include <KoShapeFillWrapper.h>
#include <KoSelection.h>
#include <KoCanvasBase.h>



struct KoShapeFillResourceConnector::Private
{
    KoCanvasBase *canvas;
    KisSignalAutoConnectionsStore resourceManagerConnections;

    void applyShapeColoring(KoFlake::FillVariant fillVariant, const KoColor &color);
};

KoShapeFillResourceConnector::KoShapeFillResourceConnector(QObject *parent)
    : QObject(parent),
      m_d(new Private())
{
}

KoShapeFillResourceConnector::~KoShapeFillResourceConnector()
{
}

void KoShapeFillResourceConnector::connectToCanvas(KoCanvasBase *canvas)
{
    m_d->resourceManagerConnections.clear();
    m_d->canvas = 0;

    KIS_SAFE_ASSERT_RECOVER_RETURN(!canvas || canvas->resourceManager());
    KIS_SAFE_ASSERT_RECOVER_RETURN(!canvas || canvas->selectedShapesProxy());

    m_d->canvas = canvas;

    if (m_d->canvas) {
        m_d->resourceManagerConnections.addConnection(
            canvas->resourceManager(), SIGNAL(canvasResourceChanged(int,QVariant)),
            this, SLOT(slotCanvasResourceChanged(int,QVariant)));
    }
}

void KoShapeFillResourceConnector::disconnect()
{
    connectToCanvas(0);
}

void KoShapeFillResourceConnector::slotCanvasResourceChanged(int key, const QVariant &value)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->canvas);

    if (key == KoCanvasResourceManager::ForegroundColor) {
        m_d->applyShapeColoring(KoFlake::Fill, value.value<KoColor>());
    } else if (key == KoCanvasResourceManager::BackgroundColor) {
        m_d->applyShapeColoring(KoFlake::StrokeFill, value.value<KoColor>());
    }
}


void KoShapeFillResourceConnector::Private::applyShapeColoring(KoFlake::FillVariant fillVariant, const KoColor &color)
{
    QList<KoShape *> selectedEditableShapes = canvas->selectedShapesProxy()->selection()->selectedEditableShapes();

    if (selectedEditableShapes.isEmpty()) {
        return;
    }

    KoShapeFillWrapper wrapper(selectedEditableShapes, fillVariant);
    KUndo2Command *command = wrapper.setColor(color.toQColor()); // TODO: do the conversion in a better way!

    if (command) {
        canvas->addCommand(command);
    }
}
