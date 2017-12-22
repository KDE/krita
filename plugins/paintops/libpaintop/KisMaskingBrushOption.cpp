/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisMaskingBrushOption.h"

#include "kis_brush_chooser.h"

#include <QWidget>
#include <QVBoxLayout>

#include <QDomDocument>
#include "kis_brush.h"
#include "kis_image.h"
#include "kis_brush_option.h"

#include "KisMaskingBrushOptionProperties.h"


struct KisMaskingBrushOption::Private
{
    Private()
        : ui(new QWidget())
    {
        QVBoxLayout *l  = new QVBoxLayout(ui.data());

        brushChooser = new KisPredefinedBrushChooser(ui.data(), "MaskingBrushChooser");
        l->addWidget(brushChooser);
        ui->setLayout(l);
    }

    QScopedPointer<QWidget> ui;
    KisPredefinedBrushChooser *brushChooser = 0;
};

KisMaskingBrushOption::KisMaskingBrushOption()
    : KisPaintOpOption(KisPaintOpOption::MASKING_BRUSH, false),
      m_d(new Private())
{
    setObjectName("KisMaskingBrushOption");
    setConfigurationPage(m_d->ui.data());

    connect(m_d->brushChooser, SIGNAL(sigBrushChanged()), SLOT(emitSettingChanged()));
}

KisMaskingBrushOption::~KisMaskingBrushOption()
{

}

void KisMaskingBrushOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisMaskingBrushOptionProperties props;

    props.isEnabled = isChecked();
    props.brush = m_d->brushChooser->brush();

    props.write(setting.data());
}

void KisMaskingBrushOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisMaskingBrushOptionProperties props;
    props.read(setting.data());

    setChecked(props.isEnabled);
    if (props.brush) {
        m_d->brushChooser->setBrush(props.brush);
    }
}

void KisMaskingBrushOption::setImage(KisImageWSP image)
{
    m_d->brushChooser->setImage(image);
}

void KisMaskingBrushOption::lodLimitations(KisPaintopLodLimitations *l) const
{
    KisBrushSP brush = m_d->brushChooser->brush();

    if (brush) {
        brush->lodLimitations(l);
    }
}

