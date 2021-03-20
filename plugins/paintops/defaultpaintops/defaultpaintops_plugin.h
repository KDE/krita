/*
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DEFAULT_PAINTOPS_PLUGIN_H_
#define DEFAULT_PAINTOPS_PLUGIN_H_

#include <QObject>
#include <QVariant>

/**
 * A plugin wrapper that adds the paintop factories to the paintop registry.
 */
class DefaultPaintOpsPlugin : public QObject
{
    Q_OBJECT
public:
    DefaultPaintOpsPlugin(QObject *parent, const QVariantList &);
    ~DefaultPaintOpsPlugin() override;
};

#endif // DEFAULT_PAINTOPSGRAY_PLUGIN_H_
