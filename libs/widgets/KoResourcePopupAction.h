/* This file is part of the KDE project
 * Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
 * Copyright (C) 2012 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
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

#ifndef KORESOURCEPOPUPACTION_H
#define KORESOURCEPOPUPACTION_H

#include <QAction>

#include <QSharedPointer>
#include "kritawidgets_export.h"


class KoShapeBackground;
class KoAbstractResourceServerAdapter;
class QModelIndex;
class KoResource;

class KRITAWIDGETS_EXPORT KoResourcePopupAction : public QAction
{
    Q_OBJECT

public:
    /**
     * Constructs a KoResourcePopupAction (gradient or pattern) with the specified parent.
     *
     * @param parent The parent for this action.
     */
    explicit KoResourcePopupAction(QSharedPointer<KoAbstractResourceServerAdapter>gradientResourceAdapter, QObject *parent = 0);

    /**
     * Destructor
     */
    ~KoResourcePopupAction() override;

    QSharedPointer<KoShapeBackground> currentBackground() const;
    void setCurrentBackground(QSharedPointer<KoShapeBackground> background);

    void setCurrentResource(KoResource *resource);
    KoResource *currentResource() const;

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
