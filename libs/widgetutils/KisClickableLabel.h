/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2023 Halla Rempt <halla@valdyas.org>
 * SPDX-FileCopyrightText: 2023 Alvin Wong <alvin@alvinhc.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KISCLICKABLELABEL_H
#define KISCLICKABLELABEL_H

#include <QPixmap>
#include <QLabel>
#include <QObject>
#include <QWidget>
#include <QPushButton>

#include <kritawidgetutils_export.h>

class KRITAWIDGETUTILS_EXPORT KisClickableLabel : public QLabel
{
    Q_OBJECT
public:

    explicit KisClickableLabel(QWidget *parent = nullptr);
    ~KisClickableLabel() override;

    bool hasHeightForWidth() const override;
    int heightForWidth(int w) const override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void setUnscaledPixmap(QPixmap pixmap);
    void updatePixmap();

    void setDismissable(bool value = true);
    bool isDismissable();

Q_SIGNALS:
    void clicked();
    void dismissed();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QPixmap m_pixmap;

    bool m_dismissable = false;
    QPushButton *m_closeButton;
};

#endif // KISCLICKABLELABEL_H
