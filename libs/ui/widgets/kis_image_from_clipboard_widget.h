/* This file is part of the Calligra project
 * Copyright (C) 2005 Thomas Zander <zander@kde.org>
 * Copyright (C) 2005 C. Boemann <cbo@boemann.dk>
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
#ifndef KIS_IMAGE_FROM_CLIPBOARD_WIDGET_H
#define KIS_IMAGE_FROM_CLIPBOARD_WIDGET_H

#include "kis_global.h"
#include "kis_properties_configuration.h"
#include "kis_custom_image_widget.h"
#include <QPointer>

/**
 * The 'New image from clipboard' widget in the Krita startup widget.
 * This class is an exstension of the KisCustomImageWidget("Custom document" widget"
 */
class KisImageFromClipboard : public KisCustomImageWidget
{
    Q_OBJECT
public:
    /**
     * Constructor. Please note that this class is being used/created by KisDoc.
     * @param parent the parent widget
     * @param imageName the document that wants to be altered
     */
    KisImageFromClipboard(QWidget *parent, qint32 defWidth, qint32 defHeight, double resolution, const QString & defColorModel, const QString & defColorDepth, const QString & defColorProfile, const QString & imageName);
    ~KisImageFromClipboard() override;

private Q_SLOTS:
    void createImage();
    void clipboardDataChanged();
    
private:   
    void createClipboardPreview();

};

#endif

