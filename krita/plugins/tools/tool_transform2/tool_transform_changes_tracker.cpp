/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include "tool_transform_changes_tracker.h"


TransformChangesTracker::TransformChangesTracker(TransformTransactionProperties *transaction)
    : m_transaction(transaction)
{
}

void TransformChangesTracker::commitConfig(const ToolTransformArgs &config)
{
    m_config.append(config);
}

void TransformChangesTracker::requestUndo()
{
    if (m_config.size() > 1) {
        m_config.removeLast();
        *m_transaction->currentConfig() = m_config.last();
        emit sigConfigChanged();
    }
}

void TransformChangesTracker::reset()
{
    m_config.clear();
}

bool TransformChangesTracker::isEmpty() const
{
    return m_config.isEmpty();
}
