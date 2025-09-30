/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISROOTSURFACEINFOPROXY_H
#define KISROOTSURFACEINFOPROXY_H

#include <kritaui_export.h>

#include <QObject>
#include <QPointer>

class KisSRGBSurfaceColorSpaceManager;
class QWidget;
class QWindow;
class QEvent;
class KoColorProfile;

class KRITAUI_EXPORT KisRootSurfaceInfoProxy : public QObject
{
    Q_OBJECT
public:
    KisRootSurfaceInfoProxy(QWidget *watched, QObject *parent = nullptr);
    ~KisRootSurfaceInfoProxy();
    const KoColorProfile* rootSurfaceProfile() const;
    bool isReady() const;

    QString colorManagementReport() const;
    QString osPreferredColorSpaceReport() const;

Q_SIGNALS:
    void sigRootSurfaceProfileChanged(const KoColorProfile *profile) const;

private:

    bool eventFilter(QObject *watched, QEvent *event) override;

    void tryUpdateHierarchy();
    void tryReconnectSurfaceManager();
    void tryUpdateRootSurfaceProfile();

    QVector<QPointer<QObject>> getCurrentHierarchy(QWidget *wdg);

    void reconnectToHierarchy(const QVector<QPointer<QObject>> newHierarchy);

private:
    QWidget *m_watched {nullptr};
    QVector<QPointer<QObject>> m_watchedHierarchy;

    QPointer<QWidget> m_topLevelWidgetWithSurface;

    QPointer<QWindow> m_topLevelNativeWindow;
    QPointer<QObject> m_childChangedFilter;

    QPointer<KisSRGBSurfaceColorSpaceManager> m_topLevelSurfaceManager;
    QMetaObject::Connection m_surfaceManagerConnection;

    const KoColorProfile* m_rootSurfaceProfile {nullptr};

};

#endif /* KISROOTSURFACEINFOPROXY_H */
