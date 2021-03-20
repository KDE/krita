/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_COLOR_HISTORY_H
#define KIS_COLOR_HISTORY_H

#include "kis_color_patches.h"


class KisCanvasResourceProvider;

class KisColorHistory : public KisColorPatches
{
    Q_OBJECT
public:
    explicit KisColorHistory(QWidget *parent = 0);
    void setCanvas(KisCanvas2 *canvas) override;
    void unsetCanvas() override;

protected:
    KisColorSelectorBase* createPopup() const override;

public Q_SLOTS:
    void addColorToHistory(const KoColor& color);

    void clearColorHistory();
private:
    QList<KoColor> m_colorHistory;
    KisCanvasResourceProvider  *m_resourceProvider; // to disconnect...
};

#endif // KIS_COLOR_HISTORY_H
