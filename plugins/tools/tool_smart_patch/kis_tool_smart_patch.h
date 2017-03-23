/*
 *  Copyright (c) 2017 Eugene Ingerman
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

#ifndef KIS_TOOL_SMART_PATCH_H_
#define KIS_TOOL_SMART_PATCH_H_

#include <QScopedPointer>
#include "kis_tool_freehand.h"

#include "KoToolFactoryBase.h"

#include <flake/kis_node_shape.h>
#include <kis_icon.h>
#include <QKeySequence>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <KoIcon.h>

class KActionCollection;
class KoCanvasBase;

class KisToolSmartPatch : public KisToolFreehand
{
    Q_OBJECT
public:
    KisToolSmartPatch(KoCanvasBase * canvas);
    virtual ~KisToolSmartPatch();

    QWidget * createOptionWidget();

    void activatePrimaryAction();
    void deactivatePrimaryAction();

    void beginPrimaryAction(KoPointerEvent *event);
    void continuePrimaryAction(KoPointerEvent *event);
    void endPrimaryAction(KoPointerEvent *event);

protected Q_SLOTS:
    void resetCursorStyle();

public Q_SLOTS:
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    void deactivate();

Q_SIGNALS:

private:
    bool canCreateInpaintMask() const;
    void inpaintImage( KisPaintDeviceSP maskDev, KisPaintDeviceSP imageDev );

private:
    struct Private;
    const QScopedPointer<Private> m_d;

};


class KisToolSmartPatchFactory : public KoToolFactoryBase
{

public:
    KisToolSmartPatchFactory()
            : KoToolFactoryBase("KritaShape/KisToolSmartPatch") {

        setToolTip(i18n("Smart Patch Tool"));

        // Temporarily
        setSection(TOOL_TYPE_FILL);
        setIconName(koIconNameCStr("krita_tool_smart_patch"));
        //setShortcut(QKeySequence(Qt::Key_Shift + Qt::Key_B));
        setPriority(4);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    virtual ~KisToolSmartPatchFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolSmartPatch(canvas);
    }

};


#endif // KIS_TOOL_SMART_PATCH_H_
