/*
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#include <qpoint.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kaction.h>
#include <klocale.h>
#include <knuminput.h>

#include <kis_cursor.h>
#include <kis_selection_manager.h>
#include <kis_canvas_subject.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_button_press_event.h>
#include <kis_canvas_subject.h>
#include <kis_selection_options.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_iterators_pixel.h>
#include <kis_selected_transaction.h>
#include <kis_undo_adapter.h>

#include "kis_tool_selectsimilar.h"

void selectByColor(KisPaintDeviceSP dev, KisSelectionSP selection, const Q_UINT8 * c, int fuzziness, enumSelectionMode mode)
{
    // XXX: Multithread this!
    Q_INT32 x, y, w, h;

    dev->exactBounds(x, y, w, h);

    KisColorSpace * cs = dev->colorSpace();

    for (int y2 = y; y2 < y + h; ++y2) {
        KisHLineIterator hiter = dev->createHLineIterator(x, y2, w, false);
        KisHLineIterator selIter = selection->createHLineIterator(x, y2, w, true);
        while (!hiter.isDone()) {
            //if (dev->colorSpace()->hasAlpha())
            //    opacity = dev->colorSpace()->getAlpha(hiter.rawData());

            Q_UINT8 match = cs->difference(c, hiter.rawData());

            if (mode == SELECTION_ADD) {
                if (match <= fuzziness) {
                    *(selIter.rawData()) = MAX_SELECTED;
                }
            }
            else if (mode == SELECTION_SUBTRACT) {
                if (match <= fuzziness) {
                    *(selIter.rawData()) = MIN_SELECTED;
                }
            }
            ++hiter;
            ++selIter;
        }
    }

}



KisToolSelectSimilar::KisToolSelectSimilar()
    : super(i18n("Select Similar Colors"))
{
    setName("tool_select_similar");
    m_addCursor = KisCursor::load("tool_similar_selection_plus_cursor.png", 1, 21);
    m_subtractCursor = KisCursor::load("tool_similar_selection_minus_cursor.png", 1, 21);
    setCursor(m_addCursor);
    m_subject = 0;
    m_optWidget = 0;
    m_selectionOptionsWidget = 0;
    m_fuzziness = 20;
    m_currentSelectAction = m_defaultSelectAction = SELECTION_ADD;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), SLOT(slotTimer()) );
}

KisToolSelectSimilar::~KisToolSelectSimilar()
{
}

void KisToolSelectSimilar::activate()
{
    KisToolNonPaint::activate();
    m_timer->start(50);
    setPickerCursor(m_currentSelectAction);

    if (m_selectionOptionsWidget) {
        m_selectionOptionsWidget->slotActivated();
    }
}

void KisToolSelectSimilar::deactivate()
{
    m_timer->stop();
}

void KisToolSelectSimilar::buttonPress(KisButtonPressEvent *e)
{

    if (m_subject) {
        QApplication::setOverrideCursor(KisCursor::waitCursor());
        KisImageSP img;
        KisPaintDeviceSP dev;
        QPoint pos;
        Q_UINT8 opacity = OPACITY_OPAQUE;

        if (e->button() != QMouseEvent::LeftButton && e->button() != QMouseEvent::RightButton)
            return;

        if (!(img = m_subject->currentImg()))
            return;

        dev = img->activeDevice();

        if (!dev || !img->activeLayer()->visible())
            return;

        pos = QPoint(e->pos().floorX(), e->pos().floorY());
        KisSelectedTransaction *t = 0;
        if (img->undo()) t = new KisSelectedTransaction(i18n("Similar Selection"),dev);

        KisColor c = dev->colorAt(pos.x(), pos.y());
        opacity = dev->colorSpace()->getAlpha(c.data());

        // XXX we should make this configurable: "allow to select transparent"
        // if (opacity > OPACITY_TRANSPARENT)
        selectByColor(dev, dev->selection(), c.data(), m_fuzziness, m_currentSelectAction);

        dev->setDirty();
        dev->emitSelectionChanged();

        if(img->undo())
            img->undoAdapter()->addCommand(t);
        m_subject->canvasController()->updateCanvas();

        QApplication::restoreOverrideCursor();
    }
}

void KisToolSelectSimilar::slotTimer()
{
#if KDE_IS_VERSION(3,4,0)
    int state = kapp->keyboardMouseState() & (Qt::ShiftButton|Qt::ControlButton|Qt::AltButton);
#else
    int state = kapp->keyboardModifiers() & (KApplication::ShiftModifier
            |KApplication::ControlModifier|KApplication::Modifier1);
#endif
    enumSelectionMode action;

    if (state == Qt::ShiftButton)
        action = SELECTION_ADD;
    else if (state == Qt::ControlButton)
        action = SELECTION_SUBTRACT;
    else
        action = m_defaultSelectAction;

    if (action != m_currentSelectAction) {
        m_currentSelectAction = action;
        setPickerCursor(action);
    }
}

void KisToolSelectSimilar::setPickerCursor(enumSelectionMode action)
{
    switch (action) {
        case SELECTION_ADD:
            m_subject->canvasController()->setCanvasCursor(m_addCursor);
            break;
        case SELECTION_SUBTRACT:
            m_subject->canvasController()->setCanvasCursor(m_subtractCursor);
    }
}

void KisToolSelectSimilar::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection->action(name()));

    if (m_action == 0) {
        m_action = new KRadioAction(i18n("&Similar Selection"), "tool_similar_selection", "Ctrl+E", this, SLOT(activate()), collection, name());
        Q_CHECK_PTR(m_action);
        m_action->setToolTip(i18n("Select similar colors"));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

void KisToolSelectSimilar::update(KisCanvasSubject *subject)
{
    super::update(subject);
    m_subject = subject;
}

void KisToolSelectSimilar::slotSetFuzziness(int fuzziness)
{
    m_fuzziness = fuzziness;
}

void KisToolSelectSimilar::slotSetAction(int action)
{
    m_defaultSelectAction = (enumSelectionMode)action;
}

QWidget* KisToolSelectSimilar::createOptionWidget(QWidget* parent)
{
    m_optWidget = new QWidget(parent);
    Q_CHECK_PTR(m_optWidget);

    m_optWidget->setCaption(i18n("Similar Selection"));

    QVBoxLayout * l = new QVBoxLayout(m_optWidget, 0, 6);
    Q_CHECK_PTR(l);

    m_selectionOptionsWidget = new KisSelectionOptions(m_optWidget, m_subject);
    Q_CHECK_PTR(m_selectionOptionsWidget);

    l->addWidget(m_selectionOptionsWidget);
    connect (m_selectionOptionsWidget, SIGNAL(actionChanged(int)), this, SLOT(slotSetAction(int)));

    QHBoxLayout * hbox = new QHBoxLayout(l);
    Q_CHECK_PTR(hbox);

    QLabel * lbl = new QLabel(i18n("Fuzziness: "), m_optWidget);
    Q_CHECK_PTR(lbl);

    hbox->addWidget(lbl);

    KIntNumInput * input = new KIntNumInput(m_optWidget, "fuzziness");
    Q_CHECK_PTR(input);

    input->setRange(0, 200, 10, true);
    input->setValue(20);
    hbox->addWidget(input);
    connect(input, SIGNAL(valueChanged(int)), this, SLOT(slotSetFuzziness(int)));

    l->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));

    return m_optWidget;
}

QWidget* KisToolSelectSimilar::optionWidget()
{
    return m_optWidget;
}

#include "kis_tool_selectsimilar.moc"
