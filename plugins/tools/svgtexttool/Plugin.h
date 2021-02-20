/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef SVG_TEXT_TOOL_PLUGIN_H
#define SVG_TEXT_TOOL_PLUGIN_H

#include <QObject>
#include <QVariantList>

class Plugin : public QObject
{
    Q_OBJECT

public:
    Plugin(QObject *parent, const QVariantList &);
    ~Plugin() {}
};

#endif

