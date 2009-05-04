/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "RulerAssistant.h"

#include <kdebug.h>
#include <klocale.h>

#include <Ruler.h>

RulerAssistant::RulerAssistant()
    : KisPaintingAssistant("ruler", i18n("Ruler assistant")),
      m_ruler( new Ruler)
{
}

QPointF RulerAssistant::adjustPosition( const QPointF& pt) const
{
    QPointF project = m_ruler->project( pt );
    return m_ruler->project( pt );
}

Ruler* RulerAssistant::ruler()
{
    return m_ruler;
}
