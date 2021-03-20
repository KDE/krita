/* This file is part of the KDE project
 *  SPDX-FileCopyrightText: Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_EMBOSS_FILTER_PLUGIN_H_
#define _KIS_EMBOSS_FILTER_PLUGIN_H_

#include <QObject>
#include <QVariant>

class KisEmbossFilterPlugin : public QObject
{
    Q_OBJECT
public:
    KisEmbossFilterPlugin(QObject *parent, const QVariantList &);
    ~KisEmbossFilterPlugin() override;
};

#endif

