/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
