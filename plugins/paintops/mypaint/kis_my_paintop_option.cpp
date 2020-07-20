#include "kis_my_paintop_option.h"
#include <QWidget>
#include "ui_wdgmypaintoptions.h"

class KisMyPaintOpOptionsWidget: public QWidget, public Ui::WdgMyPaintOptions
{
public:
    KisMyPaintOpOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {

        setupUi(this);
    }
};

KisMyPaintOpOption::KisMyPaintOpOption()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    setObjectName("KisMyPaintOpOption");

    m_checkable = false;
    m_options = new KisMyPaintOpOptionsWidget();    

    m_options->radiusSPBox->setRange(0.0, 7.0, 2);
    m_options->radiusSPBox->setValue(radius());
    //m_options->radiusSPBox->setSuffix(i18n(" px"));

    m_options->hardnessSPBox->setRange(0.0, 1.0, 2);
    m_options->hardnessSPBox->setValue(hardness());

    m_options->opacitySPBox->setRange(0.0, 1.0, 2);
    m_options->opacitySPBox->setValue(opacity());

    m_options->eraserBox->setChecked(eraser());

//    connect(m_options->radiusSPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
//    connect(m_options->hardnessSPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
//    connect(m_options->opacitySPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));

    connect(m_options->eraserBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));

    setConfigurationPage(m_options);
}

KisMyPaintOpOption::~KisMyPaintOpOption()
{
    delete m_options;
}

void KisMyPaintOpOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{        
    KisMyPaintOptionProperties op;

    op.diameter = 2*exp(m_options->radiusSPBox->value());
    op.hardness = m_options->hardnessSPBox->value();
    op.opacity = m_options->opacitySPBox->value();
    op.eraser = m_options->eraserBox->isChecked();
    op.json = this->json;

    op.writeOptionSetting(setting);
}

void KisMyPaintOpOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{        
    KisMyPaintOptionProperties op;
    op.readOptionSetting(setting);

    if(!setting->getProperty(MYPAINT_JSON).isNull())
        this->json = setting->getProperty(MYPAINT_JSON).toByteArray();

    m_options->radiusSPBox->setValue(op.radius());
    m_options->hardnessSPBox->setValue(op.hardness);
    m_options->opacitySPBox->setValue(op.opacity);
    m_options->eraserBox->setChecked(op.eraser);
}

void KisMyPaintOpOption::refresh() {

    emitSettingChanged();
}

void KisMyPaintOpOption::setEraser(bool isEraser) const
{
    m_options->eraserBox->setChecked(isEraser);
}

bool KisMyPaintOpOption::eraser() const
{
    return m_options->eraserBox->isChecked();
}

void KisMyPaintOpOption::setOpacity(int opacity) const
{
    m_options->opacitySPBox->setValue(opacity);
}

int KisMyPaintOpOption::opacity() const
{
    return m_options->opacitySPBox->value();
}


void KisMyPaintOpOption::setHardness(int hardness) const
{
    m_options->hardnessSPBox->setValue(hardness);
}

int KisMyPaintOpOption::hardness() const
{
    return m_options->hardnessSPBox->value();
}

void KisMyPaintOpOption::setRadius(int radius) const
{
    m_options->radiusSPBox->setValue(radius);
}

int KisMyPaintOpOption::radius() const
{
    return m_options->radiusSPBox->value();
}

KisDoubleSliderSpinBox* KisMyPaintOpOption::radiusSlider() {

    return m_options->radiusSPBox;
}

KisDoubleSliderSpinBox* KisMyPaintOpOption::hardnessSlider() {

    return m_options->hardnessSPBox;
}

KisDoubleSliderSpinBox* KisMyPaintOpOption::opacitySlider() {

    return m_options->opacitySPBox;
}
