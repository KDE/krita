/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

