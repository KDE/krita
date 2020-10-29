/*
 *  Copyright (c) 2020 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef TOOLPRESETSPLUGIN_H
#define TOOLPRESETSPLUGIN_H

#include <KisActionPlugin.h>

#include <QVariant>
#include <QDockWidget>
#include <klocalizedstring.h>
#include <KoCanvasObserverBase.h>

class ToolPresets;

class BasicDocker : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    BasicDocker() : QDockWidget(i18n("Tool Presets")) {}
    ~BasicDocker() override {}
    QString observerName() override { return "ToolPresetsDocker"; }
    void setCanvas(KoCanvasBase *) override {}
    void unsetCanvas() override {}
};

class ToolPresetsPlugin : public QObject
{
    Q_OBJECT
public:
    ToolPresetsPlugin(QObject *parent, const QVariantList &);
    ~ToolPresetsPlugin() override;
};

#endif
