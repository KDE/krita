/*
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#include <qcombobox.h>
#include <qlistview.h>

#include <kaction.h>
#include <klocale.h>
#include <qcolor.h>
#include <kmessagebox.h>

#include "kis_cursor.h"
#include "kis_canvas_subject.h"
#include "kis_image.h"
#include "kis_paint_device_impl.h"
#include "kis_tool_colorpicker.h"
#include "kis_tool_colorpicker.moc"
#include "kis_button_press_event.h"
#include "kis_canvas_subject.h"
#include "kis_color.h"
#include "wdgcolorpicker.h"

namespace {
    // The location of the sample all visible layers in the combobox
    const int SAMPLE_MERGED = 0;
}

KisToolColorPicker::KisToolColorPicker()
{
    setName("tool_colorpicker");
    setCursor(KisCursor::pickerCursor());
    m_optionsWidget = 0;
    m_subject = 0;
    m_updateColor = true;
    m_normaliseValues = true;
    m_pickedColor = KisColor();
}

KisToolColorPicker::~KisToolColorPicker() 
{
}

void KisToolColorPicker::update(KisCanvasSubject *subject)
{
    m_subject = subject;
    super::update(m_subject);
}

void KisToolColorPicker::buttonPress(KisButtonPressEvent *e)
{
    if (m_subject) {
        if (e -> button() != QMouseEvent::LeftButton && e -> button() != QMouseEvent::RightButton)
            return;

        KisImageSP img;

        if (!m_subject || !(img = m_subject -> currentImg()))
            return;

        KisPaintDeviceImplSP dev = img -> activeDevice();

        bool sampleMerged = m_optionsWidget->cmbSources->currentItem() == SAMPLE_MERGED;
        if (!sampleMerged) {
            // Find layer
            QString layerName = m_optionsWidget->cmbSources->currentText();
            KisPaintDeviceImplSP dev = img->findLayer(layerName).data();

            if (!dev ) {
                return;
            }
            if (!dev -> visible()) {
                KMessageBox::information(0, i18n("Cannot pick the color as the active layer is hidden."));
                return;
            }
        }

        QPoint pos = QPoint(e -> pos().floorX(), e -> pos().floorY());

        if (!img -> bounds().contains(pos)) {
            return;
        }

        if (sampleMerged) {
            m_pickedColor = img -> mergedPixel(pos.x(), pos.y());
        } else {
            m_pickedColor = dev -> colorAt(pos.x(), pos.y());
        }

        displayPickedColor();

        if (m_updateColor) {
            if (e -> button() == QMouseEvent::LeftButton)
                m_subject -> setFGColor(m_pickedColor);
            else 
                m_subject -> setBGColor(m_pickedColor);
        }
    }
}

void KisToolColorPicker::displayPickedColor()
{
    if (m_pickedColor.data() && m_optionsWidget) {

        vKisChannelInfoSP channels = m_pickedColor.colorSpace() -> channels();
        m_optionsWidget -> listViewChannels -> clear();

        for (int i = channels.count() - 1; i >= 0 ; --i) {
            QString channelValueText;

            if (m_normaliseValues) {
                channelValueText = m_pickedColor.colorSpace() -> normalisedChannelValueText(m_pickedColor.data(), i);
            } else {
                channelValueText = m_pickedColor.colorSpace() -> channelValueText(m_pickedColor.data(), i);
            }

            m_optionsWidget -> listViewChannels -> insertItem(new QListViewItem(m_optionsWidget -> listViewChannels,
                                                channels[i] -> name(),
                                                channelValueText));
        }
    }
}

void KisToolColorPicker::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection -> action(name()));

    if (m_action == 0) {
        m_action = new KRadioAction(i18n("&Color Picker"), "colorpicker", Qt::Key_E, this, SLOT(activate()), collection, name());
        m_action -> setToolTip(i18n("Color picker"));
        m_action -> setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

QWidget* KisToolColorPicker::createOptionWidget(QWidget* parent)
{
    m_optionsWidget = new ColorPickerOptionsWidget(parent);
    
    m_optionsWidget -> cbUpdateCurrentColour -> setChecked(m_updateColor);
    m_optionsWidget -> cmbSources -> setCurrentItem(0);
    m_optionsWidget -> cbNormaliseValues -> setChecked(m_normaliseValues);

    m_optionsWidget -> listViewChannels -> setSorting(-1);

    const KisImageSP img = m_subject->currentImg();
    if (img) {
        vKisLayerSP layers = img->layers();
        
        for (vKisLayerSP_cit it = layers.begin(); it != layers.end(); ++it) {
            const KisLayerSP& layer = *it;
            if (layer->visible()) {
                m_optionsWidget->cmbSources->insertItem(layer->name());
            }
        }
    }

    connect(m_optionsWidget -> cbUpdateCurrentColour, SIGNAL(toggled(bool)), SLOT(slotSetUpdateColor(bool)));
    connect(m_optionsWidget -> cbNormaliseValues, SIGNAL(toggled(bool)), SLOT(slotSetNormaliseValues(bool)));

    return m_optionsWidget;
}

QWidget* KisToolColorPicker::optionWidget()
{
    return m_optionsWidget;
}

void KisToolColorPicker::slotSetUpdateColor(bool state)
{
    m_updateColor = state;
}


void KisToolColorPicker::slotSetNormaliseValues(bool state)
{
    m_normaliseValues = state;
    displayPickedColor();
}

