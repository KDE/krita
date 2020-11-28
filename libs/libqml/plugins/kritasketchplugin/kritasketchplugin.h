/*
 * <one line to give the library's name and an idea of what it does.>
 * SPDX-FileCopyrightText: 2013 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#ifndef KRITASKETCHPLUGIN_H
#define KRITASKETCHPLUGIN_H

#include <QQmlExtensionPlugin>

class KritaSketchPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "QQmlExtensionInterface_iid")

public:
    virtual void registerTypes(const char* uri);
    virtual void initializeEngine(QQmlEngine* engine, const char* uri);

private:

};

#endif // KRITASKETCHPLUGIN_H
