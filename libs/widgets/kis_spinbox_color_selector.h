/*
 *  SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
