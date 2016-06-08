/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_SPACING_SELECTION_WIDGET_H
#define __KIS_SPACING_SELECTION_WIDGET_H

#include <QWidget>
#include <QScopedPointer>

#include <kritapaintop_export.h>

class PAINTOP_EXPORT KisSpacingSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    KisSpacingSelectionWidget(QWidget *parent);
    ~KisSpacingSelectionWidget();

    void setSpacing(bool isAuto, qreal spacing);

    qreal spacing() const;
    bool autoSpacingActive() const;
    qreal autoSpacingCoeff() const;

Q_SIGNALS:
    void sigSpacingChanged();

private:
    Q_PRIVATE_SLOT(m_d, void slotSpacingChanged(qreal value));
    Q_PRIVATE_SLOT(m_d, void slotAutoSpacing(bool value));

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_SPACING_SELECTION_WIDGET_H */
