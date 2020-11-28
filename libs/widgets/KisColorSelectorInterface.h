/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISCOLORSELECTORINTERFACE_H
#define KISCOLORSELECTORINTERFACE_H

#include "kritawidgets_export.h"

#include <QWidget>
#include <KoColor.h>

class KoColorDisplayRendererInterface;
class KoColorSpace;

class KRITAWIDGETS_EXPORT KisColorSelectorInterface : public QWidget {
    Q_OBJECT
public:
    KisColorSelectorInterface(QWidget *parent = 0)
        : QWidget(parent)
    {}
    ~KisColorSelectorInterface() override {}
    virtual void setConfig(bool forceCircular, bool forceSelfUpdate)
    {
        Q_UNUSED(forceCircular);
        Q_UNUSED(forceSelfUpdate);
    }
    virtual void setDisplayRenderer (const KoColorDisplayRendererInterface *displayRenderer)
    {
        Q_UNUSED(displayRenderer);
    }

    virtual KoColor getCurrentColor() const = 0;

Q_SIGNALS:
    void sigNewColor(const KoColor &c);

public Q_SLOTS:
    virtual void slotSetColor(const KoColor &c) = 0;
    /**
     * @brief slotSetColorSpace
     * Set the color space the selector should cover
     *
     * This is mostly a hint to decide visual presentation.
     * Internal processing may be in a different color space and
     * input conversion shall be handled by the selector itself.
     * Calling this voids the currently selected color.
     */
    virtual void slotSetColorSpace(const KoColorSpace *cs);
};

#endif // KISCOLORSELECTORINTERFACE_H
