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

#ifndef __KIS_GUIDES_MANAGER_H
#define __KIS_GUIDES_MANAGER_H

#include <QScopedPointer>
#include <QObject>

class KisView;
class KisActionManager;
class KisCanvasDecoration;
class KoGuidesData;


class KisGuidesManager : public QObject
{
    Q_OBJECT
public:
    KisGuidesManager(QObject *parent = 0);
    ~KisGuidesManager();

    void setup(KisActionManager *actionManager);
    void setView(QPointer<KisView> view);

    KisCanvasDecoration* decoration() const;

    bool showGuides() const;
    bool lockGuides() const;
    bool snapToGuides() const;

    bool eventFilter(QObject *obj, QEvent *event);

public Q_SLOTS:
    void setShowGuides(bool value);
    void setLockGuides(bool value);
    void setSnapToGuides(bool value);

private:
    void setGuidesDataImpl(const KoGuidesData &value);
    void attachEventFilterImpl(bool value);
private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_GUIDES_MANAGER_H */
