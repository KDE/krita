/*
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
#ifndef KIS_SELECTION_COMMANDS_H_
#define KIS_SELECTION_COMMANDS_H_

#include <krita_export.h>
#include <QUndoCommand>
#include "kis_types.h"

/// The command for setting the global selection
class KRITAIMAGE_EXPORT KisSetGlobalSelectionCommand : public QUndoCommand
{

public:
    /**
     * Constructor
     * @param image the image to set the global selection on
     * @param parent the parent command
     * @param selection the selection that will be set a global selection, if 0 a new selection will be created
     */
    KisSetGlobalSelectionCommand(KisImageSP image, QUndoCommand * parent, KisSelectionSP selection = 0);
    virtual ~KisSetGlobalSelectionCommand();

    virtual void redo();
    virtual void undo();

private:
    KisImageSP m_image;
    KisSelectionSP m_newSelection;
    KisSelectionSP m_oldSelection;
};

/// The command for deselection the local selection (KisSelectionMask)
class KRITAIMAGE_EXPORT KisDeselectLocalSelectionCommand : public QUndoCommand
{

public:
    /**
     * Constructor
     * @param image the image
     * @param selectionMask the the selection mask which will get deselected
     * @param parent the parent command
     */
    KisDeselectLocalSelectionCommand(KisImageSP image, KisSelectionMaskSP selectionMask, QUndoCommand * parent = 0);
    virtual ~KisDeselectLocalSelectionCommand();

    virtual void redo();
    virtual void undo();

private:
    KisImageSP m_image;
    KisSelectionMaskSP m_selectionMask;
    KisSelectionSP m_newSelection;
    KisSelectionSP m_oldDeselectedSelection;
};

/// The command for reselection the local selection (KisSelectionMask)
class KRITAIMAGE_EXPORT KisReselectLocalSelectionCommand : public QUndoCommand
{

public:
    /**
     * Constructor
     * @param image the image
     * @param selectionMask the the selection mask which will get reselected
     * @param parent the parent command
     */
    KisReselectLocalSelectionCommand(KisImageSP image, KisSelectionMaskSP selectionMask, QUndoCommand * parent = 0);
    virtual ~KisReselectLocalSelectionCommand();

    virtual void redo();
    virtual void undo();

private:
    KisImageSP m_image;
    KisSelectionMaskSP m_selectionMask;
    KisSelectionSP m_oldSelection;
};

/// The command for deselection the global selection of KisImage
class KRITAIMAGE_EXPORT KisDeselectGlobalSelectionCommand : public QUndoCommand
{

public:
    /**
     * Constructor
     * @param image the image
     * @param parent the parent command
     */
    KisDeselectGlobalSelectionCommand(KisImageSP image, QUndoCommand * parent = 0);
    virtual ~KisDeselectGlobalSelectionCommand();

    virtual void redo();
    virtual void undo();

private:
    KisImageSP m_image;
    KisSelectionSP m_newSelection;
    KisSelectionSP m_oldDeselectedSelection;
};

/// The command for deselection the global selection of KisImage
class KRITAIMAGE_EXPORT KisReselectGlobalSelectionCommand : public QUndoCommand
{

public:
    /**
     * Constructor
     * @param image the image
     * @param parent the parent command
     */
    KisReselectGlobalSelectionCommand(KisImageSP image, QUndoCommand * parent = 0);
    virtual ~KisReselectGlobalSelectionCommand();

    virtual void redo();
    virtual void undo();

private:
    KisImageSP m_image;
    KisSelectionSP m_oldSelection;
};


#endif // KIS_SELECTION_COMMANDS_H_
