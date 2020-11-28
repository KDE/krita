/*
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _COLORSMUDGE_PAINTOP_PLUGIN_H_
#define _COLORSMUDGE_PAINTOP_PLUGIN_H_

#include <QObject>
#include <QVariant>

/**
 * A plugin wrapper that adds the paintop factories to the paintop registry.
 */
class ColorSmudgePaintOpPlugin: public QObject
{
    Q_OBJECT
public:
    ColorSmudgePaintOpPlugin(QObject *parent, const QVariantList &);
    ~ColorSmudgePaintOpPlugin() override;
};

#endif // _COLORSMUDGE_PAINTOP_PLUGIN_H_
