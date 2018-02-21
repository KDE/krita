/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KOSHAPEFILLRESOURCECONNECTOR_H
#define KOSHAPEFILLRESOURCECONNECTOR_H

#include <QObject>
#include <QScopedPointer>

class KoCanvasBase;

class KoShapeFillResourceConnector : public QObject
{
    Q_OBJECT
public:
    explicit KoShapeFillResourceConnector(QObject *parent = 0);
    ~KoShapeFillResourceConnector();

    void connectToCanvas(KoCanvasBase *canvas);
    void disconnect();

private Q_SLOTS:
    void slotCanvasResourceChanged(int key, const QVariant &value);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KOSHAPEFILLRESOURCECONNECTOR_H
