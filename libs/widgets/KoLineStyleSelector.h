/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KOLINESTYLESELECTOR_H_
#define _KOLINESTYLESELECTOR_H_

#include "kritawidgets_export.h"
#include <QComboBox>

/**
 * A custom combobox widget for selecting line styles.
 */
class KRITAWIDGETS_EXPORT KoLineStyleSelector : public QComboBox
{
    Q_OBJECT
public:
    explicit KoLineStyleSelector(QWidget *parent = 0);
    ~KoLineStyleSelector() override;

    /**
     * Adds a new line style to the combobox.
     *
     * If the style already exists, it is not added to the selector.
     *
     * @param style the line style to add
     * @return true if style is unique among the existing styles and was added, else false
     */
    bool addCustomStyle(const QVector<qreal> &style);

    /**
     * Selects the specified style.
     *
     * If the style was already added it gets selected. If the style was not added already
     * it gets temporary added and selected.
     *
     * @param style the style to display
     * @param dashes the dashes of the style if style == Qt::CustomDashLine
     */
    void setLineStyle(Qt::PenStyle style, const QVector<qreal> &dashes = QVector<qreal>());

    /// Returns the current line style
    Qt::PenStyle lineStyle() const;
    /// Returns the dashes of the current line style
    QVector<qreal> lineDashes() const;

protected:
    void paintEvent(QPaintEvent *pe) override;

private:
    class Private;
    Private * const d;
};

#endif // _KOLINESTYLESELECTOR_H_
