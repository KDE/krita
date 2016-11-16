/*
 * Copyright (C) Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>, (C) 2016
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
#ifndef KIS_VISUAL_COLOR_SELECTOR_H
#define KIS_VISUAL_COLOR_SELECTOR_H

#include <QWidget>
#include <QScopedPointer>
#include <QPixmap>
#include <QRegion>
#include <QMouseEvent>

#include <KoColor.h>
#include <KoColorSpace.h>
#include "KoColorDisplayRendererInterface.h"

#include "KisColorSelectorConfiguration.h"

#include "kritaui_export.h"

/**
 * @brief The KisVisualColorSelector class
 *
 * This gives a color selector box that draws gradients and everything.
 *
 * Unlike other color selectors, this one draws the full gamut of the given
 * colorspace.
 */
class KRITAUI_EXPORT KisVisualColorSelector : public QWidget
{
    Q_OBJECT
public:

    explicit KisVisualColorSelector(QWidget *parent = 0);
    ~KisVisualColorSelector();

    /**
     * @brief setConfig
     * @param forceCircular
     * Force circular is for space where you only have room for a circular selector.
     * @param forceSelfUpdate
     * force self-update is for making it update itself when using a modal dialog.
     */
    void setConfig(bool forceCircular, bool forceSelfUpdate);
    KoColor getCurrentColor();

Q_SIGNALS:
    void sigNewColor(KoColor c);

public Q_SLOTS:

    void slotSetColor(KoColor c);
    void slotsetColorSpace(const KoColorSpace *cs);
    void slotRebuildSelectors();
    void configurationChanged();
    void setDisplayRenderer (const KoColorDisplayRendererInterface *displayRenderer);
private Q_SLOTS:
    void updateFromWidgets(KoColor c);
    void HSXwrangler();
protected:
    void leaveEvent(QEvent *);
    void resizeEvent(QResizeEvent *);
private:
    struct Private;
    const QScopedPointer<Private> m_d;

    void updateSelectorElements(QObject *source);
    void drawGradients();

};

#endif
