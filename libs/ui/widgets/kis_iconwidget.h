/*
 *  SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
 *  SPDX-FileCopyrightText: 2003 Patrick Julien <freak@codepimps.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_ICONWIDGET_H_
#define KIS_ICONWIDGET_H_

#include <KisPopupButton.h>
#include <kritaui_export.h>

#include <KoResource.h>

/**
 * The icon widget is used in the control box where the current color and brush
 * are shown.
 */
class KRITAUI_EXPORT KisIconWidget : public KisPopupButton
{

    Q_OBJECT

public:
    KisIconWidget(QWidget *parent = 0, const QString &name = QString());
    void setThumbnail(const QImage &thumbnail);
    void setResource(KoResourceSP resource);

    QSize preferredIconSize() const;

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QImage m_thumbnail;
    KoResourceSP m_resource;
};

#endif // KIS_ICONWIDGET_H_

