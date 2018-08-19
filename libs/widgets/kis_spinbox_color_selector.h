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

#ifndef KISSPINBOXCOLORSELECTOR_H
#define KISSPINBOXCOLORSELECTOR_H

#include <QWidget>
#include "kritawidgets_export.h"
#include <QScopedPointer>
#include "KoColor.h"
#include "KoColorSpace.h"

/**
 * @brief The KisSpinboxColorSelector class
 * This will give a widget with spinboxes depending on the color space
 * Take responsibility for changing the color space.
 */
class KRITAWIDGETS_EXPORT KisSpinboxColorSelector : public QWidget
{
    Q_OBJECT
public:
    explicit KisSpinboxColorSelector(QWidget *parent);
    ~KisSpinboxColorSelector() override;

    void chooseAlpha(bool chooseAlpha);

Q_SIGNALS:

    void sigNewColor(KoColor color);

public Q_SLOTS:

    void slotSetColorSpace(const KoColorSpace *cs);
    void slotSetColor(KoColor color);
private Q_SLOTS:
    void slotUpdateFromSpinBoxes();
private:
    struct Private;
    const QScopedPointer<Private> m_d;
    void createColorFromSpinboxValues();
    void updateSpinboxesWithNewValues();
};

#endif // KISSPINBOXCOLORSELECTOR_H
