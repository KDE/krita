/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DUMMY_PLUGIN_H
#define DUMMY_PLUGIN_H

#include <QObject>
#include <QString>
#include "DummyTrivialInterface.h"

class DummyPlugin : public DummyTrivialInterface
{
    Q_OBJECT
public:
    DummyPlugin(QObject *parent, const QVariantList &);

    QString name() const override { return DUMMY_PLUGIN_ID; }
    int version() const override { return DUMMY_PLUGIN_VERSION; }
};

#endif // DUMMY_PLUGIN_H