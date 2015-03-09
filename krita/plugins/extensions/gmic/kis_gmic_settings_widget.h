#ifndef KIS_GMIC_SETTINGS_WIDGET
#define KIS_GMIC_SETTINGS_WIDGET

#include <QWidget>
#include <QHash>

#include <KUrl>

#include <Command.h>
#include <kis_config_widget.h>


/**
 * creates GUI for GMIC filter definition (Command)
 */

enum ROLE {
    CreateRole = 0,
    LoadRole = 1
};

class KisGmicSettingsWidget : public KisConfigWidget {
    Q_OBJECT
public:
    KisGmicSettingsWidget(Command * command = 0);
    ~KisGmicSettingsWidget();

    virtual KisPropertiesConfiguration* configuration() const { return 0; }
    virtual void setConfiguration(const KisPropertiesConfiguration* config) { Q_UNUSED(config) }

    Command * currentCommandSettings();

    void reload();

private:
    void createSettingsWidget(ROLE role);

private:
    Command * m_commandDefinition;
    QHash<void *, int> m_widgetToParameterIndexMapper;
    QHash<Parameter *, QWidget *> m_parameterToWidgetMapper;

    Parameter * parameter(QObject * widget);
    QWidget * widget(Parameter * parameter);

    // maps the parameter to widget in both directions, two hash tables are used for it
    void mapParameterWidget(Parameter * parameter, QWidget * widget);


private Q_SLOTS:
    void setIntValue(int value);
    void setFloatValue(qreal value);
    void setBoolValue(bool value);
    void setChoiceIndex(int index);
    void setColorValue(const QColor &color);
    void setTextValue();
    void setFolderPathValue(const KUrl &kurl);
    void setFilePathValue(const KUrl &kurl);

};

Q_DECLARE_METATYPE(KisGmicSettingsWidget*)

#endif

