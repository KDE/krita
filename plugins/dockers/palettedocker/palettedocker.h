/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef PALETTEDOCKER_H
#define PALETTEDOCKER_H

#include <QObject>
#include <QVariant>


/**
 * Docker showing the channels of the current layer
 */
class PaletteDockerPlugin : public QObject
{
    Q_OBJECT
public:
    PaletteDockerPlugin(QObject *parent, const QVariantList &);
    ~PaletteDockerPlugin() override;
};

#endif
