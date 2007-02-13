/*
   Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */
#ifndef KO_CANVAS_RESOURCE_PROVIDER_
#define KO_CANVAS_RESOURCE_PROVIDER_

#include <QObject>
#include <QHash>
#include <QVariant>

#include <KoID.h>
#include <flake_export.h>
class KoColor;

/**
 * The KoCanvasResource contains a set of per-canvas
 * properties, like current foreground color, current background
 * color and more.
 */
namespace KoCanvasResource {

    /**
     * This enum holds identifiers to the resources that can be stored in here.
     */
    enum EnumCanvasResource {
        ForegroundColor,    ///< The active forground color selected for this canvas.
        BackgroundColor,    ///< The active background color selected for this canvas.
        CompositeOperation,
        CompositeOpacity,
        HandleRadius,       ///< The handle radius used for drawing handles of any kind
        KarbonStart = 1000,      ///< Base number for karbon specific values.
        KexiStart = 2000,        ///< Base number for kexi specific values.
        KivioStart = 3000,       ///< Base number for kivio specific values.
        KPlatoStart = 4000,      ///< Base number for kplato specific values.
        KPresenterStart = 5000,  ///< Base number for kpresenter specific values.
        KritaStart = 6000,       ///< Base number for krita specific values.
        KSpreadStart = 7000,     ///< Base number for kspread specific values.
        KWordStart = 8000        ///< Base number for kword specific values.
    };

}

/**
 * The KoCanvasResourceProvider contains a set of per-canvas
 * properties, like current foreground color, current background
 * color and more. All tools belonging to the current canvas are
 * notified when a Resource changes (is set).
 */
class FLAKE_EXPORT KoCanvasResourceProvider : public QObject {

    Q_OBJECT

public:

    /**
     * Constructor.
     * @param parent the parent QObject, used for memory management.
     */
    explicit KoCanvasResourceProvider(QObject * parent);
    ~KoCanvasResourceProvider() {}

    /**
     * Set a resource of any type.
     * @param key the integer key, based on KoCanvasResource::EnumCanvasResource
     * @param value the new value for the key.
     */
    void setResource( int key, const QVariant & value );
    void setResource( int key, const KoColor & color );
    void setResource( int key, const KoID & id );

    /**
     * Returns a qvariant containing the specified resource or a standard one if the
     * specified resource does not exist.
     * @param key the key.
     */
    QVariant resource(int key);

    /**
     * Set the foregroundColor resource.
     * @param color the new foreground color
     */
    void setForegroundColor( const KoColor & color );
    /**
     * Return the foregroundColor
     */
    KoColor foregroundColor();

    /**
     * Set the backgroundColor resource.
     * @param color the new background color
     */
    void setBackgroundColor( const KoColor & color );
    /**
     * Return the backgroundColor
     */
    KoColor backgroundColor();

    /**
     * Tools that provide a handle for controlling the content that the tool can edit can
     * use this property to alter the radius that a circular handle should have on screen.
     * @param handleSize the radius in pixels.
     */
    void setHandleRadius( int handleSize );
    /// Returns the actual handle radius
    int handleRadius();

    /**
     * Return the resource determined by param key as a boolean.
     * @param key the indentifying key for the resource.
     */
    bool boolResource(int key) const;

    /**
     * Return the resource determined by param key as an integer.
     * @param key the indentifying key for the resource.
     */
    bool intResource(int key) const;

    /**
     * Return the resource determined by param key as a double.
     * @param key the indentifying key for the resource.
     */
    bool doubleResource(int key) const;

    /**
     * Return the resource determined by param key as a KoColor.
     * @param key the indentifying key for the resource.
     */
    KoColor koColorResource( int key );

    /**
     * Return the resource determined by param key as a KoID.
     * @param key the indentifying key for the resource.
     */
    KoID koIDResource(int key);

signals:
    /**
     * This signal is emitted every time a resource is set that is either
     * new or different from the previous set value.
     * @param key the indentifying key for the resource
     * @param value the variants new value.
     */
    void sigResourceChanged(int key, const QVariant & value);

private:

    KoCanvasResourceProvider(const KoCanvasResourceProvider&);
    KoCanvasResourceProvider& operator=(const KoCanvasResourceProvider&);

private:
    class Private;
    Private *const d;
};

#endif // KO_CANVAS_RESOURCE_PROVIDER_
