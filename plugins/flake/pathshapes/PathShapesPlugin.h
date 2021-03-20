/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef PATHSHAPESPLUGIN_H
#define PATHSHAPESPLUGIN_H

#include <QObject>
#include <QVariantList>

class PathShapesPlugin : public QObject
{
    Q_OBJECT

public:
    PathShapesPlugin(QObject *parent, const QVariantList &);
    ~PathShapesPlugin() override {}

};

#endif
