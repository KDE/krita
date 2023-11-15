/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "SvgTextShapeManagerBlocker.h"

SvgTextShapeManagerBlockerAdapter::SvgTextShapeManagerBlockerAdapter(KoShapeManager *shapeManager)
    :m_manager(shapeManager), m_managerState(shapeManager->updatesBlocked())
{

}

void SvgTextShapeManagerBlockerAdapter::lock()
{
    m_manager->setUpdatesBlocked(true);
}

void SvgTextShapeManagerBlockerAdapter::unlock()
{
    m_manager->setUpdatesBlocked(m_managerState);
}
