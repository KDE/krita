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
#ifndef KIS_TOOL_MANAGER
#define KIS_TOOL_MANAGER

#include <map>

#include <QObject>

#include "kis_tool_controller.h"
#include "kis_global.h"
#include "kis_tool_types.h"
#include "kis_input_device.h"

class QDockWidget;
class KoView;
class KisCanvasSubject;
class KisView;
class KisTool;
class KisToolRegistry;
class KisCanvasController;
class OldToolBox;

/**
 * This class manages the activation and deactivation of tools for
 * each input device.
 */
class KisToolManager : public QObject, public KisToolControllerInterface {

    Q_OBJECT

public:

    KisToolManager(KisCanvasSubject * parent, KisCanvasController * controller);
    ~KisToolManager();

public:

    void setUp(OldToolBox * toolbox, QDockWidget * toolPaletteWidget, KActionCollection * collection);
    
    // Called when the toolbox is deleted because the view was made inactive in favour of another view
    void youAintGotNoToolBox();
    
    void updateGUI();

    virtual void setCurrentTool(KisTool *tool);
    virtual void setCurrentTool(const QString & toolName);
    
    virtual KisTool *currentTool() const;
    
    void setToolForInputDevice(KisInputDevice oldDevice, KisInputDevice newDevice);

    KisTool *findTool(const QString &toolName, KisInputDevice inputDevice = KisInputDevice::unknown()) const;

    void activateCurrentTool();

private:
    
    void resetToolBox(OldToolBox * toolbox);
    
private:

    typedef std::map<KisInputDevice, KisTool *> InputDeviceToolMap;
    typedef std::map<KisInputDevice, vKisTool> InputDeviceToolSetMap;

    InputDeviceToolMap m_inputDeviceToolMap;
    InputDeviceToolSetMap m_inputDeviceToolSetMap;
    
    KisCanvasSubject * m_subject;
    KisCanvasController * m_controller;

    QDockWidget * m_toolPaletteWidget;
    KActionCollection * m_actionCollection;

    OldToolBox * m_toolBox;

    KisTool * m_oldTool;
    KisTool * m_dummyTool;
    
    vKisTool m_tools;

    bool m_tools_disabled;
    bool setup;
};


#endif
