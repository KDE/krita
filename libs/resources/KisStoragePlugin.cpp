/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KisStoragePlugin.h"

const QString KisStoragePlugin::s_meta_generator("meta:generator");
const QString KisStoragePlugin::s_meta_author("dc:author");
const QString KisStoragePlugin::s_meta_title("dc:title");
const QString KisStoragePlugin::s_meta_description("dc:description");
const QString KisStoragePlugin::s_meta_initial_creator("meta:initial-creator");
const QString KisStoragePlugin::s_meta_creator("cd:creator");
const QString KisStoragePlugin::s_meta_creation_data("meta:creation-data");
const QString KisStoragePlugin::s_meta_dc_date("meta:dc-date");
const QString KisStoragePlugin::s_meta_user_defined("meta:meta-userdefined");
const QString KisStoragePlugin::s_meta_name("meta:name");
const QString KisStoragePlugin::s_meta_value("meta:value");
const QString KisStoragePlugin::s_meta_version("meta:bundle-version");

class KisStoragePlugin::Private
{
public:
    QString location;
};

KisStoragePlugin::KisStoragePlugin(const QString &location)
    : d(new Private())
{
    d->location = location;
}

KisStoragePlugin::~KisStoragePlugin()
{

}

QString KisStoragePlugin::location() const
{
    return d->location;
}
