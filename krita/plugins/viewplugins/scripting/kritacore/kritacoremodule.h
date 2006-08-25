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

#ifndef KRITA_KROSS_KRITACOREMODULE_H
#define KRITA_KROSS_KRITACOREMODULE_H

#include <QString>
#include <QVariant>
#include <QObject>

#define KROSS_MAIN_EXPORT KDE_EXPORT

#if 0

#include <api/class.h>
#include <api/module.h>

namespace Kross { namespace Api {
    class Manager;
}}

#endif

class KisView;

namespace Kross { namespace KritaCore {

    /**
     * The main KritaCoreModule class enables access to the Krita
     * functionality from within the Kross scripting backends.
     */
    class KritaCoreModule : public QObject
    {
            Q_OBJECT
        public:
            KritaCoreModule(KisView* view);
            virtual ~KritaCoreModule();

        public slots:

            /**
            * Returns the \a KoApplicationAdaptor object.
            */
            QObject* application();

#if 0
            /**
             * This function return a new Image.
             * It takes four arguments :
             *  - width
             *  - height
             *  - colorspace id
             *  - name of the image
             *
             * And in return you get an Image object.
             * 
             * For example (in ruby) :
             * @code
             * Krosskritacore::newImage(10,20, "RGBA", "kikoo")
             * @endcode
             */
            Kross::Api::Object::Ptr newImage(Kross::Api::List::Ptr);
            /**
             * This function return a new Color with the given RGB triplet
             * It takes three arguments :
             *  - red color (0 to 255)
             *  - blue color (0 to 255)
             *  - green color (0 to 255)
             * 
             * For example (in ruby) :
             * @code
             * Krosskritacore::newRGBColor(255,0,0) # create a red color
             * Krosskritacore::newRGBColor(255,255,255) # create a white color
             * @endcode
             */
            Kross::Api::Object::Ptr newRGBColor(Kross::Api::List::Ptr);
            /**
             * This function return a new Color with the given HSV triplet
             * It takes three arguments :
             *  - hue color (0 to 255)
             *  - saturation color (0 to 255)
             *  - value color (0 to 255)
             * 
             * For example (in ruby) :
             * @code
             * Krosskritacore::newRGBColor(255,125,0)
             * @endcode
             */
            Kross::Api::Object::Ptr newHSVColor(Kross::Api::List::Ptr);
            /**
             * This function return a Pattern taken from the list of ressources
             * of krita
             * It takes one argument :
             *  - the name of the pattern
             * 
             * For example (in ruby) :
             * @code
             * Krosskritacore::getPattern("Bricks")
             * @endcode
             */
            Kross::Api::Object::Ptr getPattern(Kross::Api::List::Ptr);
            /**
             * This function return a Brush taken from the list of ressources
             * of krita
             * It takes one argument :
             *  - the name of the pattern
             * 
             * For example (in ruby) :
             * @code
             * Krosskritacore::getBrush("Circle (05)")
             * @endcode
             */
            Kross::Api::Object::Ptr getBrush(Kross::Api::List::Ptr);
            /**
             * This function return a Brush with a circular shape
             * It takes at least two arguments :
             *  - width
             *  - height
             * 
             * It can takes two other arguments :
             *  - width of the shading
             *  - height of the shading
             * 
             * If the shading isn't specified, no shading will be used.
             * 
             * For example (in ruby) :
             * @code
             * Krosskritacore::newCircleBrush(10,20) # create a plain circle
             * Krosskritacore::newCircleBrush(10,20,5,10) # create a gradient
             * @endcode
             */
            Kross::Api::Object::Ptr newCircleBrush(Kross::Api::List::Ptr);
            /**
             * This function return a Brush with a rectangular shape
             * It takes at least two arguments :
             *  - width
             *  - height
             * 
             * It can takes two other arguments :
             *  - width of the shading
             *  - height of the shading
             * 
             * If the shading isn't specified, no shading will be used.
             * 
             * For example (in ruby) :
             * @code
             * Krosskritacore::newRectBrush(10,20) # create a plain rectangle
             * Krosskritacore::newRectBrush(10,20,5,10) # create a gradient
             * @endcode
             */
            Kross::Api::Object::Ptr newRectBrush(Kross::Api::List::Ptr);
            /**
             * This function return a Filter taken from the list of ressources
             * of krita
             * It takes one argument :
             *  - the name of the filter
             * 
             * For example (in ruby) :
             * @code
             * Krosskritacore::getFilter("invert")
             * @endcode
             */
            Kross::Api::Object::Ptr getFilter(Kross::Api::List::Ptr);
            /**
             * This function loads a Brush and then returns it.
             * It takes one argument: the filename of the brush.
             */
            Kross::Api::Object::Ptr loadBrush(Kross::Api::List::Ptr);
            /**
             * This function loads a Pattern and then returns it.
             * It takes one argument: the filename of the pattern.
             */
            Kross::Api::Object::Ptr loadPattern(Kross::Api::List::Ptr);
            /**
             * This function return the directory where the script is located.
             */
            Kross::Api::Object::Ptr getPackagePath(Kross::Api::List::Ptr);
        private:
            QString m_packagePath;
#endif

        private:
            class Private;
            Private* const d;
    };

}}

#endif
