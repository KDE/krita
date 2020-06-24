#ifndef KIS_MY_PAINTOP_OPTION_H
#define KIS_MY_PAINTOP_OPTION_H

#include <KisPaintopPropertiesBase.h>
#include <kis_properties_configuration.h>

const QString MYPAINT_DIAMETER = "MyPaint/diameter";

class KisMyPaintOptionProperties: public KisPaintopPropertiesBase
{
public:    
    int radius() const {
        return diameter/2;
    }

    void readOptionSettingImpl(const KisPropertiesConfiguration *settings) override {

        diameter = settings->getFloat(MYPAINT_DIAMETER);
        diameter = qRound(diameter)==0 ? 40 : diameter;
    }

    void writeOptionSettingImpl(KisPropertiesConfiguration *setting) const override {

        setting->setProperty(MYPAINT_DIAMETER, diameter);
    }


public:
    float diameter;

};

#endif // KIS_MY_PAINTOP_OPTION_H
