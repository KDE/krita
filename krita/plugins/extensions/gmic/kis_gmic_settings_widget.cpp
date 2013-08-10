

// Qt
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QStringListModel>
#include <QCheckBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QFileDialog>

#include <kis_gmic_settings_widget.h>

#include <Parameter.h>

// kritaui lib
#include <kis_slider_spin_box.h>
#include <kis_properties_configuration.h>

//
#include <kurlrequester.h>

// debug
#include <iostream>
#include <kcolorbutton.h>
#include <kfiledialog.h>

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

                gridLayout->addWidget(new QLabel(choiceParam->name()), row, 0);
                gridLayout->addWidget(combo, row, 1, 1, 2);
                row++;
                break;
            }
            case Parameter::NOTE_P:
            {
                NoteParameter * noteParam = static_cast<NoteParameter *>(p);
                QLabel * label = new QLabel;
                label->setText(noteParam->m_label);
                label->setWordWrap(true);

                gridLayout->addWidget(label, row, 0, 1, 3);
                row++;
                break;
            }
            case Parameter::LINK_P:
            {
                LinkParameter * linkParam = static_cast<LinkParameter *>(p);
                QLabel * label = new QLabel;
                label->setText(linkParam->m_link);
                label->setWordWrap(true);
                label->setOpenExternalLinks(true);

                gridLayout->addWidget(label, row, 0, 1, 3);
                row++;
                break;
            }
            case Parameter::BOOL_P:
            {
                BoolParameter * boolParam = static_cast<BoolParameter *>(p);
                QCheckBox *checkBox = new QCheckBox(boolParam->m_name);
                checkBox->setChecked(boolParam->m_value);

                m_widgetToParameterIndexMapper[checkBox] = i;

                QObject::connect(checkBox, SIGNAL(stateChanged(int)), this, SIGNAL(sigConfigurationItemChanged()));
                QObject::connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(setBoolValue(bool)));

                gridLayout->addWidget(checkBox, row, 0, 1, 2);
                row++;
                break;
            }

            case Parameter::COLOR_P:
            {
                ColorParameter * colorParam = static_cast<ColorParameter *>(p);
                KColorButton *colorButton = new KColorButton(colorParam->m_value);


                m_widgetToParameterIndexMapper[colorParam] = i;

                QObject::connect(colorButton, SIGNAL(changed(QColor)), this, SIGNAL(sigConfigurationItemChanged()));
                QObject::connect(colorButton, SIGNAL(changed(QColor)), this, SLOT(setColorValue(QColor)));

                gridLayout->addWidget(new QLabel(colorParam->name()), row, 0);
                gridLayout->addWidget(colorButton, row, 1, 1, 2);
                row++;
                break;
            }
            case Parameter::FOLDER_P:
            {
                FolderParameter * folderParam = static_cast<FolderParameter *>(p);
                KUrlRequester * urlRequester = new KUrlRequester;
                urlRequester->fileDialog()->setMode(KFile::Directory);

                m_widgetToParameterIndexMapper[ urlRequester ] = i;

                QObject::connect(urlRequester, SIGNAL(urlSelected(KUrl)), this, SIGNAL(sigConfigurationItemChanged()));
                QObject::connect(urlRequester, SIGNAL(urlSelected(KUrl)), this, SLOT(setFolderPathValue(KUrl)));

                gridLayout->addWidget(new QLabel(folderParam->name()), row, 0);
                gridLayout->addWidget(urlRequester, row, 1, 1, 3);
                row++;
                break;
            }
            case Parameter::FILE_P:
            {
                FileParameter * fileParam = static_cast<FileParameter *>(p);
                KUrlRequester * urlRequester = new KUrlRequester;
                urlRequester->fileDialog()->setMode(KFile::File);

                m_widgetToParameterIndexMapper[ urlRequester ] = i;

                QObject::connect(urlRequester, SIGNAL(urlSelected(KUrl)), this, SIGNAL(sigConfigurationItemChanged()));
                QObject::connect(urlRequester, SIGNAL(urlSelected(KUrl)), this, SLOT(setFilePathValue(KUrl)));

                gridLayout->addWidget(new QLabel(fileParam->name()), row, 0);
                gridLayout->addWidget(urlRequester, row, 1, 1, 3);
                row++;
                break;
            }

            case Parameter::TEXT_P:
            {
                TextParameter * textParam = static_cast<TextParameter *>(p);
                QPushButton * updateBtn = new QPushButton("Update");

                if (!textParam->m_multiline)
                {
                    QLineEdit * lineEdit = new QLineEdit;
                    lineEdit->setText(textParam->m_value);

                    m_widgetToParameterIndexMapper[lineEdit] = i;

                    QObject::connect(updateBtn, SIGNAL(clicked(bool)), lineEdit, SIGNAL(editingFinished()));
                    QObject::connect(lineEdit, SIGNAL(editingFinished()), this, SIGNAL(sigConfigurationItemChanged()));
                    QObject::connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(setTextValue()));
                    gridLayout->addWidget(new QLabel(textParam->name()), row, 0);
                    gridLayout->addWidget(lineEdit, row, 1, 1, 1);
                    gridLayout->addWidget(updateBtn, row, 3, 1, 1);
                    row++;

                }
                else
                {
                    QTextEdit * multiLineEdit = new QTextEdit;
                    multiLineEdit->setText(textParam->m_value);

                    m_widgetToParameterIndexMapper[multiLineEdit] = i;

                    QObject::connect(updateBtn, SIGNAL(clicked(bool)), multiLineEdit, SIGNAL(editingFinished()));
                    QObject::connect(multiLineEdit, SIGNAL(editingFinished()), this, SIGNAL(sigConfigurationItemChanged()));
                    QObject::connect(multiLineEdit, SIGNAL(editingFinished()), this, SLOT(setTextValue()));

                    gridLayout->addWidget(new QLabel(textParam->name()), row, 0);
                    gridLayout->addWidget(updateBtn, row, 1);
                    row++;
                    gridLayout->addWidget(multiLineEdit, row, 0, 1, 4, Qt::AlignCenter);
                    row++;
                }
                break;
            }

            default:{
                std::cout << "IGNORING : " << qPrintable(PARAMETER_NAMES[p->m_type]) << std::endl;
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

void KisGmicSettingsWidget::setBoolValue(bool value)
{
    Parameter * p = parameter(sender());
    if (!p)
    {
        return;
    }

    if (p->m_type != Parameter::BOOL_P)
    {
        return;
    }
    BoolParameter * boolParam = static_cast<BoolParameter *>(p);
    boolParam->m_value = value;
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


void KisGmicSettingsWidget::setColorValue(const QColor& color)
{
    Parameter * p = parameter(sender());
    if (!p)
    {
        return;
    }

    if (p->m_type != Parameter::COLOR_P)
    {
        return;
    }

    ColorParameter * colorParam = static_cast<ColorParameter *>(p);
    colorParam->m_value = color;
}

void KisGmicSettingsWidget::setTextValue()
{
    Parameter * p = parameter(sender());
    if (!p)
    {
        return;
    }

    if (p->m_type != Parameter::TEXT_P)
    {
        return;
    }

    TextParameter * textParam = static_cast<TextParameter *>(p);
    QString result = "INVALID";
    if (textParam->m_multiline)
    {
        const QTextEdit * textEdit = qobject_cast< const QTextEdit *>(sender());
        if (textEdit)
        {
            result = textEdit->toPlainText();
        }
    }
    else
    {
        const QLineEdit * lineEdit = qobject_cast< const QLineEdit *>(sender());
        if (lineEdit)
        {
            result = lineEdit->text();
        }
    }

    textParam->m_value = result;
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


void KisGmicSettingsWidget::setFolderPathValue(const KUrl& kurl)
{
    Parameter * p = parameter(sender());
    if (!p)
    {
        return;
    }

    if (p->m_type != Parameter::FOLDER_P)
    {
        return;
    }

    FolderParameter * folderParam = static_cast<FolderParameter  *>(p);
    folderParam->m_folderPath = kurl.path();
}

void KisGmicSettingsWidget::setFilePathValue(const KUrl& kurl)
{
    Parameter * p = parameter(sender());
    if (!p)
    {
        return;
    }

    if (p->m_type != Parameter::FILE_P)
    {
        return;
    }

    FileParameter * fileParam = static_cast<FileParameter *>(p);
    fileParam->m_filePath = kurl.path();
}
