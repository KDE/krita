/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
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

#ifndef GRID_CONFIG_WIDGET_H
#define GRID_CONFIG_WIDGET_H

#include <QWidget>

namespace Ui {
class GridConfigWidget;
}

class GridConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GridConfigWidget(QWidget *parent = 0);
    ~GridConfigWidget();

private:
    Ui::GridConfigWidget *ui;
};

#endif // GRID_CONFIG_WIDGET_H
