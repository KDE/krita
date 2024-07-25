/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_TOOL_SELECT_SIMILAR_H_
#define KIS_TOOL_SELECT_SIMILAR_H_

#include <KisSelectionToolFactoryBase.h>
#include <kis_icon.h>
#include <kconfig.h>
#include "kis_tool_select_base.h"
#include <kconfiggroup.h>
#include <commands_new/KisMergeLabeledLayersCommand.h>

/*
 * Tool to select colors by pointing at a color on the image.
 */
class KisToolSelectSimilar: public KisToolSelect
{
    Q_OBJECT

public:
    KisToolSelectSimilar(KoCanvasBase * canvas);
    void beginPrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;
    void paint(QPainter&, const KoViewConverter &) override {}
    QWidget* createOptionWidget() override;
    void resetCursorStyle() override;

public Q_SLOTS:
    void activate(const QSet<KoShape*> &shapes) override;
    void deactivate() override;
    void slotSetThreshold(int);
    void slotSetOpacitySpread(int);

protected:
    using KisToolSelectBase::m_widgetHelper;
    bool isPixelOnly() const override { return true; }
    bool usesColorLabels() const override { return true; }

private:
    int m_threshold;
    int m_opacitySpread;
    KConfigGroup m_configGroup;
    KisPaintDeviceSP m_referencePaintDevice;
    KisMergeLabeledLayersCommand::ReferenceNodeInfoListSP m_referenceNodeList;
    int m_previousTime;
};


class KisToolSelectSimilarFactory : public KisSelectionToolFactoryBase
{
public:
    KisToolSelectSimilarFactory()
        : KisSelectionToolFactoryBase("KisToolSelectSimilar")
    {
        setToolTip(i18n("Similar Color Selection Tool"));
        setSection(ToolBoxSection::Select);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("tool_similar_selection"));
        setPriority(5);
    }
    ~KisToolSelectSimilarFactory() override {}
    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return  new KisToolSelectSimilar(canvas);
    }
};


#endif // KIS_TOOL_SELECT_SIMILAR_H_

