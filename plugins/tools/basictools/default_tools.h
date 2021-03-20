/*
 *  SPDX-FileCopyrightText: 2003 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DEFAULT_TOOLS_H_
#define DEFAULT_TOOLS_H_

#include <QObject>
#include <QVariant>

/**
 * A module wrapper around Krita's default tools.
 * Despite the fact that new tools are created for every new view,
 * it is not possible to make tools standard parts of the type of the
 * imagesize plugin, because we need to create a new set of tools for every
 * pointer device (mouse, stylus, eraser, puck, etc.). So this plugin is
 * a module which is loaded once into Krita. For every tool there is a factory
 * class that is registered with the tool registry, and that is used to create
 * new instances of the tools.
 */
class DefaultTools : public QObject
{
    Q_OBJECT
public:
    DefaultTools(QObject *parent, const QVariantList &);
    ~DefaultTools() override;

};

#endif // DEFAULT_TOOLS_H_
