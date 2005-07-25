/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef KIS_RENDER_INFORMATION_H_
#define KIS_RENDER_INFORMATION_H_

#include <ksharedptr.h>

/**
 * This class can contain information a color strategy might need to render a paint device
 * to a QImage
 **/
class KisRenderInformation : public KShared
{
	// Entirely free for the color strategies to define
};

typedef KSharedPtr<KisRenderInformation> KisRenderInformationSP;


#endif // KIS_RENDER_INFORMATION_H_
