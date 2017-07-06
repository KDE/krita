#include "kis_wdg_edge_detection.h"

#include <filter/kis_filter_configuration.h>

KisWdgEdgeDetection::KisWdgEdgeDetection(QWidget *parent) :
    KisConfigWidget(parent),
    ui(new Ui_WidgetEdgeDetection)
{
    ui->setupUi(this);
}

KisWdgEdgeDetection::~KisWdgEdgeDetection()
{
    delete ui;
}

KisPropertiesConfigurationSP KisWdgEdgeDetection::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("edge detection", 1);
    config->setProperty("horizRadius", ui->spnHorizontalRadius->value());
    config->setProperty("vertRadius", ui->spnVerticalRadius->value());
    config->setProperty("type", "prewit");
    config->setProperty("lockAspect", true);

    return config;
}

void KisWdgEdgeDetection::setConfiguration(const KisPropertiesConfigurationSP config)
{
    ui->spnHorizontalRadius->setValue(config->getFloat("horizRadius", 1.0));
    ui->spnVerticalRadius->setValue(config->getFloat("vertRadius", 1.0));
}
