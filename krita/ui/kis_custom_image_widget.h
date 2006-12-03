/* This file is part of the KOffice project
 * Copyright (C) 2005 Thomas Zander <zander@kde.org>
 * Copyright (C) 2005 Casper Boemann <cbr@boemann.dk>
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
#ifndef KIS_CUSTOM_IMAGE_WIDGET_H
#define KIS_CUSTOM_IMAGE_WIDGET_H

#include "kis_global.h"
#include "kis_dlg_image_properties.h"
#include "KoUnit.h"

class KisDoc2;
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
    KisCustomImageWidget(QWidget *parent, KisDoc2 *doc, qint32 defWidth, qint32 defHeight, double resolution, const QString & defColorSpaceName, const QString & imageName);

private slots:
    void buttonClicked();
    void fillCmbProfiles(const KoID & s);
    void widthUnitChanged(int index);
    void widthChanged(double value);
    void heightUnitChanged(int index);
    void heightChanged(double value);

signals:
    /// this signal is emitted (as defined by KoDocument) the moment the document is 'ready'
    void documentSelected();

private:
    quint8 backgroundOpacity() const;

    KisDoc2 *m_doc;
    double m_width, m_height;
    KoUnit m_widthUnit, m_heightUnit;
};

#endif
