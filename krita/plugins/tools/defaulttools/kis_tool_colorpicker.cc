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
#include <string.h>

#include <qpoint.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <QListWidget>
#include <QList>

#include <kaction.h>
#include <klocale.h>
#include <qcolor.h>
#include <kmessagebox.h>

#include "kis_layer.h"
#include "kis_cursor.h"
#include "kis_canvas_subject.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_tool_colorpicker.h"
#include "kis_tool_colorpicker.moc"
#include "kis_button_press_event.h"
#include "kis_canvas_subject.h"
#include "kis_iterators_pixel.h"
#include "kis_color.h"
#include "kis_resourceserver.h"
#include "kis_palette.h"

namespace {
    // The location of the sample all visible layers in the combobox
    const int SAMPLE_MERGED = 0;
}

KisToolColorPicker::KisToolColorPicker()
    : super (i18n("Color Picker"))
{
    setObjectName("tool_colorpicker");
    setCursor(KisCursor::pickerCursor());
    m_optionsWidget = 0;
    m_subject = 0;
    m_radius = 1;
    m_addPalette = false;
    m_updateColor = true;
    m_normaliseValues = false;
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
        if (e->button() != Qt::LeftButton && e->button() != Qt::RightButton)
            return;

        KisImageSP img;

        if (!m_subject || !(img = m_subject->currentImg()))
            return;

        KisPaintDeviceSP dev = img->activeDevice();

        if (!dev) return;

        bool sampleMerged = m_optionsWidget->cmbSources->currentIndex() == SAMPLE_MERGED;
        if (!sampleMerged) {
            if (!img->activeLayer())
            {
                KMessageBox::information(0, i18n("Cannot pick a color as no layer is active."));
                return;
            }
            if (!img->activeLayer()-> visible()) {
                KMessageBox::information(0, i18n("Cannot pick a color as the active layer is not visible."));
                return;
            }
        }

        QPoint pos = QPoint(e->pos().floorX(), e->pos().floorY());

        if (!img->bounds().contains(pos)) {
            return;
        }

        if (sampleMerged) {
            dev = img->mergedImage();
        }

        if (m_radius == 1) {
            m_pickedColor = dev->colorAt (pos.x(), pos.y());
        } else {
            // radius 2 ==> 9 pixels, 3 => 9 pixels, etc
            static int counts[] = { 0, 1, 9, 25, 45, 69, 109, 145, 193, 249 };

            KisColorSpace* cs = dev->colorSpace();
            int pixelSize = cs->pixelSize();

            quint8* data = new quint8[pixelSize];
            quint8** pixels = new quint8*[counts[m_radius]];
            quint8* weights = new quint8[counts[m_radius]];

            int i = 0;
            // dummy init
            KisHLineIteratorPixel iter = dev->createHLineIterator(0, 0, 1, false);;
            for (int y = - m_radius; y <= m_radius; y++) {
                for (int x = - m_radius; x <= m_radius; x++) {
                    if (x*x + y*y < m_radius * m_radius) {
                        iter = dev->createHLineIterator(pos.x() + x, pos.y() + y, 1, false);

                        pixels[i] = new quint8[pixelSize];
                        memcpy(pixels[i], iter.rawData(), pixelSize);

                        if (x == 0 && y == 0) {
                            // Because the sum of the weights must be 255,
                            // we cheat a bit, and weigh the center pixel differently in order
                            // to sum to 255 in total
                            // It's -(counts -1), because we'll add the center one implicitly
                            // through that calculation
                            weights[i] = 255 - (counts[m_radius]-1) * (255 / counts[m_radius]);
                        } else {
                            weights[i] = 255 / counts[m_radius];
                        }
                        i++;
                    }
                }
            }
            // Weird, I can't do that directly :/
            const quint8** cpixels = const_cast<const quint8**>(pixels);
            cs->mixColors(cpixels, weights, counts[m_radius], data);
            m_pickedColor = KisColor(data, cs);

            for (i = 0; i < counts[m_radius]; i++)
                delete[] pixels[i];
            delete[] pixels;
            delete[] data;
        }

        displayPickedColor();

        if (m_updateColor) {
            if (e->button() == Qt::LeftButton)
                m_subject->setFGColor(m_pickedColor);
            else
                m_subject->setBGColor(m_pickedColor);
        }

        if (m_addPalette) {
            // Convert to RGB to add to palette, we ought to have our own format :(
            KisPaletteEntry ent;
            ent.color = m_pickedColor.toQColor();
            // We don't ask for a name, too intrusive here

            KisPalette* palette = m_palettes.at(m_optionsWidget->cmbPalette->currentIndex());
            palette->add(ent);

            if (!palette->save()) {
                KMessageBox::error(0, i18n("Cannot write to palette file %1. Maybe it is read-only.").arg(palette->filename()), i18n("Palette"));
            }
        }
    }
}

