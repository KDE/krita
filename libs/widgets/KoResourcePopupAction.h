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

#include <KAction>

class KoShapeBackground;
class KoAbstractResourceServerAdapter;
class QModelIndex;

class KoResourcePopupAction : public KAction
{
    Q_OBJECT

public:
    /**
     * Constructs a KoResourcePopupAction (gradient or pattern) with the specified parent.
     *
     * @param parent The parent for this action.
     */
    explicit KoResourcePopupAction(KoAbstractResourceServerAdapter *gradientResourceAdapter, QObject *parent = 0);

    /**
     * Destructor
     */
    virtual ~KoResourcePopupAction();

    KoShapeBackground *currentBackground();
    void setCurrentBackground(KoShapeBackground *background);

signals:
    /// Emitted when a resource was selected
    void resourceSelected(KoShapeBackground * background);

public slots:
    void updateIcon();

private slots:
    void indexChanged(QModelIndex modelIndex);

private:
    class Private;
    Private * const d;
};

#endif /* KORESOURCEPOPUPACTION_H */
