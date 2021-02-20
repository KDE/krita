/*
 *  SPDX-FileCopyrightText: 2008 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef CURVE_PAINTOP_PLUGIN_H_
#define CURVE_PAINTOP_PLUGIN_H_

#include <QObject>
#include <QVariant>

/**
 * A plugin wrapper that adds the paintop factories to the paintop registry.
 */
class CurvePaintOpPlugin : public QObject
{
    Q_OBJECT
public:
    CurvePaintOpPlugin(QObject *parent, const QVariantList &);
    ~CurvePaintOpPlugin() override;
};

#endif // CURVE_PAINTOP_PLUGIN_H_
