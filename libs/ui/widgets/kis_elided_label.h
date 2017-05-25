/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_ELIDED_LABEL_H
#define __KIS_ELIDED_LABEL_H

#include <QScopedPointer>
#include <QLabel>

#include "kritaui_export.h"


/**
 * A special QLabel subclass that makes the text elidable.
 *
 * Please use setLongText() instead of setText(). The latter one will
 * not work!
 */

class KRITAUI_EXPORT KisElidedLabel : public QLabel
{
public:
    KisElidedLabel(const QString &text, Qt::TextElideMode mode, QWidget *parent = 0);
    ~KisElidedLabel() override;

    void setLongText(const QString &text);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateText();
    using QLabel::setText;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_ELIDED_LABEL_H */
