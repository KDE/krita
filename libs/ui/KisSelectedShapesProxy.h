/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSELECTEDSHAPESPROXY_H
#define KISSELECTEDSHAPESPROXY_H

#include <QObject>
#include <QScopedPointer>
#include <KoSelectedShapesProxy.h>

class KoShapeManager;

class KisSelectedShapesProxy : public KoSelectedShapesProxy
{
    Q_OBJECT
public:
    KisSelectedShapesProxy(KoShapeManager *globalShapeManager);
    ~KisSelectedShapesProxy() override;

    void setShapeManager(KoShapeManager *manager);

    KoSelection *selection() override;


Q_SIGNALS:
    void selectionChanged();
    void selectionContentChanged();
    void currentLayerChanged(const KoShapeLayer *layer);


private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISSELECTEDSHAPESPROXY_H
