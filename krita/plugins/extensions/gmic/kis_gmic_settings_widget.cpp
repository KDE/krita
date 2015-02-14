

// Qt
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QStringListModel>
#include <QCheckBox>
#include <QLineEdit>
#include <QTextEdit>

#include <kis_gmic_settings_widget.h>
#include <kis_debug.h>

#include <Parameter.h>

// kritaui lib
#include <kis_slider_spin_box.h>
#include <kis_properties_configuration.h>

//
#include "widgets/kis_url_requester.h"
#include <kdeversion.h>
#include <kcolorbutton.h>
#include <klocalizedstring.h>
#include <kseparator.h>

#include <KoFileDialog.h>
#include <kfiledialog.h> // For kisurlrequester...

KisGmicSettingsWidget::KisGmicSettingsWidget(Command * command)
    :   KisConfigWidget(0, 0, 250),
        m_commandDefinition(command)
{
    createSettingsWidget(CreateRole);
}

KisGmicSettingsWidget::~KisGmicSettingsWidget()
{
    m_widgetToParameterIndexMapper.clear();
    m_parameterToWidgetMapper.clear();
}


void KisGmicSettingsWidget::createSettingsWidget(ROLE role)
{

    QList<Parameter *> parameters = m_commandDefinition->m_parameters;

    if (parameters.size() == 0)
    {
        dbgPlugins << "No parameters!";
        return;
    }

    QGridLayout * gridLayout(0);
    if (role == CreateRole)
    {
        gridLayout = new QGridLayout(this);
    }

    int row = 0;
    for (int i = 0; i < parameters.size();i++)
    {
        Parameter * p = parameters.at(i);
        dbgPlugins << "Processing: " << qPrintable(PARAMETER_NAMES[p->m_type]) << " " << qPrintable(p->toString());
        switch (p->m_type)
        {
            case Parameter::INT_P:
            {
                IntParameter * intParam = static_cast<IntParameter *>(p);
                KisSliderSpinBox * spinBox(0);
                if (role == CreateRole)
                {
                    spinBox = new KisSliderSpinBox;
                    spinBox->setMinimum(intParam->m_minValue);
                    spinBox->setMaximum(intParam->m_maxValue);

                    // map widget to parameter index
                    m_widgetToParameterIndexMapper[spinBox] = i;
                    mapParameterWidget(intParam, spinBox);

                    // TODO: check if it should update preview!!!, only then recompute preview
                    connect(spinBox, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationItemChanged()));
                    connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(setIntValue(int)));

                    gridLayout->addWidget(new QLabel(intParam->name()), row, 0);
                    gridLayout->addWidget(spinBox, row, 1, 1, 3);

                    row++;
                }
                else if (role == LoadRole)
                {
                    spinBox = qobject_cast<KisSliderSpinBox *>(widget(intParam));
                }

                if (spinBox)
                {
                    spinBox->setValue(intParam->m_value);
                }
                else
                {
                    dbgPlugins << "Widget not available; Role: " << role << p->m_type;
                }
                break;
            }
            case Parameter::FLOAT_P:
            {
                FloatParameter * floatParam = static_cast<FloatParameter *>(p);

                KisDoubleSliderSpinBox * spinBox(0);
                if (role == CreateRole)
                {
                    spinBox = new KisDoubleSliderSpinBox;
                    // TODO: check if 2 decimals is correct
                    spinBox->setRange(floatParam->m_minValue, floatParam->m_maxValue, 2);

                    m_widgetToParameterIndexMapper[spinBox] = i;
                    mapParameterWidget(floatParam, spinBox);

                    connect(spinBox, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationItemChanged()));
                    connect(spinBox, SIGNAL(valueChanged(qreal)), this, SLOT(setFloatValue(qreal)));

                    gridLayout->addWidget(new QLabel(floatParam->name()), row, 0);
                    gridLayout->addWidget(spinBox, row, 1, 1, 3);
                    row++;
                }
                else if (role == LoadRole)
                {
                    spinBox = qobject_cast<KisDoubleSliderSpinBox *>(widget(floatParam));
                }

                if (spinBox)
                {
                    spinBox->setValue(floatParam->m_value);
                }
                else
                {
                    dbgPlugins << "Widget not available; Role: " << role << " TYPE " <<p->m_type;
                }
                break;
            }
            case Parameter::CHOICE_P:
            {
                ChoiceParameter * choiceParam = static_cast<ChoiceParameter *>(p);

                QComboBox * combo(0);
                if (role == CreateRole)
                {
                    combo = new QComboBox;
                    QStringListModel *model = new QStringListModel();
                    model->setStringList(choiceParam->m_choices);
                    combo->setModel(model);

                    m_widgetToParameterIndexMapper[combo] = i;
                    mapParameterWidget(choiceParam, combo);

                    connect(combo, SIGNAL(currentIndexChanged(QString)), this, SIGNAL(sigConfigurationItemChanged()));
                    connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(setChoiceIndex(int)));

                    gridLayout->addWidget(new QLabel(choiceParam->name()), row, 0);
                    gridLayout->addWidget(combo, row, 1, 1, 3);
                    row++;
                }
                else if (role == LoadRole)
                {
                    combo = qobject_cast<QComboBox *>(widget(choiceParam));
                }

                if (combo)
                {
                    combo->setCurrentIndex(choiceParam->m_value);
                }
                else
                {
                    dbgPlugins << "Widget not available; Role: " << role << " TYPE " <<p->m_type;
                }
                break;
            }
            case Parameter::NOTE_P:
            {
                if (role == CreateRole)
                {
                    NoteParameter * noteParam = static_cast<NoteParameter *>(p);
                    QLabel * label = new QLabel;
                    QString labelTxt = noteParam->m_label.replace(QString("\\n"), QString("<br />"));
                    label->setText(labelTxt);
                    label->setWordWrap(true);

                    mapParameterWidget(noteParam, label);

                    gridLayout->addWidget(label, row, 0, 1, 3);
                    row++;
                }
                break;
            }
            case Parameter::LINK_P:
            {
                if (role == CreateRole)
                {
                    LinkParameter * linkParam = static_cast<LinkParameter *>(p);
                    QLabel * label = new QLabel;
                    label->setText(linkParam->m_link);
                    label->setWordWrap(true);
                    label->setOpenExternalLinks(true);

                    mapParameterWidget(linkParam, label);

                    gridLayout->addWidget(label, row, 0, 1, 3);
                    row++;
                }
                break;
            }
            case Parameter::SEPARATOR_P:
            {
                if (role == CreateRole)
                {
                    SeparatorParameter * linkParam = static_cast<SeparatorParameter *>(p);
                    KSeparator * kseparator = new KSeparator(Qt::Horizontal);
                    mapParameterWidget(linkParam, kseparator);
                    gridLayout->addWidget(kseparator, row, 0, 1, 4);
                    row++;
                }
                break;
            }
            case Parameter::BOOL_P:
            {
                BoolParameter * boolParam = static_cast<BoolParameter *>(p);
                QCheckBox *checkBox(0);
                if (role == CreateRole)
                {
                    checkBox = new QCheckBox(boolParam->m_name);

                    m_widgetToParameterIndexMapper[checkBox] = i;
                    mapParameterWidget(boolParam, checkBox);

                    connect(checkBox, SIGNAL(stateChanged(int)), this, SIGNAL(sigConfigurationItemChanged()));
                    connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(setBoolValue(bool)));

                    gridLayout->addWidget(checkBox, row, 0, 1, 2);
                    row++;
                }
                else if (role == LoadRole)
                {
                    checkBox = qobject_cast<QCheckBox *>(widget(boolParam));
                }

                if (checkBox)
                {
                    checkBox->setChecked(boolParam->m_value);
                }
                else
                {
                    dbgPlugins << "Widget not available; Role: " << role << " TYPE " <<p->m_type;
                }
                break;
            }

            case Parameter::COLOR_P:
            {
                ColorParameter * colorParam = static_cast<ColorParameter *>(p);
                KColorButton *colorButton(0);
                if (role == CreateRole)
                {
                    colorButton = new KColorButton;
#if KDE_IS_VERSION(4,5,0)
                    colorButton->setAlphaChannelEnabled(colorParam->m_hasAlpha);
#endif
                    m_widgetToParameterIndexMapper[colorParam] = i;
                    mapParameterWidget(colorParam, colorButton);

                    connect(colorButton, SIGNAL(changed(QColor)), this, SIGNAL(sigConfigurationItemChanged()));
                    connect(colorButton, SIGNAL(changed(QColor)), this, SLOT(setColorValue(QColor)));

                    gridLayout->addWidget(new QLabel(colorParam->name()), row, 0);
                    gridLayout->addWidget(colorButton, row, 1, 1, 3);
                    row++;
                }
                else if (role == LoadRole)
                {
                    colorButton = qobject_cast<KColorButton *>(widget(colorParam));
                }

                if (colorButton)
                {
                    colorButton->setColor(colorParam->m_value);
                }
                else
                {
                    dbgPlugins << "Widget not available; Role: " << role << " TYPE " <<p->m_type;
                }
                break;
            }
            case Parameter::FOLDER_P:
            {
                FolderParameter * folderParam = static_cast<FolderParameter *>(p);
                KisUrlRequester * urlRequester(0);
                if (role == CreateRole)
                {
                    urlRequester = new KisUrlRequester;
                    urlRequester->setMode(KoFileDialog::OpenDirectory);

                    m_widgetToParameterIndexMapper[ urlRequester ] = i;
                    mapParameterWidget(folderParam, urlRequester);

                    connect(urlRequester, SIGNAL(urlSelected(KUrl)), this, SIGNAL(sigConfigurationItemChanged()));
                    connect(urlRequester, SIGNAL(urlSelected(KUrl)), this, SLOT(setFolderPathValue(KUrl)));

                    gridLayout->addWidget(new QLabel(folderParam->name()), row, 0);
                    gridLayout->addWidget(urlRequester, row, 1, 1, 3);
                    row++;
                }
                else if (role == LoadRole)
                {
                    urlRequester = qobject_cast<KisUrlRequester *>(widget(folderParam));
                }

                if (urlRequester)
                {
                    urlRequester->setUrl(KUrl(folderParam->toUiValue()));
                }
                else
                {
                    dbgPlugins << "Widget not available; Role: " << role << " TYPE " <<p->m_type;
                }
                break;
            }
            case Parameter::FILE_P:
            {
                FileParameter * fileParam = static_cast<FileParameter *>(p);
                KisUrlRequester * urlRequester(0);
                if (role == CreateRole)
                {
                    urlRequester = new KisUrlRequester;
                    urlRequester->setMode(KoFileDialog::OpenFile);

                    m_widgetToParameterIndexMapper[ urlRequester ] = i;
                    mapParameterWidget(fileParam, urlRequester);

                    connect(urlRequester, SIGNAL(urlSelected(KUrl)), this, SIGNAL(sigConfigurationItemChanged()));
                    connect(urlRequester, SIGNAL(urlSelected(KUrl)), this, SLOT(setFilePathValue(KUrl)));

                    gridLayout->addWidget(new QLabel(fileParam->name()), row, 0);
                    gridLayout->addWidget(urlRequester, row, 1, 1, 3);
                    row++;
                }
                else if (role == LoadRole)
                {
                    urlRequester = qobject_cast<KisUrlRequester *>(widget(fileParam));
                }

                if (urlRequester)
                {
                    urlRequester->setUrl(KUrl(fileParam->toUiValue()));
                }
                else
                {
                    dbgPlugins << "Widget not available; Role: " << role << " TYPE " <<p->m_type;
                }
                break;
            }

            case Parameter::TEXT_P:
            {
                TextParameter * textParam = static_cast<TextParameter *>(p);
                QPushButton * updateBtn(0);
                if (role == CreateRole)
                {
                    updateBtn = new QPushButton(i18n("Update"));
                }

                if (!textParam->m_multiline)
                {
                    QLineEdit * lineEdit(0);
                    if (role == CreateRole)
                    {
                        lineEdit = new QLineEdit;
                        lineEdit->setText(textParam->toUiValue());

                        m_widgetToParameterIndexMapper[lineEdit] = i;
                        mapParameterWidget(textParam, lineEdit);

                        connect(updateBtn, SIGNAL(clicked(bool)), lineEdit, SIGNAL(editingFinished()));
                        connect(lineEdit, SIGNAL(editingFinished()), this, SIGNAL(sigConfigurationItemChanged()));
                        connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(setTextValue()));
                        gridLayout->addWidget(new QLabel(textParam->name()), row, 0);
                        gridLayout->addWidget(lineEdit, row, 1, 1, 1);
                        gridLayout->addWidget(updateBtn, row, 3, 1, 1);
                        row++;
                    }
                    else if (role == LoadRole)
                    {
                        lineEdit = qobject_cast<QLineEdit*>(widget(textParam));
                    }

                    if (lineEdit)
                    {
                        lineEdit->setText(textParam->toUiValue());
                    }
                    else
                    {
                        dbgPlugins << "Widget not available; Role: " << role << " TYPE " <<p->m_type;
                    }


                }
                else /*if (textParam->m_multiline)*/
                {
                    QTextEdit * multiLineEdit(0);
                    if (role == CreateRole)
                    {
                        multiLineEdit = new QTextEdit;
                        multiLineEdit->setText(textParam->toUiValue());

                        m_widgetToParameterIndexMapper[multiLineEdit] = i;
                        mapParameterWidget(textParam, multiLineEdit);

                        connect(updateBtn, SIGNAL(clicked(bool)), multiLineEdit, SIGNAL(editingFinished()));
                        connect(multiLineEdit, SIGNAL(editingFinished()), this, SIGNAL(sigConfigurationItemChanged()));
                        connect(multiLineEdit, SIGNAL(editingFinished()), this, SLOT(setTextValue()));

                        gridLayout->addWidget(new QLabel(textParam->name()), row, 0);
                        gridLayout->addWidget(updateBtn, row, 1);
                        row++;
                        gridLayout->addWidget(multiLineEdit, row, 0, 1, 4, Qt::AlignCenter);
                        row++;
                    }
                    else if (role == LoadRole)
                    {
                        multiLineEdit = qobject_cast<QTextEdit*>(widget(textParam));
                    }

                    if (multiLineEdit)
                    {
                        multiLineEdit->setText(textParam->toUiValue());
                    }
                    else
                    {
                        dbgPlugins << "Widget not available; Role: " << role << " TYPE " <<p->m_type;
                    }
                }
                break;
            }

            default:{
                dbgPlugins << "IGNORING : " << qPrintable(PARAMETER_NAMES[p->m_type]);
                break;
            }

        }

    }

    if (role == CreateRole)
    {
        setLayout(gridLayout);
    }
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
    dbgPlugins << "setting choice param: failed?";
    Parameter * p = parameter(sender());
    if (!p)
    {
        return;
    }

    if (p->m_type != Parameter::CHOICE_P)
    {
        return;
    }
    dbgPlugins << "NO!" << "Setting " << index;

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

    textParam->fromUiValue(result);
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

QWidget* KisGmicSettingsWidget::widget(Parameter* parameter)
{
    if (!parameter)
    {
        return 0;
    }

    if (!m_parameterToWidgetMapper.contains(parameter))
    {
        return 0;
    }

    return qobject_cast<QWidget *>(m_parameterToWidgetMapper[parameter]);
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

    FolderParameter * folderParam = static_cast<FolderParameter *>(p);
    folderParam->fromUiValue(kurl.path());
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
    fileParam->fromUiValue(kurl.path());
}

void KisGmicSettingsWidget::mapParameterWidget(Parameter* parameter, QWidget* widget)
{
    m_parameterToWidgetMapper[parameter] = widget;
}


void KisGmicSettingsWidget::reload()
{
    createSettingsWidget(LoadRole);
}
