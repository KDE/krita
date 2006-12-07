/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_layer_container.h"

#include <QPainter>

#include <KoViewConverter.h>
#include <KoShapeContainer.h>

#include <kis_types.h>
#include <kis_group_layer.h>


class KisLayerContainer::Private
{

public:
    KisGroupLayerSP groupLayer;
};

KisLayerContainer::KisLayerContainer( KoShapeContainer *parent, KisGroupLayerSP groupLayer )
    : KoShapeContainer()
{
    m_d = new Private();
    m_d->groupLayer = groupLayer;

    setParent( parent );
    setShapeId( KIS_LAYER_CONTAINER_ID );
}

KisLayerContainer::~KisLayerContainer()
{
    delete m_d;
}

void KisLayerContainer::paintComponent(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED( painter );
    Q_UNUSED( converter );
}
