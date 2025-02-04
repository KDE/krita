/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisWdgFilterFastColorOverlay.h"

#include <KisViewManager.h>
#include <kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include "kis_filter_configuration.h"
#include <ui_wdgfilterfastcoloroverlay.h>
#include <KoCompositeOpRegistry.h>
#include "KisFilterFastColorOverlay.h"
#include <KisGlobalResourcesInterface.h>

KisWdgFilterFastColorOverlay::KisWdgFilterFastColorOverlay(QWidget *parent)
    : KisConfigWidget(parent)
    , m_view(nullptr)
    , m_compositeOp(KisFilterFastColorOverlay::defaultCompositeOp())
{
    m_widget.reset(new Ui_WdgFilterFastColorOverlay());
    m_widget->setupUi(this);

    m_widget->intOpacity->setRange(0, 100);
    m_widget->intOpacity->setSingleStep(1);
    m_widget->intOpacity->setPageStep(10);

    connect(m_widget->intOpacity, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_widget->cmbCompositeOp, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCompositeOpComboIndexChanged(int)));
    connect(m_widget->bnColor, SIGNAL(changed(const KoColor&)), SIGNAL(sigConfigurationItemChanged()));

    connect(m_widget->radioCompositeOpNormal, SIGNAL(toggled(bool)), this, SLOT(slotCompositeOpNormalToggled(bool)));
    connect(m_widget->radioCompositeOpTint, SIGNAL(toggled(bool)), this, SLOT(slotCompositeOpTintToggled(bool)));
    connect(m_widget->radioCompositeOpCustom, SIGNAL(toggled(bool)), this, SLOT(slotCompositeOpCustomToggled(bool)));
}

KisWdgFilterFastColorOverlay::~KisWdgFilterFastColorOverlay() = default;

void KisWdgFilterFastColorOverlay::setView(KisViewManager *view)
{
    m_view = view;

    KoCanvasResourcesInterfaceSP canvasResources = view ? view->canvasBase()->resourceManager()->canvasResourcesInterface() : nullptr;
    setCanvasResourcesInterface(canvasResources);
}

void KisWdgFilterFastColorOverlay::setConfiguration(const KisPropertiesConfigurationSP config)
{
    m_widget->intOpacity->setValue(config->getPropertyLazy("opacity", KisFilterFastColorOverlay::defaultOpacity()));
    m_widget->cmbCompositeOp->selectCompositeOp(KoID(config->getPropertyLazy("compositeopCustom", KisFilterFastColorOverlay::defaultCompositeOp())));
    m_widget->bnColor->setColor(config->getColor("color", KoColor(KisFilterFastColorOverlay::defaultColor(), KoColorSpaceRegistry::instance()->rgb8())));

    m_compositeOp = KoID(config->getPropertyLazy("compositeop", KisFilterFastColorOverlay::defaultCompositeOp()));

    if (m_compositeOp.id() == COMPOSITE_OVER) {
        m_widget->radioCompositeOpNormal->setChecked(true);
    } else if (m_compositeOp.id() == COMPOSITE_TINT) {
        m_widget->radioCompositeOpTint->setChecked(true);
    } else {
        m_widget->radioCompositeOpCustom->setChecked(true);
    }
}

KisPropertiesConfigurationSP KisWdgFilterFastColorOverlay::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration(KisFilterFastColorOverlay::id().id(), 1, KisGlobalResourcesInterface::instance());

    config->setProperty("opacity", m_widget->intOpacity->value());
    config->setProperty("compositeop", m_compositeOp.id());
    config->setProperty("compositeopCustom", m_widget->cmbCompositeOp->selectedCompositeOp().id());
    config->setProperty("color", m_widget->bnColor->color().toQColor());

    return config;
}

void KisWdgFilterFastColorOverlay::slotCompositeOpNormalToggled(bool checked)
{
    if (checked) {
        m_compositeOp = KoID(COMPOSITE_OVER);

        emit sigConfigurationItemChanged();
    }
}

void KisWdgFilterFastColorOverlay::slotCompositeOpTintToggled(bool checked)
{
    if (checked) {
        m_compositeOp = KoID(COMPOSITE_TINT);

        emit sigConfigurationItemChanged();
    }
}

void KisWdgFilterFastColorOverlay::slotCompositeOpCustomToggled(bool checked)
{
    m_widget->cmbCompositeOp->setEnabled(checked);

    if (checked) {
        slotCompositeOpComboIndexChanged(m_widget->cmbCompositeOp->currentIndex());
    }
}

void KisWdgFilterFastColorOverlay::slotCompositeOpComboIndexChanged(int index)
{
    Q_UNUSED(index);

    m_compositeOp = m_widget->cmbCompositeOp->selectedCompositeOp();

    emit sigConfigurationItemChanged();
}
