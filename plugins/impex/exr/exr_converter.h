/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _EXR_CONVERTER_H_
#define _EXR_CONVERTER_H_

#include <stdio.h>

#include <QObject>

#include "kis_types.h"
#include <KisImageBuilderResult.h>
class KisDocument;

class QUrl;

class exrConverter : public QObject
{
    Q_OBJECT
public:
    exrConverter(KisDocument *doc, bool showNotifications);
    virtual ~exrConverter();
public:
    KisImageBuilder_Result buildImage(const QUrl &uri);
    KisImageBuilder_Result buildFile(const QUrl &uri, KisPaintLayerSP layer);
    KisImageBuilder_Result buildFile(const QUrl &uri, KisGroupLayerSP layer);
    /**
     * Retrieve the constructed image
     */
    KisImageWSP image();
private:
    KisImageBuilder_Result decode(const QUrl &uri);
public Q_SLOTS:
    virtual void cancel();
private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif
