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
#include "kis_brush_selection_widget.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>

#include <QDomDocument>
#include "kis_brush.h"
#include "kis_image.h"
#include "kis_brush_option.h"

#include "KisMaskingBrushOptionProperties.h"
#include <strokes/KisMaskingBrushCompositeOpFactory.h>
#include <KoCompositeOpRegistry.h>

struct KisMaskingBrushOption::Private
{
    Private()
        : ui(new QWidget())
    {
        QVBoxLayout *l  = new QVBoxLayout();

        QHBoxLayout *compositeOpLayout = new QHBoxLayout();
        compositeSelector = new QComboBox(ui.data());

        const QStringList supportedComposites = KisMaskingBrushCompositeOpFactory::supportedCompositeOpIds();
        Q_FOREACH (const QString &id, supportedComposites) {
            const QString name = KoCompositeOpRegistry::instance().getKoID(id).name();
            compositeSelector->addItem(name, id);
        }
        compositeSelector->setCurrentIndex(0);

        compositeOpLayout->addWidget(new QLabel(i18n("Blending Mode:")), 0);
        compositeOpLayout->addWidget(compositeSelector, 1);

        l->addLayout(compositeOpLayout, 0);


        brushChooser = new KisBrushSelectionWidget(ui.data());
        l->addWidget(brushChooser, 1);
        ui->setLayout(l);
    }

    QScopedPointer<QWidget> ui;
    KisBrushSelectionWidget *brushChooser = 0;
    QComboBox *compositeSelector = 0;
    MasterBrushSizeAdapter masterBrushSizeAdapter;
};

KisMaskingBrushOption::KisMaskingBrushOption(MasterBrushSizeAdapter masterBrushSizeAdapter)
    : KisPaintOpOption(KisPaintOpOption::MASKING_BRUSH, false),
      m_d(new Private())
{
    m_d->masterBrushSizeAdapter = masterBrushSizeAdapter;

    setObjectName("KisMaskingBrushOption");
    setConfigurationPage(m_d->ui.data());

    connect(m_d->brushChooser, SIGNAL(sigBrushChanged()), SLOT(emitSettingChanged()));
    connect(m_d->compositeSelector, SIGNAL(currentIndexChanged(int)), SLOT(emitSettingChanged()));
}

KisMaskingBrushOption::~KisMaskingBrushOption()
{

}

void KisMaskingBrushOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisMaskingBrushOptionProperties props;

    props.isEnabled = isChecked();
    props.brush = m_d->brushChooser->brush();
    props.compositeOpId = m_d->compositeSelector->currentData().toString();

    props.write(setting.data(), m_d->masterBrushSizeAdapter());
}

void KisMaskingBrushOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisMaskingBrushOptionProperties props;
    props.read(setting.data(), m_d->masterBrushSizeAdapter());

    setChecked(props.isEnabled);

    const int selectedIndex = qMax(0, m_d->compositeSelector->findData(props.compositeOpId));
    m_d->compositeSelector->setCurrentIndex(selectedIndex);

    if (props.brush) {
        m_d->brushChooser->setCurrentBrush(props.brush);
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

