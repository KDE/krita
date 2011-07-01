/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_stroke_strategy.h"


KisStrokeStrategy::KisStrokeStrategy(QString id, QString name)
    : m_exclusive(false),
      m_needsIndirectPainting(false),
      m_id(id),
      m_name(name)
{
}

KisStrokeStrategy::~KisStrokeStrategy()
{
}


KisDabProcessingStrategy* KisStrokeStrategy::createInitStrategy()
{
    return 0;
}

KisDabProcessingStrategy* KisStrokeStrategy::createFinishStrategy()
{
    return 0;
}

KisDabProcessingStrategy* KisStrokeStrategy::createCancelStrategy()
{
    return 0;
}

KisDabProcessingStrategy* KisStrokeStrategy::createDabStrategy()
{
    return 0;
}

bool KisStrokeStrategy::isExclusive() const
{
    return m_exclusive;
}

bool KisStrokeStrategy::needsIndirectPainting() const
{
    return m_needsIndirectPainting;
}

QString KisStrokeStrategy::id() const
{
    return m_id;
}

QString KisStrokeStrategy::name() const
{
    return m_name;
}

void KisStrokeStrategy::setExclusive(bool value)
{
    m_exclusive = value;
}

void KisStrokeStrategy::setNeedsIndirectPainting(bool value)
{
    m_needsIndirectPainting = value;
}
