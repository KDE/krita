/*
 *  SPDX-FileCopyrightText: 2026 Luna Lovecraft <ciubix8514@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef _KIS_SELECTION_ACTIONS_PANEL_BUTTON
#define _KIS_SELECTION_ACTIONS_PANEL_BUTTON

#include <qabstractbutton.h>

///Custom widget for selection actions panel buttons, to prevent them being drawn over the pop-up palette
class KisSelectionActionsPanelButton : public QAbstractButton {
  public:
    KisSelectionActionsPanelButton(const QString& iconName, const QString &tooltip, int size, QWidget *parent);
    ~KisSelectionActionsPanelButton();
    void draw(QPainter &painter);
protected:
    void paintEvent(QPaintEvent *e) override;
};


#endif
