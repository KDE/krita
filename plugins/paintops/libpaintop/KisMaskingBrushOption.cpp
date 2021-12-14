/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisMaskingBrushOption.h"

#include "kis_predefined_brush_chooser.h"
#include "kis_brush_selection_widget.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>

#include <QDomDocument>
#include "kis_brush.h"
#include "kis_image.h"
#include "kis_image_config.h"
#include "kis_brush_option.h"

#include "KisMaskingBrushOptionProperties.h"
#include <strokes/KisMaskingBrushCompositeOpFactory.h>
#include <KoCompositeOpRegistry.h>
#include <brushengine/KisPaintopSettingsIds.h>

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

        brushSizeWarningLabel = new QLabel(ui.data());
        brushSizeWarningLabel->setVisible(false);
        brushSizeWarningLabel->setWordWrap(true);

        brushChooser = new KisBrushSelectionWidget(KisImageConfig(true).maxMaskingBrushSize(), ui.data());

        QVBoxLayout *layout  = new QVBoxLayout(ui.data());
        layout->addLayout(compositeOpLayout, 0);
        layout->addWidget(brushSizeWarningLabel, 0);
        layout->addWidget(brushChooser, 1);
    }

    QScopedPointer<QWidget> ui;
    KisBrushSelectionWidget *brushChooser = 0;
    QComboBox *compositeSelector = 0;
    QLabel *brushSizeWarningLabel = 0;
    MasterBrushSizeAdapter masterBrushSizeAdapter;

    boost::optional<qreal> theoreticalMaskingBrushSize;
};

KisMaskingBrushOption::KisMaskingBrushOption(MasterBrushSizeAdapter masterBrushSizeAdapter)
    : KisPaintOpOption(i18n("Brush Tip"), KisPaintOpOption::MASKING_BRUSH, false)
    , m_d(new Private())
{
    m_d->masterBrushSizeAdapter = masterBrushSizeAdapter;

    setObjectName("KisMaskingBrushOption");
    setConfigurationPage(m_d->ui.data());

    connect(m_d->brushChooser, SIGNAL(sigBrushChanged()), SLOT(slotMaskingBrushChanged()));
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
    props.theoreticalMaskingBrushSize = m_d->theoreticalMaskingBrushSize;

    props.write(setting.data(), m_d->masterBrushSizeAdapter());
}

void KisMaskingBrushOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisMaskingBrushOptionProperties props;
    props.read(setting.data(), m_d->masterBrushSizeAdapter(), resourcesInterface(), canvasResourcesInterface());

    setChecked(props.isEnabled);

    const int selectedIndex = qMax(0, m_d->compositeSelector->findData(props.compositeOpId));
    m_d->compositeSelector->setCurrentIndex(selectedIndex);
    m_d->theoreticalMaskingBrushSize = props.theoreticalMaskingBrushSize;

    updateWarningLabelStatus();

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

void KisMaskingBrushOption::slotMaskingBrushChanged()
{
    m_d->theoreticalMaskingBrushSize = boost::none;
    updateWarningLabelStatus();
    emitSettingChanged();
}

void KisMaskingBrushOption::updateWarningLabelStatus()
{
    if (m_d->theoreticalMaskingBrushSize) {
        KisBrushSP brush = m_d->brushChooser->brush();
        const qreal realBrushSize = brush ? brush->userEffectiveSize() : 1.0;

        m_d->brushSizeWarningLabel->setVisible(true);
        m_d->brushSizeWarningLabel->setText(
            i18nc("warning about too big size of the masked brush",
                  "WARNING: Dependent size of the masked brush grew too big (%1 pixels). Its value has been cropped to %2 pixels.",
                  *m_d->theoreticalMaskingBrushSize,
                  realBrushSize));
    } else {
        m_d->brushSizeWarningLabel->setVisible(false);
    }
}

