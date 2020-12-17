/* This file is part of the Calligra project
 * SPDX-FileCopyrightText: 2005 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_IMAGE_FROM_CLIPBOARD_WIDGET_H
#define KIS_IMAGE_FROM_CLIPBOARD_WIDGET_H

#include "kis_global.h"
#include "kis_properties_configuration.h"
#include "kis_custom_image_widget.h"
#include <QPointer>

/**
 * The 'New image from clipboard' widget in the Krita startup widget.
 * This class is an extension of the KisCustomImageWidget("Custom document" widget"
 */
class KisImageFromClipboard : public KisCustomImageWidget
{
    Q_OBJECT
public:
    /**
     * Constructor. Please note that this class is being used/created by KisDoc.
     * @param parent the parent widget
     * @param defWidth The defined width
     * @param defHeight The defined height
     * @param resolution The image resolution
     * @param defColorModel The defined color model
     * @param defColorDepth The defined color depth
     * @param defColorProfile The defined color profile
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

