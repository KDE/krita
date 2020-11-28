/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __TIMELINE_LAYERS_HEADER_H
#define __TIMELINE_LAYERS_HEADER_H

#include <QHeaderView>
#include <QProxyStyle>

#include <QScopedPointer>


class KisAnimTimelineLayersHeader : public QHeaderView
{
    Q_OBJECT

public:
    KisAnimTimelineLayersHeader(QWidget *parent);
    KisAnimTimelineLayersHeader();
    ~KisAnimTimelineLayersHeader() override;

protected:
    void paintSection(QPainter *painter, const QRect &rect, int layerIndex) const override;
    QSize sectionSizeFromContents(int layerIndex) const override;
    bool viewportEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override {mousePressEvent(event);}

Q_SIGNALS:
    void sigRequestContextMenu(const QPoint &pos);

private:
    struct Private;
    const QScopedPointer<Private> m_d;


};

#endif /* __TIMELINE_LAYERS_HEADER_H */
