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
#ifndef KIS_CUSTOM_IMAGE_WIDGET_H
#define KIS_CUSTOM_IMAGE_WIDGET_H

#include "kis_global.h"
#include "KoUnit.h"
#include "kis_properties_configuration.h"

#include <ui_wdgnewimage.h>

class KisDoc2;
class KoID;

class WdgNewImage : public QWidget, public Ui::WdgNewImage
{
    Q_OBJECT

public:
    WdgNewImage(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

/**
 * The 'Custom Document' widget in the Krita startup widget.
 * This class embeds the image size and colorspace to allow the user to select the image properties
 * for a new empty image document.
 */
class KisCustomImageWidget : public WdgNewImage
{
    Q_OBJECT
public:
    /**
     * Constructor. Please note that this class is being used/created by KisDoc.
     * @param parent the parent widget
     * @param doc the document that wants to be altered
     */
    KisCustomImageWidget(QWidget *parent, KisDoc2 *doc, qint32 defWidth, qint32 defHeight, bool clipAvailable, double resolution, const QString & defColorModel, const QString & defColorDepth, const QString & defColorProfile, const QString & imageName);
    virtual ~KisCustomImageWidget();
private slots:
    void createImage();
    void widthUnitChanged(int index);
    void widthChanged(double value);
    void heightUnitChanged(int index);
    void heightChanged(double value);
    void resolutionChanged(double value);
    void clipboardDataChanged();
    void screenSizeClicked();
    void predefinedClicked(int index);
    void saveAsPredefined();
    void switchWidthHeight();
signals:
    /// this signal is emitted (as defined by KoDocument) the moment the document is 'ready'
    void documentSelected();

private:
    quint8 backgroundOpacity() const;
    void fillPredefined();

    KisDoc2 *m_doc;
    double m_width, m_height;
    KoUnit m_widthUnit, m_heightUnit;
    QList<KisPropertiesConfiguration*> m_predefined;
};

#endif
