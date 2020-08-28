#include "KisWdgFilterFastColorOverlay.h"

#include "kis_filter_configuration.h"
#include <ui_wdgfilterfastcoloroverlay.h>
#include <KoCompositeOpRegistry.h>
#include "KisFilterFastColorOverlay.h"


KisWdgFilterFastColorOverlay::KisWdgFilterFastColorOverlay(QWidget *parent)
    : KisConfigWidget(parent)
{
    m_widget.reset(new Ui_WdgFilterFastColorOverlay());
    m_widget->setupUi(this);

    m_widget->intOpacity->setRange(0, 100);
    m_widget->intOpacity->setSingleStep(1);
    m_widget->intOpacity->setPageStep(10);

    connect(m_widget->intOpacity, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_widget->cmbCompositeOp, SIGNAL(currentIndexChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_widget->bnColor, SIGNAL(changed(const KoColor&)), SIGNAL(sigConfigurationItemChanged()));
}

KisWdgFilterFastColorOverlay::~KisWdgFilterFastColorOverlay()
{
}

void KisWdgFilterFastColorOverlay::setView(KisViewManager *view)
{
    m_view = view;
}

void KisWdgFilterFastColorOverlay::setConfiguration(const KisPropertiesConfigurationSP config)
{
    m_widget->intOpacity->setValue(config->getPropertyLazy("opacity", 75));
    m_widget->cmbCompositeOp->selectCompositeOp(KoID(config->getPropertyLazy("compositeop", COMPOSITE_OVER)));
    m_widget->bnColor->setColor(config->getColor("color", KoColor(QColor(185, 221, 255), KoColorSpaceRegistry::instance()->rgb8())));
}

KisPropertiesConfigurationSP KisWdgFilterFastColorOverlay::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration(KisFilterFastColorOverlay::id().id(), 1);

    config->setProperty("opacity", m_widget->intOpacity->value());
    config->setProperty("compositeop", m_widget->cmbCompositeOp->selectedCompositeOp().id());
    config->setProperty("color", m_widget->bnColor->color().toQColor());

    return config;
}
