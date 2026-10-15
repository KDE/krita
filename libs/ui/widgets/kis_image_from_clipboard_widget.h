/*
 * SPDX-FileCopyrightText: 2005 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_IMAGE_FROM_CLIPBOARD_WIDGET_H
#define KIS_IMAGE_FROM_CLIPBOARD_WIDGET_H

#include "kis_custom_image_widget.h"

/**
 * The 'New image from clipboard' widget in the Krita startup widget.
 * This class is an extension of the KisCustomImageWidget("Custom document" widget"
 */
class KisImageFromClipboardWidget : public KisCustomImageWidget
{
    Q_OBJECT
public:
    /**
     * Constructor. Please note that this class is being used/created by KisDoc.
     * @param parent the parent widget
     * @param imageName the document that wants to be altered
     * @param dialogType is a prefix that will be used for fetching image layout settings
     */
    KisImageFromClipboardWidget(QWidget *parent, const QString & imageName, const QLatin1String &dialogType);
    ~KisImageFromClipboardWidget() override;

protected:

    void showEvent(QShowEvent *event) override;
    bool shouldSaveImageDimensions() const override;

private Q_SLOTS:
    void createClipboardPreview();
    void createImage() override;
    void clipboardDataChanged();
    void enableImageCreation(const QImage &);
};

#endif

