/*  This file is part of the Calligra project, made within the KDE community.
 *
 * Copyright 2012  Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOPAGENAVIGATOR_H
#define KOPAGENAVIGATOR_H

// Qt
#include <QStackedWidget>

class KoPageNavigatorButton;
class KoPAView;
class KActionCollection;


class KoPageNavigator : public QStackedWidget
{
    Q_OBJECT

    enum State {Display = 0, Edit = 1};

public:
    explicit KoPageNavigator(KoPAView *view);
    virtual ~KoPageNavigator();

    void initActions();

protected:
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual bool eventFilter(QObject *watched, QEvent *event);

private Q_SLOTS:
    void updateDisplayLabel();
    void onPageNumberEntered();

private:
    class Private;
    Private *const d;
};

#endif //KOPAGENAVIGATOR_H
