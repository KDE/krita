/*
 *  SPDX-FileCopyrightText: 2003 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/
#ifndef KO_LCMS_ENGINE_PLUGIN_H
#define KO_LCMS_ENGINE_PLUGIN_H

#include <QObject>
#include <QVariantList>

class LcmsEnginePlugin : public QObject
{
    Q_OBJECT
public:
    LcmsEnginePlugin(QObject *parent, const QVariantList &);

};

#endif // KO_LCMS_ENGINE_PLUGIN_H
