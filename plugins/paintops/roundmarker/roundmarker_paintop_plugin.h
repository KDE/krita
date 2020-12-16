/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _ROUNDMARKER_PAINTOP_PLUGIN_H_
#define _ROUNDMARKER_PAINTOP_PLUGIN_H_

#include <QObject>
#include <QVariant>

/**
 * A plugin wrapper that adds the paintop factories to the paintop registry.
 */
class RoundMarkerPaintOpPlugin: public QObject
{
    Q_OBJECT
public:
    RoundMarkerPaintOpPlugin(QObject *parent, const QVariantList &);
    ~RoundMarkerPaintOpPlugin() override;
};

#endif // _ROUNDMARKER_PAINTOP_PLUGIN_H_