void KisToolColorPicker::displayPickedColor()
{
    if (m_pickedColor.data() && m_optionsWidget) {

        Q3ValueVector<KisChannelInfo *> channels = m_pickedColor.colorSpace()->channels();
        m_optionsWidget->listViewChannels->clear();

        for (int i = 0; i < channels.count(); ++i) {
            QString channelValueText;

            if (m_normaliseValues) {
                channelValueText = QString(i18n("%1%")).arg(m_pickedColor.colorSpace()->normalisedChannelValueText(m_pickedColor.data(), i));
            } else {
                channelValueText = m_pickedColor.colorSpace()->channelValueText(m_pickedColor.data(), i);
            }

            QTreeWidgetItem *item = new QTreeWidgetItem(m_optionsWidget->listViewChannels);
            item->setText(0, channels[i]->name());
            item->setText(1, channelValueText);
        }
    }
}

void KisToolColorPicker::setup(KActionCollection *collection)
{
    m_action = collection->action(objectName());

    if (m_action == 0) {
        m_action = new KAction(KIcon("colorpicker"),
                               i18n("&Color Picker"),
                               collection,
                               objectName());
        m_action->setShortcut(Qt::Key_P);
        connect(m_action, SIGNAL(triggered()), this, SLOT(activate()));
        m_action->setToolTip(i18n("Color picker"));
        m_action->setActionGroup(actionGroup());
        m_ownAction = true;
    }
}

QWidget* KisToolColorPicker::createOptionWidget(QWidget* parent)
{
    m_optionsWidget = new ColorPickerOptionsWidget(parent);

    m_optionsWidget->cbUpdateCurrentColour->setChecked(m_updateColor);

    m_optionsWidget->cmbSources->setCurrentIndex(0);

    m_optionsWidget->cbNormaliseValues->setChecked(m_normaliseValues);
    m_optionsWidget->cbPalette->setChecked(m_addPalette);
    m_optionsWidget->radius->setValue(m_radius);

    m_optionsWidget->listViewChannels->setSortingEnabled(false);

    connect(m_optionsWidget->cbUpdateCurrentColour, SIGNAL(toggled(bool)), SLOT(slotSetUpdateColor(bool)));
    connect(m_optionsWidget->cbNormaliseValues, SIGNAL(toggled(bool)), SLOT(slotSetNormaliseValues(bool)));
    connect(m_optionsWidget->cbPalette, SIGNAL(toggled(bool)),
            SLOT(slotSetAddPalette(bool)));
    connect(m_optionsWidget->radius, SIGNAL(valueChanged(int)),
            SLOT(slotChangeRadius(int)));

    KisResourceServerBase* srv = KisResourceServerRegistry::instance()->get("PaletteServer");

    if (!srv) {
        return m_optionsWidget;
    }

    QList<KisResource*> palettes = srv->resources();

    foreach (KisResource *resource, palettes) {
        KisPalette* palette = dynamic_cast<KisPalette*>(resource);
        if (palette) {
            m_optionsWidget->cmbPalette->addItem(palette->name());
            m_palettes.append(palette);
        }
    }

    connect(srv, SIGNAL(resourceAdded(KisResource*)), this, SLOT(slotAddPalette(KisResource*)));

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

void KisToolColorPicker::slotSetAddPalette(bool state) {
    m_addPalette = state;
}

void KisToolColorPicker::slotChangeRadius(int value) {
    m_radius = value;
}

void KisToolColorPicker::slotAddPalette(KisResource* resource) {
    KisPalette* palette = dynamic_cast<KisPalette*>(resource);
    if (palette) {
        m_optionsWidget->cmbPalette->addItem(palette->name());
        m_palettes.append(palette);
    }
}

