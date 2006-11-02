/*
 *  Copyright (c) 2006 Boudewijn Rempt
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KO_CANVAS_RESOURCE_PROVIDER_
#define KO_CANVAS_RESOURCE_PROVIDER_

#include <QObject>
#include <QHash>
#include <QVariant>

enum enumCanvasResource {
    FOREGROUND_COLOR = 0,
    BACKGROUND_COLOR = 1,
    COMPOSITE_OPERARION = 2,
    COMPOSITE_OPACITY = 4,
    // XXX: Extend with necessary items, or make base numbers per canvas type as with QEvent::type?
    // like:
    KRITA_CANVAS_BASE = 1000,
    HDR_EXPOSURE = 1001,
    CURRENT_KIS_BRUSH = 1002,
    CURRENT_PATTERN = 1003,
    CURRENT_GRADIENT = 1004,
    CURRENT_PAINTOP = 1005,
    CURRENT_PAINTOP_SETTINGS = 1007,
    KIVIO_CANVAS_BASE = 2000
};

/**
 * A single resource, lika a color.
 *
 * XXX: This is a copy of libs/kotext/styles/Styles_p.h -- should
 *      we generalize this, or does Qt already have something
 *      suitable?
 */
class KoCanvasResource {

public:

    KoCanvasResource() {}

    KoCanvasResource(enumCanvasResource key, const QVariant &value)
        {
            this->key = key;
            this->value = value;
        }

    inline bool operator==(const KoCanvasResource &res) const
        {
            return res.key == key && res.value == value;
        }

    inline bool operator!=(const KoCanvasResource &res) const
        {
            return res.key != key || res.value != value;
        }

    enumCanvasResource key;
    QVariant value;
};

/**
 * The KoCanvasResourceProvider contains a set of per-canvas
 * &reserties, like current foreground color, current background
 * color and more. All tools belonging to the current canvas are
 * notified when a Resource changes (is set).
 */
class KoCanvasResourceProvider : public QObject {

    Q_OBJECT

public:

    KoCanvasResourceProvider(QObject * parent);

    // XXX: Who is going to delete the resource objects? This class?
    // The QHash hashmap? The creator?
    void setResource( KoCanvasResource & res);

    void setResource( enumCanvasResource key, const QVariant & value );

    /// @return a pointer to the specified resource or 0 if the
    /// specified resource does not exist.
    QVariant resource(enumCanvasResource key);

signals:

    void sigResourceChanged(const KoCanvasResource & res);

private:

    ~KoCanvasResourceProvider() {};
    KoCanvasResourceProvider(const KoCanvasResourceProvider&);
    KoCanvasResourceProvider& operator=(const KoCanvasResourceProvider&);

private:
    QVariant m_empty;
    QHash<int, KoCanvasResource> m_resources;

};

#endif // KO_CANVAS_RESOURCE_PROVIDER_
