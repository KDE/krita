/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
