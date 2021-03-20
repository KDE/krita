/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

        compositeSelector = new QComboBox(ui.data());

        const QStringList supportedComposites = KisMaskingBrushCompositeOpFactory::supportedCompositeOpIds();
        Q_FOREACH (const QString &id, supportedComposites) {
            const QString name = KoCompositeOpRegistry::instance().getKoID(id).name();
            compositeSelector->addItem(name, id);
        }
        compositeSelector->setCurrentIndex(0);

        QHBoxLayout *compositeOpLayout = new QHBoxLayout();
        compositeOpLayout->addWidget(new QLabel(i18n("Blending Mode:")), 0);
        compositeOpLayout->addWidget(compositeSelector, 1);

        brushChooser = new KisBrushSelectionWidget(ui.data());

        QVBoxLayout *layout  = new QVBoxLayout(ui.data());
        layout->addLayout(compositeOpLayout, 0);
        layout->addWidget(brushChooser, 1);
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
    props.read(setting.data(), m_d->masterBrushSizeAdapter(), resourcesInterface(), canvasResourcesInterface());

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

