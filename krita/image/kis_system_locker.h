/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_SYSTEM_LOCKER_H_
#define _KIS_SYSTEM_LOCKER_H_

#include "kis_base_node.h"

/**
 * This class will lock a \ref KisBaseNode upon construction, and unlock it
 * at destruction. Use it before processing a node.
 */
class KisSystemLocker
{
public:
    inline KisSystemLocker(KisBaseNodeSP _node) : m_node(_node) {
        Q_ASSERT(!_node->systemLocked());
        m_node->setSystemLocked(true);
    }
    inline ~KisSystemLocker() {
        m_node->setSystemLocked(false);
    }
private:
    KisBaseNodeSP m_node;

};


#endif
