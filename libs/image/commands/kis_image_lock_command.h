/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef KIS_IMAGE_LOCK_COMMAND_H_
#define KIS_IMAGE_LOCK_COMMAND_H_

#include "kis_types.h"
#include "kis_image_command.h"

/**
  * The command for image locking inside macro commands.
  *
  * It will ensurce that the image is properly locked during the execution
  * of macro commands. Place it at the start and end of the macro command.
  */
class KisImageLockCommand : public KisImageCommand
{

public:
    /**
     * Constructor
     * @param image The image the command will be working on.
     * @param lockImage Locking state of the image, while redo.
     */
    KisImageLockCommand(KisImageWSP image, bool lockImage);

    void redo() override;
    void undo() override;

private:
    bool m_lockImage;
};


#endif
