/*
 *  kis_clipboard.h - part of Krayon
 *
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef __KIS_CLIPBOARD_H_
#define __KIS_CLIPBOARD_H_

#include <QObject>
#include <QSize>
#include "kis_types.h"
#include <kritaui_export.h>
#include <KoColorProfile.h>

class QRect;
class QMimeData;
class KisTimeSpan;
class KisBlockUntilOperationsFinishedMediator;

enum enumPasteBehaviour {
    PASTE_ASSUME_WEB,
    PASTE_ASSUME_MONITOR,
    PASTE_ASK
};

/**
 * The Krita clipboard is a clipboard that can store paint devices
 * instead of just qimage's.
 */
class KRITAUI_EXPORT KisClipboard : public QObject
{

    Q_OBJECT
    Q_PROPERTY(bool clip READ hasClip NOTIFY clipChanged)

public:
    KisClipboard();
    ~KisClipboard() override;

    static KisClipboard* instance();

    /**
     * Sets the clipboard to the contents of the specified paint device; also
     * set the system clipboard to a QImage representation of the specified
     * paint device.
     *
     * @param dev The paint device that will be stored on the clipboard
     * @param topLeft a hint about the place where the clip should be pasted by default
     */
    void setClip(KisPaintDeviceSP dev, const QPoint& topLeft);

    void setClip(KisPaintDeviceSP dev, const QPoint& topLeft, const KisTimeSpan &range);

    /**
     * Get the contents of the clipboard in the form of a paint device.
     */
    KisPaintDeviceSP clip(const QRect &imageBounds, bool showPopup, KisTimeSpan *clipRange = 0, const KoColorProfile *destProfile = 0);

    bool hasClip() const;

    QSize clipSize() const;

    void setLayers(KisNodeList nodes, KisImageSP image, bool forceCopy = false);
    bool hasLayers() const;
    const QMimeData* layersMimeData() const;

Q_SIGNALS:

    void clipCreated();

private Q_SLOTS:

    void clipboardDataChanged();

private:

    KisClipboard(const KisClipboard &);
    KisClipboard operator=(const KisClipboard &);

    bool m_hasClip;

    bool m_pushedClipboard;

Q_SIGNALS:
    void clipChanged();
};

#endif // __KIS_CLIPBOARD_H_
