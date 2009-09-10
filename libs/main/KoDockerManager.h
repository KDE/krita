/* This file is part of the KDE project
 *
 * Copyright (c) 2008 Casper Boemann <cbr@boemann.dk>
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
#ifndef DOCKER_MANAGER_H
#define DOCKER_MANAGER_H

#include "komain_export.h"

#include <QObject>
#include <QMap>

class KoView;

/**
   The docker manager makes sure that tool option widgets are shown at the right time.
 */
class KOMAIN_EXPORT KoDockerManager : public QObject
{
    Q_OBJECT
public:
    explicit KoDockerManager(KoView* mainWindow);
    ~KoDockerManager();

public slots:
    void removeUnusedOptionWidgets();
    /**
     * Update the option widgets to the argument ones, removing the currently set widgets.
     */
    void newOptionWidgets(const QMap<QString, QWidget *> & optionWidgetMap, QWidget *callingView);


private:
    class Private;
    Private * const d;
};

#endif
