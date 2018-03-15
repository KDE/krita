/*
 * This file is part of PyKrita, Krita' Python scripting plugin.
 *
 * Copyright (C) 2013 Alex Turbov <i.zaufi@gmail.com>
 * Copyright (C) 2014-2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) version 3.
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
#ifndef _PYQT_PLUGIN_H_
#define _PYQT_PLUGIN_H_

#include <QObject>

#include <KisActionPlugin.h>
#include "PythonPluginManager.h"

class KritaPyQtPlugin : public QObject
{
    Q_OBJECT
public:
    KritaPyQtPlugin(QObject *parent, const QVariantList &);
    virtual ~KritaPyQtPlugin();
private:
    PythonPluginManager *pluginManager;
    bool m_autoReload;
};

#endif
