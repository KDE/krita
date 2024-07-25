/*
 *  SPDX-FileCopyrightText: 2024 Freya Lupen <penguinflyer2222@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_DOCKER_HUD_H
#define __KIS_DOCKER_HUD_H

#include <QScopedPointer>
#include <QWidget>

class KisDockerHud : public QWidget
{
    Q_OBJECT
public:
    KisDockerHud(QString borrowerName, QString configId);
    ~KisDockerHud() override;

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

public Q_SLOTS:
    void slotUpdateIcons();
    void borrowOrReturnDocker();
    void returnDocker(bool beingTaken = false);
    void borrowDocker();
    void setIsShown(bool isShown);

private Q_SLOTS:
    void showDockerConfig();
    void writeDockerList(QList<QVariant>);
    void readDockerList();
    void writeCurrentDocker();
    QString readCurrentDocker();
    void tryConnectToDockers();
    void showBorrowerLabel(QString borrowerName);
    void hideBorrowerLabel();

private:
    struct Private;
    const QScopedPointer<Private> m_d;

    static QHash<QString, QList<QString>> borrowedWidgetOwners;
};

#endif /* __KIS_DOCKER_HUD_H */
