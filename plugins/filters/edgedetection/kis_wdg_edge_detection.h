#ifndef KIS_WDG_EDGE_DETECTION_H
#define KIS_WDG_EDGE_DETECTION_H

#include <QWidget>
#include <kis_config_widget.h>
#include "ui_wdg_edge_detection.h"

class Ui_WidgetEdgeDetection;


class KisWdgEdgeDetection : public KisConfigWidget
{
    Q_OBJECT

public:
    explicit KisWdgEdgeDetection(QWidget *parent);
    ~KisWdgEdgeDetection();

    KisPropertiesConfigurationSP configuration() const override;
    void setConfiguration(const KisPropertiesConfigurationSP config) override;

private:
    Ui_WidgetEdgeDetection *ui;
};

#endif // KIS_WDG_EDGE_DETECTION_H
