/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef LIBKIS_VIEW_H
#define LIBKIS_VIEW_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"

class KisView;

/**
 * View represents one view on a document. A document can be 
 * shown in more than one view at a time.
 */
class KRITALIBKIS_EXPORT View : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(View)

public:
    explicit View(KisView *view, QObject *parent = 0);
    ~View() override;
    
    bool operator==(const View &other) const;
    bool operator!=(const View &other) const;

public Q_SLOTS:

    /**
     * @return the window this view is shown in.
     */
    Window* window() const;

    /**
     * @return the document this view is showing.
     */    
    Document* document() const;

    /**
     * @return true if the current view is visible, false if not.
     */
    bool visible() const;
    
    /**
     * Make the current view visible.
     */
    void setVisible();

    /**
     * @return the canvas this view is showing. The canvas controls
     * things like zoom and rotation.
     */
    Canvas* canvas() const;

    /**
     * @brief activateResource activates the given resource.
     * @param resource: a pattern, gradient or paintop preset
     */
    void activateResource(Resource *resource);

private:

    friend class Window;
    KisView *view();

    struct Private;
    Private *const d;

};

#endif // LIBKIS_VIEW_H
