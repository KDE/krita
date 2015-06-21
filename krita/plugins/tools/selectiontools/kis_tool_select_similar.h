/*
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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
#ifndef KIS_TOOL_SELECT_SIMILAR_H_
#define KIS_TOOL_SELECT_SIMILAR_H_

#include <KoToolFactoryBase.h>
#include <KoIcon.h>
#include <kconfig.h>
#include "kis_tool_select_base.h"
#include <kconfiggroup.h>



/*
 * Tool to select colors by pointing at a color on the image.
 */
class KisToolSelectSimilar: public KisToolSelectBase
{
    Q_OBJECT
    Q_PROPERTY(int selectionAction READ selectionAction WRITE setSelectionAction NOTIFY selectionActionChanged)
    Q_SIGNALS: void selectionActionChanged();

public:
    KisToolSelectSimilar(KoCanvasBase * canvas);
    void beginPrimaryAction(KoPointerEvent *event);
    void paint(QPainter&, const KoViewConverter &) {}
    QWidget* createOptionWidget();


    public Q_SLOTS:    
    void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    void setSelectionAction(int newSelectionAction);
    void slotSetFuzziness(int);

protected:
    using KisToolSelectBase::m_widgetHelper;

private:
    int m_fuzziness;
    KConfigGroup m_configGroup;
};


class KisToolSelectSimilarFactory : public KoToolFactoryBase
{
public:
    KisToolSelectSimilarFactory(const QStringList&)
            : KoToolFactoryBase("KisToolSelectSimilar") {
        setToolTip(i18n("Similar Color Selection Tool"));
        setToolType(TOOL_TYPE_SELECTED);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("tool_similar_selection"));
        setPriority(57);
    }
    virtual ~KisToolSelectSimilarFactory() {}
    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return  new KisToolSelectSimilar(canvas);
    }
};


#endif // KIS_TOOL_SELECT_SIMILAR_H_

