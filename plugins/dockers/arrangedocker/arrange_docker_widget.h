/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    Ui::ArrangeDockerWidget *ui;

    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // ARRANGE_DOCKER_WIDGET_H
