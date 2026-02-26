/*
 *  SPDX-FileCopyrightText: 2026 Luna Lovecraft <ciubix8514@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef _KIS_SELECTION_ACTIONS_PANNEL_HANDLE
#define _KIS_SELECTION_ACTIONS_PANNEL_HANDLE
#include <qpainter.h>
#include <qwidget.h>

class KisSelectionActionsPanelHandle : public QWidget
{
public:
    KisSelectionActionsPanelHandle(int size, QWidget *parent);
    ~KisSelectionActionsPanelHandle();
    void draw(QPainter& painter);
    void set_held(bool held);
private:
    struct Private;
    QScopedPointer<Private> d;
};
#endif
