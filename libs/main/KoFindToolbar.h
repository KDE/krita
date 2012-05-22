/* This file is part of the KDE project
 *
 * Copyright (c) 2010 Arjen Hiemstra <ahiemstra@heimr.nl>
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



#ifndef KOFINDTOOLBAR_H
#define KOFINDTOOLBAR_H

#include <QWidget>
#include "komain_export.h"

class KActionCollection;
class KoFindBase;
/**
 * \brief A search toolbar widget.
 *
 * This class implements a search toolbar containing an autocomplete text
 * field, previous/next buttons and an options selector. It uses the methods
 * provided by KoFindBase to find and navigate search matches.
 */
class KOMAIN_EXPORT KoFindToolbar : public QWidget
{
    Q_OBJECT
public:
    /**
     * Constructor.
     *
     * \param find The KoFindBase instance this toolbar operates on.
     * \param ac An action collection that will get several find-related actions added to it.
     */
    explicit KoFindToolbar(KoFindBase *find, KActionCollection *ac, QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~KoFindToolbar();

public Q_SLOTS:
    /**
     * Set focus to this widget, show it and hide replace related features.
     */
    void activateSearch();

    /**
     * Set focus to this widget, show it including replace related features.
     */
    void activateReplace();
    
private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void matchFound());
    Q_PRIVATE_SLOT(d, void noMatchFound());
    Q_PRIVATE_SLOT(d, void searchWrapped(bool direction));
    Q_PRIVATE_SLOT(d, void addToHistory());
    Q_PRIVATE_SLOT(d, void find(const QString &pattern));
    Q_PRIVATE_SLOT(d, void optionChanged());
    Q_PRIVATE_SLOT(d, void replace());
    Q_PRIVATE_SLOT(d, void replaceAll());
    Q_PRIVATE_SLOT(d, void inputTimeout());
};

#endif // KOFINDTOOLBAR_H
