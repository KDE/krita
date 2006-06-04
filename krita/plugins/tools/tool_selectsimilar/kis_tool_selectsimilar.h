/*
 *  Copyright (c) 1999 Matthias Elter
 *  Copyright (c) 2002 Patrick Julien
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
#ifndef KIS_TOOL_SELECT_PICKER_H_
#define KIS_TOOL_SELECT_PICKER_H_

#include <kis_tool_non_paint.h>
#include <kis_tool_factory.h>
#include <kis_selection.h>

class KisCanvasSubject;
class QWidget;
class QCheckBox;
class KisIntSpinbox;

/**
 * Tool to select colours by pointing at a color on the image.
 * TODO:
 *       Implement shift/shift-ctrl keyboard shortcuts for
 *       temporary add/subtract selection mode.
 */

class KisSelectionOptions;

class KisToolSelectSimilar : public KisToolNonPaint {

    Q_OBJECT
    typedef KisToolNonPaint super;

public:
    KisToolSelectSimilar();
    virtual ~KisToolSelectSimilar();

    virtual void update(KisCanvasSubject *subject);
    virtual void setup(KActionCollection *collection);
    virtual quint32 priority() { return 8; }
    virtual enumToolType toolType() { return TOOL_SELECT; }

public slots:

    void activate();
    void deactivate();

    virtual void slotSetFuzziness(int);
    virtual void slotSetAction(int);

private:
    virtual QWidget* createOptionWidget(QWidget* parent);
    virtual QWidget* optionWidget();

    virtual void buttonPress(KisButtonPressEvent *e);
    void setPickerCursor(enumSelectionMode);

    KisCanvasSubject *m_subject;
    QWidget *m_optWidget;
    KisSelectionOptions *m_selectionOptionsWidget;

    int m_fuzziness;
    enumSelectionMode m_defaultSelectAction;
    enumSelectionMode m_currentSelectAction;
    QTimer *m_timer;
    QCursor m_addCursor;
    QCursor m_subtractCursor;

private slots:
    void slotTimer();
};

class KisToolSelectSimilarFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolSelectSimilarFactory() : super() {};
    virtual ~KisToolSelectSimilarFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolSelectSimilar();
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t;
    }
    virtual KoID id() { return KoID("selectsimilar", i18n("Select Similar")); }
};


#endif // KIS_TOOL_SELECT_PICKER_H_

