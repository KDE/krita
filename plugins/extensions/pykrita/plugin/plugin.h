/*
 * This file is part of PyKrita, Krita' Python scripting plugin.
 *
 * SPDX-FileCopyrightText: 2013 Alex Turbov <i.zaufi@gmail.com>
 * SPDX-FileCopyrightText: 2014-2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only
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
