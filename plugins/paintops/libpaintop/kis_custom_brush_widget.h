/*
 *  SPDX-FileCopyrightText: 2005 Bart Coppens <kde@bartcoppens.be>
 *  SPDX-FileCopyrightText: 2026 Freya Lupen <penguinflyer2222@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CUSTOM_BRUSH_H_
#define KIS_CUSTOM_BRUSH_H_

#include <QDialog>

#include "kis_config_widget.h"
#include "kis_types.h"
#include "KoResourceServer.h"

class QLabel;
class QDialogButtonBox;

class KisBrush;

class KisCustomBrushWidget : public QDialog
{
    Q_OBJECT
public:
    KisCustomBrushWidget(QWidget *parent, const QString& caption);
    virtual ~KisCustomBrushWidget();

private Q_SLOTS:
    // Swaps the brush widget and reloads the brush.
    void slotSourceChanged(int source);

    void slotReload();

    // Exports the brushtip and adds it to the resource server.
    void slotAddPredefined();

    void slotUpdateSaveButton();

    // Responds to the the export configuration changing.
    void slotUpdateBrush();

Q_SIGNALS:
    void sigNewPredefinedBrush(KoResourceSP );

private:
    // @returns the brush widget that should be shown according to m_clipboard.
    KisConfigWidget* currentBrushWidget();

    // @returns the brush widget that should not be shown according to m_clipboard.
    KisConfigWidget* otherBrushWidget();

    /* Swaps between GBR (non-animated) and GIH (animated) brush widgets.
     * Clipboard source doesn't support animated and shows the GBR widget,
     * otherwise the GIH widget allowing animated is shown.
     */
    void swapBrushWidget();

    // @returns the current view's image.
    KisImageWSP currentImage();
    int currentBrushStyle();

    // @returns a temporary filepath to export to with the proper filename and extension.
    QString fileName();

    // Caches the device(s) used for the brushtip from the image or clipboard.
    void loadImage();

    /* Updates the preview thumbnail if brush style, use mask, or preserve alpha
     * have changed, or if updateDev indicates the paint device has changed.
     * @param updateDev whether the paint device has changed.
     */
    void updatePreviewImage(bool updateDev);

    QLabel* m_preview {nullptr};
    KisConfigWidget* m_brushWidget {nullptr};
    KisConfigWidget* m_animBrushWidget {nullptr};
    QDialogButtonBox* m_buttonBox {nullptr};

    KisPaintDeviceSP m_dev {nullptr};
    QRect m_devRect;
    QVector<QVector<KisPaintDevice*>> m_devices;
    QSize m_imageSize;
    const KoColorSpace* m_imageColorSpace;

    KoResourceServer<KisBrush> *m_rServer {0};

    bool m_useClipboard {false};
    KisPropertiesConfigurationSP m_oldConfig {nullptr};
};


#endif // KIS_CUSTOM_BRUSH_H_
