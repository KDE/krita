/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 L. E. Segovia <amy@amyspark.me>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_IMAGE_INTERFACE_H
#define __KIS_IMAGE_INTERFACE_H

#include "kisimageinterface_export.h"

#include <QObject>
#include <QRect>
#include <QSharedMemory>
#include <QSize>

class KisQmicApplicator;
class KisViewManager;

class KISIMAGEINTERFACE_EXPORT KisImageInterface : public QObject
{
    Q_OBJECT

public:
    KisImageInterface(KisViewManager *parent = nullptr);
    ~KisImageInterface() = default;

    QSize gmic_qt_get_image_size();
    QByteArray gmic_qt_get_cropped_images(int mode, QRectF &cropRect);
    void gmic_qt_output_images(int mode, QStringList layers);
    void gmic_qt_detach();

private:
    struct Private;
    Private *p;

private Q_SLOTS:
    void slotStartApplicator(QStringList gmicImages);
    void slotGmicFinished(bool successfully, int milliseconds, const QString &msg);
};

#endif
