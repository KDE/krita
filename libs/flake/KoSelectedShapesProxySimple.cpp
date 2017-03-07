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

#include "KoSelectedShapesProxySimple.h"

#include "kis_assert.h"
#include <KoShapeManager.h>
#include <KoSelection.h>

KoSelectedShapesProxySimple::KoSelectedShapesProxySimple(KoShapeManager *shapeManager)
    : m_shapeManager(shapeManager)
{
    KIS_ASSERT_RECOVER_RETURN(m_shapeManager);

    connect(m_shapeManager.data(), SIGNAL(selectionChanged()), SIGNAL(selectionChanged()));
    connect(m_shapeManager.data(), SIGNAL(selectionContentChanged()), SIGNAL(selectionContentChanged()));
    connect(m_shapeManager->selection(), SIGNAL(currentLayerChanged(const KoShapeLayer*)), SIGNAL(currentLayerChanged(const KoShapeLayer*)));
}

KoSelection *KoSelectedShapesProxySimple::selection()
{
    KIS_ASSERT_RECOVER_RETURN_VALUE(m_shapeManager, 0);
    return m_shapeManager->selection();
}

