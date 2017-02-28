/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KIS_TOOL_LAZY_BRUSH_H_
#define KIS_TOOL_LAZY_BRUSH_H_

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

class KisToolLazyBrush : public KisToolFreehand
{
    Q_OBJECT
public:
    KisToolLazyBrush(KoCanvasBase * canvas);
    virtual ~KisToolLazyBrush();

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
    bool colorizeMaskActive() const;
    bool canCreateColorizeMask() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};


class KisToolLazyBrushFactory : public KoToolFactoryBase
{

public:
    KisToolLazyBrushFactory()
            : KoToolFactoryBase("KritaShape/KisToolLazyBrush") {

        setToolTip(i18n("Colorize Mask Editing Tool"));

        // Temporarily
        setSection(TOOL_TYPE_FILL);
        setIconName(koIconNameCStr("krita_tool_lazybrush"));
        //setShortcut(QKeySequence(Qt::Key_Shift + Qt::Key_B));
        setPriority(3);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    virtual ~KisToolLazyBrushFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolLazyBrush(canvas);
    }

};


#endif // KIS_TOOL_LAZY_BRUSH_H_
