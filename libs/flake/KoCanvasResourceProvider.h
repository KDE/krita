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

namespace KoCanvasResource {

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
        // XXX: Maybe we should move the next section to Kritas classes.
        //      (where there is a new enum with the first value being
        //          Foo = KoCanvasResourceProvider::Krita+1
        HdrExposure,
        CurrentBrush,
        CurrentPattern,
        CurrentGradient,
        CurrentPaintop,
        CurrentPaintopSettings,
        CurrentKritaLayer,
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

    explicit KoCanvasResourceProvider(QObject * parent);
    ~KoCanvasResourceProvider() {}

    void setResource( KoCanvasResource::EnumCanvasResource key, const QVariant & value );

    /// @return a qvariant containing the specified resource or 0 if the
    /// specified resource does not exist.
    QVariant resource(KoCanvasResource::EnumCanvasResource key);


    void setKoColor( KoCanvasResource::EnumCanvasResource key, const KoColor & color );
    KoColor koColor( KoCanvasResource::EnumCanvasResource key );

    void setForegroundColor( const KoColor & color );
    KoColor foregroundColor();

    void setBackgroundColor( const KoColor & color );
    KoColor backgroundColor();

    void setKoID( KoCanvasResource::EnumCanvasResource key, const KoID & id );
    KoID koID(KoCanvasResource::EnumCanvasResource key);

    /// Sets the actual handle radius
    void setHandleRadius( int handleSize );
    /// Returns the actual handle radius
    int handleRadius();

signals:

    void sigResourceChanged(KoCanvasResource::EnumCanvasResource key, const QVariant & res);

private:

    KoCanvasResourceProvider(const KoCanvasResourceProvider&);
    KoCanvasResourceProvider& operator=(const KoCanvasResourceProvider&);

private:
    QVariant m_empty;
    QHash<int, QVariant> m_resources;

};

#endif // KO_CANVAS_RESOURCE_PROVIDER_
