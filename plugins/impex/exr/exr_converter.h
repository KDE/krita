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

class EXRConverter : public QObject
{
    Q_OBJECT
public:
    EXRConverter(KisDocument *doc, bool showNotifications);
    ~EXRConverter() override;
public:
    KisImageBuilder_Result buildImage(const QString &filename);
    KisImageBuilder_Result buildFile(const QString &filename, KisPaintLayerSP layer);
    KisImageBuilder_Result buildFile(const QString &filename, KisGroupLayerSP layer, bool flatten=false);
    /**
     * Retrieve the constructed image
     */
    KisImageSP image();
    QString errorMessage() const;
private:
    KisImageBuilder_Result decode(const QString &filename);

public Q_SLOTS:
    virtual void cancel();
private:
    struct Private;
    const QScopedPointer<Private> d;
};

#endif
