/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KO_COLOR_PROFILE_H_
#define _KO_COLOR_PROFILE_H_

#include <QString>

#include <pigment_export.h>

/**
 * Contains information needed for color transformation.
 */
class PIGMENT_EXPORT KoColorProfile {
        struct Private;
    public:
        /**
         * @param fileName file name to load or save that profile
         */
        KoColorProfile(QString fileName = "");
        virtual ~KoColorProfile();
        /**
         * Load the profile in memory.
         * @return true if the profile has been successfully loaded
         */
        virtual bool load();
        
        /**
         * Override this function to save the profile.
         * @param fileName destination
         * @return true if the profile has been successfully saved
         */
        virtual bool save(QString fileName);
    
        /**
         * @return true if the profile is valid, false if it isn't been loaded in memory yet, or
         * if the loaded memory is a bad profile
         */
        virtual bool valid() const =0;
        
        /**
         * @return the name of this profile
         */
        QString name() const;
        /**
         * @return the info of this profile
         */
        QString info() const;
        /**
         * @return the filename of the profile (it might be empty)
         */
        QString fileName() const;
        /**
         * @param filename new filename
         */
        void setFileName(const QString &filename);
        
        /**
         * @return true if you can use this profile can be used to convert color from a different
         * profile to this one
         */
        virtual bool isSuitableForOutput() const =0;
        /**
         * @return true if this profile is suitable to use for printing
         */
        virtual bool isSuitableForPrinting() const =0;
        /**
         * @return true if this profile is suitable to use for display
         */
        virtual bool isSuitableForDisplay() const =0;
    protected:
        /**
         * Allows to define the name of this profile.
         */
        void setName(const QString &name);
        /**
         * Allows to set the information string of that profile.
         */
        void setInfo(const QString &info);
    private:
        Private* const d;
};

#endif
