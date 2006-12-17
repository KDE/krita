/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#include "KoShapeShearCommand.h"

#include <klocale.h>

KoShapeShearCommand::KoShapeShearCommand(const KoSelectionSet &shapes, QList<double> &previousShearXs, QList<double> &previousShearYs, QList<double> &newShearXs, QList<double> &newShearYs)
: m_previousShearXs(previousShearXs)
, m_previousShearYs(previousShearYs)
, m_newShearXs(newShearXs)
, m_newShearYs(newShearYs)
{
    m_shapes = shapes.toList();
    Q_ASSERT(m_shapes.count() == m_previousShearXs.count());
    Q_ASSERT(m_shapes.count() == m_previousShearYs.count());
    Q_ASSERT(m_shapes.count() == m_newShearXs.count());
    Q_ASSERT(m_shapes.count() == m_newShearYs.count());
}

void KoShapeShearCommand::execute() {
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->shear( m_newShearXs.at(i), m_newShearYs.at(i));
        m_shapes.at(i)->repaint();
    }
}

void KoShapeShearCommand::unexecute() {
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->shear( m_previousShearXs.at(i), m_previousShearYs.at(i) );
        m_shapes.at(i)->repaint();
    }
}

QString KoShapeShearCommand::name () const {
    return i18n( "Shear shapes" );
}
