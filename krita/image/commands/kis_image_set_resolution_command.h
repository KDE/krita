/*
 *  Copyright (c) 2010 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef KIS_IMAGE_SET_RESOLUTION_COMMAND_H
#define KIS_IMAGE_SET_RESOLUTION_COMMAND_H

#include <krita_export.h>

#include "kis_image_command.h"
#include "kis_types.h"

class KRITAIMAGE_EXPORT KisImageSetResolutionCommand : public KisImageCommand
{

public:
    KisImageSetResolutionCommand(KisImageSP image, double newXRes, double newYRes);
    virtual void undo();
    virtual void redo();
    
private:
    double m_newXRes;
    double m_newYRes;
    double m_oldXRes;
    double m_oldYRes;
};

#endif // KIS_IMAGE_SET_RESOLUTION_COMMAND_H
