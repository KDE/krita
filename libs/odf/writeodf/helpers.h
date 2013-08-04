#ifndef WRITEODF_ODFWRITER_H
#define WRITEODF_ODFWRITER_H

#include "writeodf/writeodfconfig.h"

namespace writeodf {

template <class T>
void addConfigItem(T& config, const QString & configName, const QString& value)
{
    config_config_item item(config.add_config_config_item(configName, "string"));
    item.addTextNode(value);
}

template <class T>
void addConfigItem(T& config, const QString & configName, bool value)
{
    config_config_item item(config.add_config_config_item(configName, "boolean"));
    item.addTextNode(value ? "true" : "false");
}

template <class T>
void addConfigItem(T& config, const QString & configName, int value)
{
    config_config_item item(config.add_config_config_item(configName, "int"));
    item.addTextNode(QString::number(value));
}

template <class T>
void addConfigItem(T& config, const QString & configName, double value)
{
    config_config_item item(config.add_config_config_item(configName, "double"));
    item.addTextNode(QString::number(value));
}

template <class T>
void addConfigItem(T& config, const QString & configName, float value)
{
    config_config_item item(config.add_config_config_item(configName, "double"));
    item.addTextNode(QString::number(value));
}

template <class T>
void addConfigItem(T& config, const QString & configName, long value)
{
    config_config_item item(config.add_config_config_item(configName, "long"));
    item.addTextNode(QString::number(value));
}

template <class T>
void addConfigItem(T& config, const QString & configName, short value)
{
    config_config_item item(config.add_config_config_item(configName, "short"));
    item.addTextNode(QString::number(value));
}

}

#endif
