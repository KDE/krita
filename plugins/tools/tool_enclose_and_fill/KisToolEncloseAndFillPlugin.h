/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISTOOLENCLOSEANDFILLPLUGIN_H
#define KISTOOLENCLOSEANDFILLPLUGIN_H

#include <QObject>
#include <QVariant>

class KisToolEncloseAndFillPlugin : public QObject
{
    Q_OBJECT
public:
    KisToolEncloseAndFillPlugin(QObject *parent, const QVariantList &);
    ~KisToolEncloseAndFillPlugin() override;

};

#endif
