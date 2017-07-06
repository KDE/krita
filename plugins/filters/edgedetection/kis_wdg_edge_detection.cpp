#include "kis_wdg_edge_detection.h"

#include <filter/kis_filter_configuration.h>
#include <QComboBox>

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
    if (ui->cmbType->currentIndex() == 0) {
        config->setProperty("type", "prewit");
    } else if (ui->cmbType->currentIndex() == 1) {
        config->setProperty("type", "sobolvector");
    } else if (ui->cmbType->currentIndex() == 2) {
        config->setProperty("type", "simple");
    }
    config->setProperty("lockAspect", true);

    return config;
}

void KisWdgEdgeDetection::setConfiguration(const KisPropertiesConfigurationSP config)
{
    ui->spnHorizontalRadius->setValue(config->getFloat("horizRadius", 1.0));
    ui->spnVerticalRadius->setValue(config->getFloat("vertRadius", 1.0));
}
