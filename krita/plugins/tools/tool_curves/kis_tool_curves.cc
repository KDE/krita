/*
 *  kis_tool_star.cc -- part of Krita
 *
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

/* Just an initial commit. Emanuele Tamponi */


#include <math.h>

#include <qpainter.h>
#include <qspinbox.h>
#include <qlayout.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdebug.h>
#include <knuminput.h>

#include "kis_doc.h"
#include "kis_view.h"
#include "kis_painter.h"
#include "kis_int_spinbox.h"
#include "kis_canvas_subject.h"
#include "kis_canvas_controller.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_paintop_registry.h"
#include "kis_canvas.h"
#include "kis_canvas_painter.h"
#include "kis_cursor.h"
#include "kis_int_spinbox.h"

#include "kis_tool_curves.h"
#include "wdg_tool_curves.h"

KisToolCurves::KisToolCurves()
    : super(i18n("Curves Initial Commit")),
      m_currentImage (0)
{
    setName("tool_curves");
    setCursor(KisCursor::load("tool_curves_cursor.png", 6, 6));
}

KisToolCurves::~KisToolCurves()
{
}

void KisToolCurves::update (KisCanvasSubject *subject)
{
    super::update (subject);
    if (m_subject)
        m_currentImage = m_subject->currentImg ();
}

void KisToolCurves::buttonPress(KisButtonPressEvent *event)
{
    if (m_currentImage && event->button() == LeftButton) {

    }
}

void KisToolCurves::move(KisMoveEvent *event)
{

}

void KisToolCurves::buttonRelease(KisButtonReleaseEvent *event)
{
    if (!m_subject || !m_currentImage)
        return;
}

void KisToolCurves::draw(const KisPoint& start, const KisPoint& end )
{
    if (!m_subject || !m_currentImage)
        return;
}

void KisToolCurves::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection->action(name()));

    if (m_action == 0) {
        KShortcut shortcut(Qt::Key_Plus);
        shortcut.append(KShortcut(Qt::Key_F9));
        m_action = new KRadioAction(i18n("&Curves"),
                                    "tool_curves",
                                    shortcut,
                                    this,
                                    SLOT(activate()),
                                    collection,
                                    name());
        Q_CHECK_PTR(m_action);

        m_action->setToolTip(i18n("Draw a curves"));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

QWidget* KisToolCurves::createOptionWidget(QWidget* parent)
{
    QWidget *widget = super::createOptionWidget(parent);

    m_optWidget = new WdgToolCurves(widget);
    Q_CHECK_PTR(m_optWidget);

//    m_optWidget->ratioSpinBox->setValue(m_innerOuterRatio);

    QGridLayout *optionLayout = new QGridLayout(widget, 1, 1);
    super::addOptionWidgetLayout(optionLayout);

    optionLayout->addWidget(m_optWidget, 0, 0);

    return widget;
}

#include "kis_tool_curves.moc"
