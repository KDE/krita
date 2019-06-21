/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

private:
    QList<KoColor> m_colorHistory;
    KisCanvasResourceProvider  *m_resourceProvider; // to disconnect...
};

#endif // KIS_COLOR_HISTORY_H
