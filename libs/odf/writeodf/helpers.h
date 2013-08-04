/* This file is part of the KDE project
   Copyright (C) 2013 Jos van den Oever <jos@vandenoever.info>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
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
