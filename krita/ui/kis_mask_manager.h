/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
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
#ifndef KIS_MASK_MANAGER
#define KIS_MASK_MANAGER

#include <QObject>

#include "kis_types.h"

class KisView2;
class KActionCollection;
class KAction;
class KToggleAction;
class KisNodeCommandsAdapter;

#include "kis_mask.h"

/**
 * Handle the gui for manipulating masks.
 */
class KisMaskManager : public QObject
{

    Q_OBJECT

public:


    KisMaskManager(KisView2 * view);
    ~KisMaskManager() {}

signals:

    void sigMaskActivated(KisMaskSP mask);

private slots:
    
    /**
     * Create a new global selection from the active mask.
     */
    void maskToSelection();

    /**
     * Create a new layer from the current mask. The user can choose
     * the colorspace of the new layer and which channels should be
     * filled from the mask channel.
     */
    void maskToLayer();

private:
    
    friend class KisNodeManager;
    
    void setup(KActionCollection * actionCollection);

    void updateGUI();
    
    /**
     * @return the paint device associated with the currently
     *         active mask, if there is one.
     */
    KisPaintDeviceSP activeDevice();

    /**
     * @return the active mask, if there is one
     */
    KisMaskSP activeMask();

    /**
     * Create a new transparency mask.
     */
    void createTransparencyMask();

    /**
     * Create a new filter mask.
     */
    void createFilterMask();


    /**
     * Create an exact duplicate of the current mask.
     */
    void duplicateMask();

    /**
     * Delete the mask
     */
    void removeMask();

    /**
     * Move the mask one up in the mask stack. If we reach the top
     * of the stack, try to move the mask to the layer on top of the
     * active layer.
     */
    void raiseMask();

    /**
     * Move the mask one down in the mask stack. If we reach the bottom
     * of the stack, try to move the mask to the layer beneath the
     * active layer.
     */
    void lowerMask();

    /**
     * Move the mask to the top of the mask stack of the active layer
     */
    void maskToTop();

    /**
     * Move the mask to the bottom of the mask stack of the active
     * layer
     */
    void maskToBottom();

    /**
     * Show the mask properties dialog
     */
    void maskProperties();

    /**
     * called whenever the mask stack is updated to enable/disable all
     * menu items
     */
    void masksUpdated();

    /**
     * Activate a new mask. There can be only one mask active per
     * view; and if the mask is active, it becomes the paint device.
     */
    void activateMask(KisMaskSP mask);

    void createSelectionMask(KisNodeSP parent, KisNodeSP above);
    void createFilterMask(KisNodeSP parent, KisNodeSP above);
    void createTransparencyMask(KisNodeSP parent, KisNodeSP above);

    KisView2 * m_view;

    KisMaskSP m_activeMask;
    KAction *m_maskToSelection;
    KAction *m_maskToLayer;
    KisNodeCommandsAdapter* m_commandsAdapter;

};

#endif // KIS_MASK_MANAGER
