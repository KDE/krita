/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_BRUSH_SERVER_PROVIDER_H
#define KIS_BRUSH_SERVER_PROVIDER_H

#include <QString>
#include <QStringList>
#include <QList>

#include <KoResourceServer.h>

#include "kritabrush_export.h"
#include "kis_brush.h"

/**
 *
 */
class BRUSH_EXPORT KisBrushServerProvider : public QObject
{

    Q_OBJECT

public:
    KisBrushServerProvider();
    ~KisBrushServerProvider() override;

    KoResourceServer<KisBrush>* brushServer();

    static KisBrushServerProvider* instance();

private:

    KisBrushServerProvider(const KisBrushServerProvider&);
    KisBrushServerProvider operator=(const KisBrushServerProvider&);

    KoResourceServer<KisBrush>* m_brushServer;
};

#endif
