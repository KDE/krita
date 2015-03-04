/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_SHADE_SELECTOR_LINE_COMBO_BOX_POPUP_H
#define __KIS_SHADE_SELECTOR_LINE_COMBO_BOX_POPUP_H

#include <QWidget>

class KisShadeSelectorLineBase;
class KisShadeSelectorLineEditor;
class KisColorSelectorBaseProxy;


class KisShadeSelectorLineComboBoxPopup : public QWidget {
    Q_OBJECT
public:
    ~KisShadeSelectorLineComboBoxPopup();

    const int spacing;

    KisShadeSelectorLineComboBoxPopup(QWidget* parent);
    void setConfiguration(const QString &string);

private Q_SLOTS:
    void activateItem(QWidget *widget);

private:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent * e);
    void mousePressEvent(QMouseEvent* e);

    void updateSelectedArea(const QRect &newRect);
    void updateHighlightedArea(const QRect &newRect);

private:
    KisShadeSelectorLineBase *m_lastHighlightedItem;
    KisShadeSelectorLineBase *m_lastSelectedItem;
    KisShadeSelectorLineEditor *m_lineEditor;

    QRect m_highlightedArea;
    QRect m_selectedArea;
    QScopedPointer<KisColorSelectorBaseProxy> m_parentProxy;
};

#endif /* __KIS_SHADE_SELECTOR_LINE_COMBO_BOX_POPUP_H */
