/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    ~KisShadeSelectorLineComboBoxPopup() override;

    const int spacing;

    KisShadeSelectorLineComboBoxPopup(QWidget* parent);
    void setConfiguration(const QString &string);

private Q_SLOTS:
    void activateItem(QWidget *widget);

private:
    void paintEvent(QPaintEvent *) override;
    void mouseMoveEvent(QMouseEvent * e) override;
    void mousePressEvent(QMouseEvent* e) override;

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
