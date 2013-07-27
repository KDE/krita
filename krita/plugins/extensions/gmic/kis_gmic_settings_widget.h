#ifndef KIS_GMIC_SETTINGS_WIDGET
#define KIS_GMIC_SETTINGS_WIDGET

#include <QWidget>
#include <QHash>

#include <Command.h>
#include <kis_config_widget.h>


/**
 * creates GUI for GMIC filter definition (Command)
 */
class KisGmicSettingsWidget : public KisConfigWidget {
    Q_OBJECT
public:
    KisGmicSettingsWidget(Command * command = 0);
    ~KisGmicSettingsWidget();


    virtual KisPropertiesConfiguration* configuration() const { return 0; }
    virtual void setConfiguration(const KisPropertiesConfiguration* config) { Q_UNUSED(config) }

    Command * currentCommandSettings();



private:
    void createSettingsWidget();

private:
    Command * m_commandDefinition;
    QHash<void *, int> m_widgetToParameterIndexMapper;
    //Command m_currentPreset;
    Parameter * parameter(QObject * widget);

private slots:
    void setIntValue(int value);
    void setFloatValue(qreal value);
    void setChoiceIndex(int index);

};

Q_DECLARE_METATYPE(KisGmicSettingsWidget*)

#endif

