/*
 *  kis_tool_fill.h - part of Krayon^Krita
 *
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
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

#ifndef KIS_TOOL_FILL_H_
#define KIS_TOOL_FILL_H_

#include "kis_tool_paint.h"
#include <flake/kis_node_shape.h>
#include <KoIcon.h>
#include <kis_icon.h>
#include <QPoint>
#include <kconfig.h>
#include <kconfiggroup.h>



class QWidget;
class QCheckBox;
class KisSliderSpinBox;
class KoCanvasBase;

class KisToolFill : public KisToolPaint
{

    Q_OBJECT

public:

    KisToolFill(KoCanvasBase * canvas);
    virtual ~KisToolFill();

    void beginPrimaryAction(KoPointerEvent *event);
    void endPrimaryAction(KoPointerEvent *event);

    virtual QWidget * createOptionWidget();

public Q_SLOTS:
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    void slotSetUseFastMode(bool);
    void slotSetThreshold(int);
    void slotSetUsePattern(bool);
    void slotSetSampleMerged(bool);
    void slotSetFillSelection(bool);
    void slotSetSizemod(int);
    void slotSetFeather(int);

protected Q_SLOTS:
    virtual void resetCursorStyle();
protected:
    virtual bool wantsAutoScroll() const { return false; }
private:
    void updateGUI();

private:
    int m_feather;
    int m_sizemod;
    QPoint m_startPos;
    int m_threshold;
    bool m_unmerged;
    bool m_usePattern;
    bool m_fillOnlySelection;

    QCheckBox *m_useFastMode;
    KisSliderSpinBox *m_slThreshold;
    KisSliderSpinBox *m_sizemodWidget;
    KisSliderSpinBox *m_featherWidget;
    QCheckBox *m_checkUsePattern;
    QCheckBox *m_checkSampleMerged;
    QCheckBox *m_checkFillSelection;

    KConfigGroup m_configGroup;
};


#include "KoToolFactoryBase.h"

class KisToolFillFactory : public KoToolFactoryBase
{

public:
    KisToolFillFactory(const QStringList&)
            : KoToolFactoryBase("KritaFill/KisToolFill") {
        setToolTip(i18n("Fill Tool"));
        setToolType(TOOL_TYPE_FILL);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("krita_tool_color_fill"));
        //setShortcut( QKeySequence( Qt::Key_F ) );
        setPriority(14);
    }

    virtual ~KisToolFillFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolFill(canvas);
    }

};

#endif //__filltool_h__

