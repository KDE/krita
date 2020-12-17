/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SKETCH_PAINTOP_PLUGIN_H_
#define SKETCH_PAINTOP_PLUGIN_H_

#include <QObject>
#include <QVariant>

/**
 * A plugin wrapper that adds the paintop factories to the paintop registry.
 */
class SketchPaintOpPlugin : public QObject
{
    Q_OBJECT
public:
    SketchPaintOpPlugin(QObject *parent, const QVariantList &);
    ~SketchPaintOpPlugin() override;
};

#endif // SKETCH_PAINTOP_PLUGIN_H_
