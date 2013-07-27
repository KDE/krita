

// Qt
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QStringListModel>

#include <kis_gmic_settings_widget.h>

#include <Parameter.h>

// kritaui lib
#include <kis_slider_spin_box.h>
#include <kis_properties_configuration.h>

// debug
#include <iostream>

KisGmicSettingsWidget::KisGmicSettingsWidget(Command * command):
    m_commandDefinition(command)
    //,m_currentPreset(*command)
{
    createSettingsWidget();
}

KisGmicSettingsWidget::~KisGmicSettingsWidget()
{

}


void KisGmicSettingsWidget::createSettingsWidget()
{

    QList<Parameter *> parameters = m_commandDefinition->m_parameters;

    if (parameters.size() == 0)
    {
        std::cout << "Parameters ZERO!!! exiting...." << std::endl;
        return;
    }

    QGridLayout * gridLayout = new QGridLayout(this);

    int row = 0;
    for (int i = 0; i < parameters.size();i++)
    {
        Parameter * p = parameters.at(i);
        std::cout << "Processing: " << qPrintable(PARAMETER_NAMES[p->m_type]) << " " << qPrintable(p->toString()) << std::endl;
        switch (p->m_type)
        {

            case Parameter::INT_P:
            {
                IntParameter * intParam = static_cast<IntParameter *>(p);

                KisSliderSpinBox * spinBox = new KisSliderSpinBox;
                spinBox->setMinimum(intParam->m_minValue);
                spinBox->setMaximum(intParam->m_maxValue);
                spinBox->setValue(intParam->m_defaultValue);


                // map widget to parameter index
                m_widgetToParameterIndexMapper[spinBox] = i;

                // TODO: check if it should update preview!!!, only then recompute preview
                QObject::connect(spinBox, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationItemChanged()));
                QObject::connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(setIntValue(int)));

                gridLayout->addWidget(new QLabel(intParam->name()), row, 0);
                gridLayout->addWidget(spinBox, row, 1, 1, 3);
                row++;
                break;
            }
            case Parameter::FLOAT_P:
            {
                FloatParameter * floatParam = static_cast<FloatParameter *>(p);

                KisDoubleSliderSpinBox * spinBox = new KisDoubleSliderSpinBox;

                // TODO: check if 2 decimals is correct
                spinBox->setRange(floatParam->m_minValue, floatParam->m_maxValue, 2);
                spinBox->setValue(floatParam->m_defaultValue);

                m_widgetToParameterIndexMapper[spinBox] = i;

                QObject::connect(spinBox, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationItemChanged()));
                QObject::connect(spinBox, SIGNAL(valueChanged(qreal)), this, SLOT(setFloatValue(qreal)));

                gridLayout->addWidget(new QLabel(floatParam->name()), row, 0);
                gridLayout->addWidget(spinBox, row, 1, 1, 3);
                row++;
                break;
            }
            case Parameter::CHOICE_P:
            {
                ChoiceParameter * choiceParam = static_cast<ChoiceParameter *>(p);

                QComboBox * combo = new QComboBox;
                QStringListModel *model = new QStringListModel();
                model->setStringList(choiceParam->m_choices);
                combo->setModel(model);

                m_widgetToParameterIndexMapper[combo] = i;

                QObject::connect(combo, SIGNAL(currentIndexChanged(QString)), this, SIGNAL(sigConfigurationItemChanged()));
                QObject::connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(setChoiceIndex(int)));

                gridLayout->addWidget(combo, row, 1, 1, 2);
                row++;
                break;
            }
            case Parameter::NOTE_P:
            {
                NoteParameter * noteParam = static_cast<NoteParameter *>(p);
                QLabel * label = new QLabel;
                label->setText(noteParam->m_label);

                gridLayout->addWidget(label, row, 0, 1, 3);
                row++;
                break;
            }


            default:{
                std::cout << "Ignoring : " << qPrintable(PARAMETER_NAMES[p->m_type]) << std::endl;
                break;
            }

        }

    }

    std::cout << "Setting layout" << std::endl;
    setLayout(gridLayout);
}

Command* KisGmicSettingsWidget::currentCommandSettings()
{
    return m_commandDefinition;
}

void KisGmicSettingsWidget::setIntValue(int value)
{
    Parameter * p = parameter(sender());
    if (p->m_type != Parameter::INT_P)
    {
        return;
    }
    IntParameter * intParam = static_cast<IntParameter *>(p);
    intParam->m_value = value;
}

void KisGmicSettingsWidget::setFloatValue(qreal value)
{
    Parameter * p = parameter(sender());
    if (!p)
    {
        return;
    }

    if (p->m_type != Parameter::FLOAT_P)
    {
        return;
    }
    FloatParameter * floatParam = static_cast<FloatParameter *>(p);
    floatParam->m_value = value;
}

void KisGmicSettingsWidget::setChoiceIndex(int index)
{
    qDebug() << "setting choice param: failed?";
    Parameter * p = parameter(sender());
    if (!p)
    {
        return;
    }

    if (p->m_type != Parameter::CHOICE_P)
    {
        return;
    }
    qDebug() << "NO!";
    qDebug() << "Setting " << index;

    ChoiceParameter * choiceParam = static_cast<ChoiceParameter *>(p);
    choiceParam->m_value = index;
}

Parameter * KisGmicSettingsWidget::parameter(QObject* widget)
{
    if (!widget)
    {
        return 0;
    }

    if (!m_widgetToParameterIndexMapper.contains(widget))
    {
        return 0;
    }

    int index = m_widgetToParameterIndexMapper[widget];
    Parameter * p = m_commandDefinition->m_parameters.at(index);
    return p;
}
