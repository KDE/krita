/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef ARRANGE_DOCKER_WIDGET_H
#define ARRANGE_DOCKER_WIDGET_H

#include <QWidget>
#include <QScopedPointer>

#include "kactioncollection.h"


namespace Ui {
class ArrangeDockerWidget;
}

class ArrangeDockerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ArrangeDockerWidget(QWidget *parent = 0);
    ~ArrangeDockerWidget() override;

    void setActionCollection(KActionCollection *collection);
    void switchState(bool enabled);

private:
    QScopedPointer<Ui::ArrangeDockerWidget> ui;

    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // ARRANGE_DOCKER_WIDGET_H
