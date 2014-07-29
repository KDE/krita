/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@kogmbh.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _PYQT_PLUGIN_H_
#define _PYQT_PLUGIN_H_

#include <QObject>

#include <Python.h>

#include <kis_view_plugin.h>
#include "engine.h"

class KritaPyQtPlugin : public KisViewPlugin
{
    Q_OBJECT
public:
    KritaPyQtPlugin(QObject *parent, const QVariantList &);
    virtual ~KritaPyQtPlugin();
private:
    PyKrita::Engine m_engine;
    QString m_engineFailureReason;
    bool m_autoReload;
};

#endif
