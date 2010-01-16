/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KRS_MODULE_H
#define KRS_MODULE_H

#include <QString>
#include <QVariant>
#include <QObject>
#include "krosskritacore_export.h"
#include <KoScriptingModule.h>

class KisView2;

namespace Scripting
{

/**
 * The main Module class enables access to the Krita
 * functionality from within the supported Kross scripting
 * backends like for example Python or Ruby.
 */
class KROSSKRITACORE_EXPORT Module : public KoScriptingModule
{
    Q_OBJECT
public:
    Module(KisView2* view);
    virtual ~Module();

    virtual KoDocument* doc();

public slots:

    /**
    * Returns the \a Progress object which could be
    * used to display a progressbar in Krita to visualize the
    * progress your script makes.
    *
    * Example (in Python) :
    * @code
    * import Krita
    * progress = Krita.progress()
    * progress.setProgressTotalSteps(100)
    * for i in range(100):
    *     progress.incProgress()
    * @endcode
    * Example (in Ruby) :
    * @code
    * require 'Krita'
    * progress = Krita.progress()
    * progress.setProgressTotalSteps(100)
    * for i in 0..100
    *   progress.incProgress()
    * end
    * @endcode
    */
    QObject* progress();

    /**
    * This function returns the \a Image associated with the
    * currently loaded document.
    *
    * Example (in Ruby) :
    * @code
    * require 'Krita'
    * image = Krita.image()
    * @endcode
    */
    QObject* image();

    /**
    * This function returns the current active layer.
    *
    * Example (in Ruby) :
    * @code
    * require 'Krita'
    * layer = Krita.activeLayer()
    * @endcode
    */
    QObject* activeLayer();

    /**
     * This function returns a new \a Image object.
     * It takes four arguments :
     *  - width of the new image
     *  - height of the new image
     *  - colorspace id (e.g. "RGBA" or "CMYK")
     *  - the name of the new image
     *
     * For example (in Ruby) :
     * @code
     * require 'Krita'
     * Krita.createImage(10, 20, "RGBA", "kikoo")
     * @endcode
     */
    QObject* createImage(int width, int height, const QString& colorSpaceModel, const QString& colorSpaceDepth, const QString& name);

    /**
     * This function return a new \a Color object with the given RGB triplet.
     * It takes three arguments :
     *  - red color (0 to 255)
     *  - blue color (0 to 255)
     *  - green color (0 to 255)
     *
     * For example (in Ruby) :
     * @code
     * require "Krita"
     * redcolor = Krita.createRGBColor(255,0,0)
     * whitecolor = Krita.createRGBColor(255,255,255)
     * @endcode
     */
    QObject* createRGBColor(int r, int g, int b);

    /**
     * This function return a new \a Color object with the given HSV triplet.
     * It takes three arguments :
     *  - hue color (0 to 255)
     *  - saturation color (0 to 255)
     *  - value color (0 to 255)
     *
     * For example (in Ruby) :
     * @code
     * require "Krita"
     * color = Krita.createHSVColor(255,125,0)
     * @endcode
     */
    QObject* createHSVColor(int hue, int saturation, int value);

    /**
     * This function return a \a Pattern taken from the list of resources
     * of Krita.
     * It takes one argument :
     *  - the name of the pattern
     *
     * For example (in Ruby) :
     * @code
     * require "Krita"
     * brickspattern = Krita.pattern("Bricks")
     * @endcode
     */
    QObject* pattern(const QString& patternname);
#if 0
    /**
     * This function return a \a Brush taken from the list of resources
     * of Krita.
     * It takes one argument :
     *  - the name of the brush
     *
     * For example (in Ruby) :
     * @code
     * require "Krita"
     * circlebrush = Krita.brush("Circle (05)")
     * @endcode
     */
    QObject* brush(const QString& brushname);

    /**
     * This function return a Brush with a circular shape
     * It takes four arguments :
     *  - width
     *  - height
     *  - width of the shading
     *  - height of the shading
     *
     * If the shading isn't specified, no shading will be used.
     *
     * For example (in Ruby) :
     * @code
     * require "Krita"
     * plaincircle = Krita.createCircleBrush(10,20,0,0)
     * gradientcircle = Krita.createCircleBrush(10,20,5,10)
     * @endcode
     */
    QObject* createCircleBrush(uint w, uint h, uint hf, uint vf);

    /**
     * This function return a Brush with a rectangular shape
     * It takes four arguments :
     *  - width of the brush
     *  - height of the brush
     *  - width of the shading
     *  - height of the shading
     *
     * If the shading isn't specified, no shading will be used.
     *
     * For example (in Ruby) :
     * @code
     * require "Krita"
     * plainrectangle = Krita.createRectBrush(10,20,0,0)
     * gradientrectangle = Krita.createRectBrush(10,20,5,10)
     * @endcode
     */
    QObject* createRectBrush(uint w, uint h, uint hf, uint vf);
#endif
    /**
     * This function return a \a Filter object taken from the list
     * of resources of Krita.
     * It takes one argument :
     *  - the name of the filter
     *
     * For example (in Ruby) :
     * @code
     * require "Krita"
     * invertfilter = Krita.getFilter("invert")
     * @endcode
     */
    QObject* filter(const QString& filtername);
#if 0
    /**
     * This function loads a Brush and then returns it.
     * It takes one argument: the filename of the brush.
     */
    QObject* loadBrush(const QString& filename);
#endif
    /**
     * This function loads a Pattern and then returns it.
     * It takes one argument: the filename of the pattern.
     */
    QObject* loadPattern(const QString& filename);

#if 0
    /**
     * This function return the directory where the script is located.
     */
    Kross::Api::Object::Ptr getPackagePath(Kross::Api::List::Ptr);
private:
    QString m_packagePath;
#endif
    /**
     * @return the view associated with this plugin
     */
    QWidget* view();
private:
    /// \internal d-pointer class.
    class Private;
    /// \internal d-pointer instance.
    Private* const d;
};

}

#endif
