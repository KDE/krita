/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2009 Inge Wallin <inge@lysator.liu.se>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef IMAGESHAPE_PLUGIN_H
#define IMAGESHAPE_PLUGIN_H

// Qt
#include <QObject>
#include <QVariantList>

class ImageShapePlugin : public QObject
{
    Q_OBJECT

public:
    ImageShapePlugin(QObject *parent, const QVariantList &);
    ~ImageShapePlugin() override {}
};

#endif
