/*
 *  kis_tool_select_contiguous - part of Krayon^WKrita
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2012 José Luis Vergara <pentalis@gmail.com>
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

#include "kis_tool_select_contiguous.h"
#include <QPainter>
#include <QLayout>
#include <QLabel>
#include <QApplication>
#include <QCheckBox>
#include <QVBoxLayout>

#include <kis_debug.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

#include "KoPointerEvent.h"
#include "KoViewConverter.h"

#include "kis_cursor.h"
#include "kis_selection_manager.h"
#include "kis_image.h"
#include "canvas/kis_canvas2.h"
#include "kis_layer.h"
#include "kis_selection_options.h"
#include "kis_paint_device.h"
#include "kis_fill_painter.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"
#include "kis_slider_spin_box.h"
#include "tiles3/kis_hline_iterator.h"


KisToolSelectContiguous::KisToolSelectContiguous(KoCanvasBase *canvas)
    : KisToolSelect(canvas,
                    KisCursor::load("tool_contiguous_selection_cursor.png", 6, 6),
                    i18n("Contiguous Area Selection")),
    m_fuzziness(20),
    m_sizemod(0),
    m_feather(0),
    m_limitToCurrentLayer(false)
{
    setObjectName("tool_select_contiguous");
    connect(&m_widgetHelper, &KisSelectionToolConfigWidgetHelper::selectionActionChanged,
            this, &KisToolSelectContiguous::setSelectionAction);
}

KisToolSelectContiguous::~KisToolSelectContiguous()
{
}

void KisToolSelectContiguous::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    KisToolSelect::activate(toolActivation, shapes);
    m_configGroup =  KSharedConfig::openConfig()->group(toolId());
}

void KisToolSelectContiguous::beginPrimaryAction(KoPointerEvent *event)
{
    KisToolSelectBase::beginPrimaryAction(event);
    KisPaintDeviceSP dev;

    if (!currentNode() ||
        !(dev = currentNode()->projection()) ||
        !currentNode()->visible() ||
        !selectionEditable()) {
        event->ignore();
        return;
    }

    QApplication::setOverrideCursor(KisCursor::waitCursor());


    QPoint pos = convertToImagePixelCoordFloored(event);
    QRect rc = currentImage()->bounds();
    KisFillPainter fillpainter(dev);
    fillpainter.setHeight(rc.height());
    fillpainter.setWidth(rc.width());
    fillpainter.setFillThreshold(m_fuzziness);
    fillpainter.setFeather(m_feather);
    fillpainter.setSizemod(m_sizemod);

    KisImageWSP image = currentImage();
    KisPaintDeviceSP sourceDevice = m_limitToCurrentLayer ? dev : image->projection();

    image->lock();
    KisSelectionSP selection = fillpainter.createFloodSelection(pos.x(), pos.y(), sourceDevice);
    image->unlock();

    // If we're not antialiasing, threshold the entire selection
    if (!antiAliasSelection()) {
        QRect r = selection->selectedExactRect();
        if (r.isValid()) {
            KisHLineIteratorSP selectionIt = selection->pixelSelection()->createHLineIteratorNG(r.x(), r.y(), r.width());
            for (qint32 y = 0; y < r.height(); y++) {
                do {
                    if (selectionIt->rawData()[0] > 0) {
                        selection->pixelSelection()->colorSpace()->setOpacity(selectionIt->rawData(), OPACITY_OPAQUE_U8, 1);
                    }
                } while (selectionIt->nextPixel());
                selectionIt->nextRow();
            }
        }
    }


    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (!kisCanvas || !selection->pixelSelection()) {
        QApplication::restoreOverrideCursor();
        return;
    }

    selection->pixelSelection()->invalidateOutlineCache();
    KisSelectionToolHelper helper(kisCanvas, kundo2_i18n("Select Contiguous Area"));
    helper.selectPixelSelection(selection->pixelSelection(), selectionAction());
    QApplication::restoreOverrideCursor();

}

void KisToolSelectContiguous::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

void KisToolSelectContiguous::slotSetFuzziness(int fuzziness)
{
    m_fuzziness = fuzziness;
    m_configGroup.writeEntry("fuzziness", fuzziness);
}

void KisToolSelectContiguous::slotSetSizemod(int sizemod)
{
    m_sizemod = sizemod;
    m_configGroup.writeEntry("sizemod", sizemod);
}

void KisToolSelectContiguous::slotSetFeather(int feather)
{
    m_feather = feather;
    m_configGroup.writeEntry("feather", feather);
}

QWidget* KisToolSelectContiguous::createOptionWidget()
{
    KisToolSelectBase::createOptionWidget();
    KisSelectionOptions *selectionWidget = selectionOptionWidget();

    selectionWidget->disableSelectionModeOption();

    QVBoxLayout * l = dynamic_cast<QVBoxLayout*>(selectionWidget->layout());
    Q_ASSERT(l);
    if (l) {

        QGridLayout * gridLayout = new QGridLayout();
        l->insertLayout(1, gridLayout);

        QLabel * lbl = new QLabel(i18n("Fuzziness: "), selectionWidget);
        gridLayout->addWidget(lbl, 0, 0, 1, 1);

        KisSliderSpinBox *input = new KisSliderSpinBox(selectionWidget);
        Q_CHECK_PTR(input);
        input->setObjectName("fuzziness");
        input->setRange(1, 100);
        input->setSingleStep(1);
        input->setExponentRatio(2);
        gridLayout->addWidget(input, 0, 1, 1, 1);

        lbl = new QLabel(i18n("Grow/shrink selection: "), selectionWidget);
        gridLayout->addWidget(lbl, 1, 0, 1, 1);

        KisSliderSpinBox *sizemod = new KisSliderSpinBox(selectionWidget);
        Q_CHECK_PTR(sizemod);
        sizemod->setObjectName("sizemod"); //grow/shrink selection
        sizemod->setRange(-40, 40);
        sizemod->setSingleStep(1);
        gridLayout->addWidget(sizemod, 1, 1, 1, 1);

        lbl = new QLabel(i18n("Feathering radius: "), selectionWidget);
        gridLayout->addWidget(lbl, 2, 0, 1, 1);

        KisSliderSpinBox *feather = new KisSliderSpinBox(selectionWidget);
        Q_CHECK_PTR(feather);
        feather->setObjectName("feathering");
        feather->setRange(0, 40);
        feather->setSingleStep(1);
        gridLayout->addWidget(feather, 2, 1, 1, 1);

        connect (input  , SIGNAL(valueChanged(int)), this, SLOT(slotSetFuzziness(int) ));
        connect (sizemod, SIGNAL(valueChanged(int)), this, SLOT(slotSetSizemod(int)   ));
        connect (feather, SIGNAL(valueChanged(int)), this, SLOT(slotSetFeather(int)   ));

        QCheckBox* limitToCurrentLayer = new QCheckBox(i18n("Limit to current layer"), selectionWidget);
        l->insertWidget(4, limitToCurrentLayer);
        connect (limitToCurrentLayer, SIGNAL(stateChanged(int)), this, SLOT(slotLimitToCurrentLayer(int)));



        // load configuration settings into tool options
        input->setValue(m_configGroup.readEntry("fuzziness", 20)); // fuzziness
        sizemod->setValue( m_configGroup.readEntry("sizemod", 0)); //grow/shrink
        sizemod->setSuffix(i18n(" px"));

        feather->setValue(m_configGroup.readEntry("feather", 0));
        feather->setSuffix(i18n(" px"));

        limitToCurrentLayer->setChecked(m_configGroup.readEntry("limitToCurrentLayer", false));
    }
    return selectionWidget;
}

void KisToolSelectContiguous::slotLimitToCurrentLayer(int state)
{
    if (state == Qt::PartiallyChecked)
        return;
    m_limitToCurrentLayer = (state == Qt::Checked);
    m_configGroup.writeEntry("limitToCurrentLayer", state);
}


void KisToolSelectContiguous::setSelectionAction(int action)
{
    changeSelectionAction(action);
}


QMenu* KisToolSelectContiguous::popupActionsMenu()
{
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    Q_ASSERT(kisCanvas);


    return KisSelectionToolHelper::getSelectionContextMenu(kisCanvas);
}

void KisToolSelectContiguous::resetCursorStyle()
{
    if (selectionAction() == SELECTION_ADD) {
        useCursor(KisCursor::load("tool_contiguous_selection_cursor_add.png", 6, 6));
    } else if (selectionAction() == SELECTION_SUBTRACT) {
        useCursor(KisCursor::load("tool_contiguous_selection_cursor_sub.png", 6, 6));
    } else {
        KisToolSelect::resetCursorStyle();
    }
}
