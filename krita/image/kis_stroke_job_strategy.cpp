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

#include "kis_stroke_job_strategy.h"

KisStrokeJobData::KisStrokeJobData(Sequentiality sequentiality,
                                   Exclusivity exclusivity)
    : m_sequentiality(sequentiality),
      m_exclusivity(exclusivity)
{
}

KisStrokeJobData::~KisStrokeJobData()
{
}

bool KisStrokeJobData::isBarrier() const
{
    return m_sequentiality == BARRIER;
}

bool KisStrokeJobData::isSequential() const
{
    return m_sequentiality == SEQUENTIAL;
}

bool KisStrokeJobData::isExclusive() const
{
    return m_exclusivity == EXCLUSIVE;
}


KisStrokeJobStrategy::KisStrokeJobStrategy()
{
}

KisStrokeJobStrategy::~KisStrokeJobStrategy()
{
}


