/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_TOOL_TYPES_H_
#define KIS_TOOL_TYPES_H_

#include <ksharedptr.h>
#include "kis_shared_ptr_vector.h"


class KisTool;
typedef KSharedPtr<KisTool> KisToolSP;
typedef KisSharedPtrVector<KisTool> vKisTool;
typedef vKisTool::iterator vKisTool_it;
typedef vKisTool::const_iterator vKisTool_cit;

class KisToolFactory;
typedef KSharedPtr<KisToolFactory> KisToolFactorySP;

#endif // KIS_TOOL_TYPES_H_
