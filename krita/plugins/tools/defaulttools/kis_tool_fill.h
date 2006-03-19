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

#ifndef __filltool_h__
#define __filltool_h__

#include <qpoint.h>

#include "kis_tool_paint.h"

class KisPainter;
class QWidget;
class QLabel;
class QCheckBox;
class KIntNumInput;
class KActionCollection;

class KisToolFill : public KisToolPaint {

    typedef KisToolPaint super;
    Q_OBJECT

public:

    KisToolFill();
    virtual ~KisToolFill();

    virtual void setup(KActionCollection *collection);
        virtual enumToolType toolType() { return TOOL_FILL; }

    virtual void update(KisCanvasSubject *subject);

    virtual void buttonPress(KisButtonPressEvent*);

    bool flood(int startX, int startY);

    virtual QWidget* createOptionWidget(QWidget* parent);

public slots:
    virtual void slotSetThreshold(int);
    virtual void slotSetUsePattern(int);
    virtual void slotSetSampleMerged(int);
    virtual void slotSetFillSelection(int);

private:
    int m_threshold;
    Q_INT32 m_depth;
    KisLayerSP m_lay;
    Q_UINT8* m_oldColor, *m_color;
    KisPainter *m_painter;
    KisCanvasSubject *m_subject;
    KisImageSP m_currentImage;
    bool *m_map, m_sampleMerged, m_usePattern, m_fillOnlySelection;
    KisSelectionSP m_selection;

    QLabel *m_lbThreshold;
    KIntNumInput *m_slThreshold;
    QCheckBox *m_checkUsePattern;
    QCheckBox *m_checkSampleMerged;
    QCheckBox *m_checkFillSelection;
};


#include "kis_tool_factory.h"

class KisToolFillFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolFillFactory() : super() {};
    virtual ~KisToolFillFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisToolFill * t = new KisToolFill();
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t;
    }
    virtual KisID id() { return KisID("fill", i18n("Fill Tool")); }

};




#endif //__filltool_h__

