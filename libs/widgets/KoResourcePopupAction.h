/*
 * Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
 * SPDX-FileCopyrightText: 2012 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
 * SPDX-FileCopyrightText: 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KORESOURCEPOPUPACTION_H
#define KORESOURCEPOPUPACTION_H

#include <QAction>

#include <QSharedPointer>

#include <KoResource.h>

class KoShapeBackground;
class QModelIndex;

class KoCanvasResourcesInterface;
using KoCanvasResourcesInterfaceSP = QSharedPointer<KoCanvasResourcesInterface>;

#include "kritawidgets_export.h"

class KRITAWIDGETS_EXPORT KoResourcePopupAction : public QAction
{
    Q_OBJECT

public:
    /**
     * Constructs a KoResourcePopupAction (gradient or pattern) with the specified parent.
     *
     * @param gradientResourceAdapter pointer to the gradient or pattern
     * @param parent The parent for this action.
     */
    explicit KoResourcePopupAction(const QString &resourceType, KoCanvasResourcesInterfaceSP canvasResourcesInterface, QObject *parent = 0);

    /**
     * Destructor
     */
    ~KoResourcePopupAction() override;

    QSharedPointer<KoShapeBackground> currentBackground() const;
    void setCurrentBackground(QSharedPointer<KoShapeBackground> background);

    void setCurrentResource(KoResourceSP resource);
    KoResourceSP currentResource() const;

    void setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface);

Q_SIGNALS:
    /// Emitted when a resource was selected
    void resourceSelected(QSharedPointer<KoShapeBackground>  background);

public Q_SLOTS:
    void updateIcon();

private Q_SLOTS:
    void indexChanged(const QModelIndex &modelIndex);

private:
    class Private;
    Private * const d;
};

#endif /* KORESOURCEPOPUPACTION_H */
