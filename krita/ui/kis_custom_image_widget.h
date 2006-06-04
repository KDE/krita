/* This file is part of the KOffice project
 * Copyright (C) 2005 Thomas Zander <zander@kde.org>
 * Copyright (C) 2005 Casper Boemann <cbr@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KIS_CUSTOM_IMAGE_WIDGET_H
#define KIS_CUSTOM_IMAGE_WIDGET_H

#include "kis_global.h"
#include "kis_dlg_image_properties.h"

class KisDoc;
class KoID;

/**
 * The 'Custom Document' widget in the Krita startup widget.
 * This class embeds the image size and colorspace to allow the user to select the image properties
 * for a new empty image document.
 */
class KisCustomImageWidget : public WdgNewImage {
    Q_OBJECT
public:
    /**
     * Constructor. Please note that this class is being used/created by KisDoc.
     * @param parent the parent widget
     * @param doc the document that wants to be altered
     */
    KisCustomImageWidget(QWidget *parent, KisDoc *doc, qint32 defWidth, qint32 defHeight, double resolution, QString defColorSpaceName, QString imageName);

private slots:
    void buttonClicked();
    void fillCmbProfiles(const KoID & s);

signals:
    /// this signal is emitted (as defined by KoDocument) the moment the document is 'ready'
    void documentSelected();

private:
    quint8 backgroundOpacity() const;

    KisDoc *m_doc;
};

#endif
