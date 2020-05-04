/*
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
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
#ifndef KISCOLORLABELBUTTON_H
#define KISCOLORLABELBUTTON_H

#include <QButtonGroup>
#include <QAbstractButton>
#include <QSet>

#include "kritaui_export.h"

class KRITAUI_EXPORT KisColorLabelButton : public QAbstractButton
{
    Q_OBJECT
public:
    KisColorLabelButton(QColor color, QWidget *parent = nullptr);
    ~KisColorLabelButton();

    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;

    virtual void nextCheckState() override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};


class KRITAUI_EXPORT KisColorLabelFilterGroup : public QButtonGroup {
    Q_OBJECT
public:
    KisColorLabelFilterGroup(QObject* parent);
    ~KisColorLabelFilterGroup();

    QList<QAbstractButton*> viableButtons() const;
    void setViableLabels(QSet<int> &buttons);
    QSet<int> getActiveLabels() const;

    QList<QAbstractButton*> checkedViableButtons() const;
    int countCheckedViableButtons() const;
    int countViableButtons() const;

    void reset();

private:
    void hideAll();

    QSet<int> viableColorLabels;

};

#endif // KISCOLORLABELBUTTON_H
