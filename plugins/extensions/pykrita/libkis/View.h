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
 * View
 */
class KRITALIBKIS_EXPORT View : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(View)

    Q_PROPERTY(Window* Window READ window WRITE setWindow)
    Q_PROPERTY(Document* Document READ document WRITE setDocument)
    Q_PROPERTY(bool Visible READ visible WRITE setVisible)
    Q_PROPERTY(Canvas* Canvas READ canvas WRITE setCanvas)

public:
    explicit View(KisView *view, QObject *parent = 0);
    virtual ~View();

    Window* window() const;
    void setWindow(Window* value);

    Document* document() const;
    void setDocument(Document* value);

    bool visible() const;
    void setVisible(bool value);

    Canvas* canvas() const;
    void setCanvas(Canvas* value);

public Q_SLOTS:

    void close(bool confirm);

private:
    struct Private;
    Private *const d;

};

#endif // LIBKIS_VIEW_H
