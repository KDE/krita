/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_MASK_MANAGER
#define KIS_MASK_MANAGER

#include <QObject>
#include <QPointer>

#include "kis_types.h"
#include "KisView.h"

class KisViewManager;
class KActionCollection;
class KisNodeCommandsAdapter;
class KisActionManager;

#include "kis_mask.h"

/**
 * Handle the gui for manipulating masks.
 */
class KisMaskManager : public QObject
{

    Q_OBJECT

public:


    KisMaskManager(KisViewManager * view);
    ~KisMaskManager() override {}
    void setView(QPointer<KisView>view);

private:
    
    friend class KisNodeManager;
    
    void setup(KActionCollection * actionCollection, KisActionManager *actionManager);

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

    void adjustMaskPosition(KisNodeSP node, KisNodeSP activeNode, bool avoidActiveNode, KisNodeSP &parent, KisNodeSP &above);
    void createMaskCommon(KisMaskSP mask, KisNodeSP activeNode, KisPaintDeviceSP copyFrom, const KUndo2MagicString &macroName, const QString &nodeType, const QString &nodeName, bool suppressSelection, bool avoidActiveNode, bool updateImage = true);
    QString createMaskNameCommon(KisNodeSP targetNode, const QString& nodeType, const QString &desiredName);

    KisNodeSP createSelectionMask(KisNodeSP activeNode, KisPaintDeviceSP copyFrom, bool convertActiveNode);
    KisNodeSP createFilterMask(KisNodeSP activeNode, KisPaintDeviceSP copyFrom, bool quiet, bool convertActiveNode);
    KisNodeSP createColorizeMask(KisNodeSP activeNode);
    KisNodeSP createTransformMask(KisNodeSP activeNode);
    KisNodeSP createTransparencyMask(KisNodeSP activeNode, KisPaintDeviceSP copyFrom, bool convertActiveNode);

    KisViewManager * m_view;
    QPointer<KisView>m_imageView;
    KisNodeCommandsAdapter* m_commandsAdapter;

};

#endif // KIS_MASK_MANAGER
