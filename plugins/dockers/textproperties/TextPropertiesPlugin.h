/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TEXTPROPERTIESPLUGIN_H
#define TEXTPROPERTIESPLUGIN_H

#include <QObject>
#include <QVariant>

class TextPropertiesPlugin : public QObject
{
    Q_OBJECT
public:
    TextPropertiesPlugin(QObject *parent, const QVariantList &);
    ~TextPropertiesPlugin();
};

#endif // TEXTPROPERTIESPLUGIN_H
