/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (c) 2015 Michael Abrahams <miabraha@gmail.com>
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

#ifndef KIS_TOOL_SELECT_MAGNETIC_H_
#define KIS_TOOL_SELECT_MAGNETIC_H_

#include <KoCreatePathTool.h>
#include <KisSelectionToolFactoryBase.h>
#include "kis_tool_select_base.h"
#include "kis_delegated_tool.h"

#include <kis_icon.h>

class KoCanvasBase;
class KisToolSelectMagnetic;


class __KisToolSelectMagneticLocalTool : public KoCreatePathTool {
public:
    __KisToolSelectMagneticLocalTool(KoCanvasBase * canvas, KisToolSelectMagnetic* parentTool);
    void paintPath(KoPathShape &path, QPainter &painter, const KoViewConverter &converter) override;
    void addPathShape(KoPathShape* pathShape) override;

    friend class KisToolSelectMagnetic;

    using KoCreatePathTool::createOptionWidgets;
    using KoCreatePathTool::endPathWithoutLastPoint;
    using KoCreatePathTool::endPath;
    using KoCreatePathTool::cancelPath;
    using KoCreatePathTool::removeLastPoint;

protected:
    void paintOutline(QPainter *painter, const QPainterPath &path, qreal width);

private:
    KisToolSelectMagnetic* const m_selectionTool;
};

typedef KisDelegatedTool<KisTool, __KisToolSelectMagneticLocalTool,
DeselectShapesActivationPolicy> DelegatedSelectMagneticTool;

struct KisDelegatedSelectMagneticWrapper : public DelegatedSelectMagneticTool {
    KisDelegatedSelectMagneticWrapper(KoCanvasBase *canvas,
                                  const QCursor &cursor,
                                  KisTool* delegateTool)
        : DelegatedSelectMagneticTool(canvas, cursor, dynamic_cast<__KisToolSelectMagneticLocalTool*>(delegateTool))
    {
    }

    // If an event is explicitly forwarded only as an action (e.g. shift-click is captured by "change size")
    // we will receive a primary action but no mousePressEvent.  Thus these events must be explicitly forwarded.
    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;

    bool hasUserInteractionRunning() const;
};


class KisToolSelectMagnetic : public KisToolSelectBase<KisDelegatedSelectMagneticWrapper>
{
    Q_OBJECT
public:
    KisToolSelectMagnetic(KoCanvasBase * canvas);
    void mousePressEvent(KoPointerEvent* event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resetCursorStyle() override;

protected:
    void requestStrokeCancellation() override;
    void requestStrokeEnd() override;
    friend class __KisToolSelectMagneticLocalTool;
    QList<QPointer<QWidget> > createOptionWidgets() override;
};

class KisToolSelectMagneticFactory : public KisSelectionToolFactoryBase
{
public:
    KisToolSelectMagneticFactory()
        : KisSelectionToolFactoryBase("KisToolSelectMagnetic") {
        setToolTip(i18n("Magnetic Selection Tool"));
        setSection(TOOL_TYPE_SELECTION);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("tool_magnetic_selection"));
        setPriority(6);
    }

    ~KisToolSelectMagneticFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolSelectMagnetic(canvas);
    }


};



#endif // KIS_TOOL_SELECT_MAGNETIC_H_
