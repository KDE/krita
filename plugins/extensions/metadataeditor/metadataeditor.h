/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _METADATAEDITOR_H_
#define _METADATAEDITOR_H_

#include <QVariant>

#include <KisActionPlugin.h>

namespace KisMetaData
{
}

/**
 * Template of view plugin
 */
class metadataeditorPlugin : public KisActionPlugin
{
    Q_OBJECT
public:
    metadataeditorPlugin(QObject *parent, const QVariantList &);
    ~metadataeditorPlugin() override;

private Q_SLOTS:

    void slotEditLayerMetaData();
};

#endif // metadataeditorPlugin_H
