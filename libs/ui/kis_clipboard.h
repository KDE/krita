/*
 *  kis_clipboard.h - part of Krayon
 *
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2019 Dmitrii Utkin <loentar@gmail.com>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef __KIS_CLIPBOARD_H_
#define __KIS_CLIPBOARD_H_

#include <QObject>
#include <QSize>

#include <kis_types.h>

#include <kritaui_export.h>

#include <kis_default_bounds_base.h>

class QRect;
class QMimeData;
class KisMimeData;
class KisTimeSpan;
class KoColorSpace;

class KisClipboardPrivate;

/**
 * The Krita clipboard is a clipboard that can store paint devices
 * instead of just qimage's.
 */
class KRITAUI_EXPORT KisClipboard : public QObject
{

    Q_OBJECT
    Q_PROPERTY(bool clip READ hasClip NOTIFY clipChanged)

public:
    enum PasteBehaviour { PASTE_ASSUME_WEB = 0, PASTE_ASSUME_MONITOR, PASTE_ASK };

    enum PasteFormatBehaviour { PASTE_FORMAT_ASK = 0, PASTE_FORMAT_DOWNLOAD, PASTE_FORMAT_LOCAL, PASTE_FORMAT_CLIP };

    KisClipboard();
    ~KisClipboard() override;

    static KisClipboard* instance();

    /**
     * Get the contents of the clipboard in the form of a paint device.
     *
     * If the paint device originates from Krita itself, it may have
     * non-transparent default pixel and hence infinite visual extent.
     * To handle the case properly, the caller must provide a valid
     * \p defaultBounds object, which will make the infinite clip to
     * limit itself inside the bounds of the relevant image.
     */
    KisPaintDeviceSP clip(KisDefaultBoundsBaseSP defaultBounds,
                          bool showPopup,
                          int overridePasteBehaviour = -1) const;

    /**
     * Get the contents of the clipboard in a form of a bounded
     * image. To get the contents of the image just use the normal
     * `image->projection()` device.
     *
     * The origin of the "clip" is always placed at point (0,0)
     * of the image. The size of the image equals to the size of
     * the original "clip"
     *
     * The meaning of the "clip size" depends on where the clip
     * comes from:
     *
     * - when pasted from the clipboard buffer (i.e. QClipboard::image()),
     *   the size of the image equals to the size of the image in the buffer
     *
     * - when pasted from a URL, the size of the image also equals
     *   the size of the image in the file.
     *
     *   There is small complication though: if the source file is .kra, then
     *  `image->projection()` may have non-transparent default pixel. The caller
     *   should be ready to handle that by either keeping the parent KisImage
     *   alive all the time or by setting his own defaultBounds object on it.
     *
     * - when pasted from a Krita selection, then the top-left of the selection
     *   is placed at point (0,0) and the size of the image equals to the original
     *   size of the selection
     *
     * - when paster from a Krita layer, then the size of the image equals to
     *   the size of the original source image
     */
    KisImageSP clipImage(bool showPopup,
                         int overridePasteBehaviour = -1) const;

    /**
     * Given the mimedata for a paste event, ask user which source
     * they'll want to use for the image.
     *
     * @return A pair of bool and PasteFormatBehaviour:
     *   The boolean indicates acceptance of the paste event.
     *   If set, the PasteFormatBehaviour member indicates the chosen source.
     */
    QPair<bool, PasteFormatBehaviour> askUserForSource(const QMimeData *data,
                                                       bool useClipboardFallback = false) const;

    //A function to allow for caching the clipboard data
    QPair<bool, PasteFormatBehaviour> askUserForSourceWithData(QImage qimage,
                                                               const QList<QUrl> urls,
                                                               bool useClipboardFallback = false) const;

    KisPaintDeviceSP clipFromKritaLayers(const KoColorSpace *cs, KisDefaultBoundsBaseSP defaultBounds) const;
    KisImageSP clipImageFromKritaLayers(const KoColorSpace *cs = nullptr) const;
    KisImageSP clipImageFromBoardContents(bool showPopup, int overridePasteBehaviour, QPair<bool, PasteFormatBehaviour> source = {false, PasteFormatBehaviour::PASTE_FORMAT_ASK}) const;

    QImage getImageWithFallback(const QMimeData *cbData, bool useClipboardFallback = false) const;
    KisImageSP clipFromBoardContentsWithData(
        QImage image,
        const QList<QUrl> urls,
        bool showPopup,
        int overridePasteBehaviour = -1,
        QPair<bool, PasteFormatBehaviour> source = {false, PasteFormatBehaviour::PASTE_FORMAT_ASK}) const;

    KisImageSP fetchImageByURL(const QUrl &originalUrl) const;

    bool hasClip() const;

    /**
     * \see KisMimeData::referenceColorSpace()
     */
    const KoColorSpace* referenceColorSpace() const;

    void setLayers(KisNodeList nodes, KisImageSP image, const QRect referenceRect, bool forceCopy = false);
    bool hasLayers() const;
    bool hasLayerStyles() const;

    const QMimeData* layersMimeData() const;

    bool hasUrls() const;
    bool hasImage() const;

Q_SIGNALS:
    void clipChanged();

private Q_SLOTS:
    void clipboardDataChanged();

private:
    Q_DISABLE_COPY(KisClipboard);

    QImage getImageFromMimeData(const QMimeData *cbData) const;

    KisClipboardPrivate *const d;
};

#endif // __KIS_CLIPBOARD_H_
