/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
