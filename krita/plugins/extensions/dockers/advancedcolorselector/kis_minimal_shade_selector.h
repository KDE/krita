/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_MINIMAL_SHADE_SELECTOR_H
#define KIS_MINIMAL_SHADE_SELECTOR_H

#include "kis_color_selector_base.h"

class KisShadeSelectorLine;
class KisCanvas2;

class KisMinimalShadeSelector : public KisColorSelectorBase
{
Q_OBJECT
public:
    explicit KisMinimalShadeSelector(QWidget *parent = 0);
    void setCanvas(KisCanvas2* canvas);

public slots:
    void setColor(const QColor& color);
    void updateSettings();
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

protected slots:
    void resourceChanged(int key, const QVariant& v);

protected:
    void paintEvent(QPaintEvent *);
    virtual KisColorSelectorBase* createPopup() const;

private:
    QList<KisShadeSelectorLine*> m_shadingLines;
    QColor m_lastColor;
    KisCanvas2* m_canvas;
};

#endif
