/*
 *  SPDX-FileCopyrightText: 2008 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef HAIRY_PAINTOP_PLUGIN_H_
#define HAIRY_PAINTOP_PLUGIN_H_

#include <QObject>
#include <QVariant>

/**
 * A plugin wrapper that adds the paintop factories to the paintop registry.
 */
class HairyPaintOpPlugin : public QObject
{
    Q_OBJECT
public:
    HairyPaintOpPlugin(QObject *parent, const QVariantList &);
    ~HairyPaintOpPlugin() override;
};

#endif // HAIRY_PAINTOP_PLUGIN_H_
