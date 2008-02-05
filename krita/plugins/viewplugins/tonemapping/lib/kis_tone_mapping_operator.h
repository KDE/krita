/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_TONE_MAPPING_OPERATOR_H_
#define _KIS_TONE_MAPPING_OPERATOR_H_

class KisBookmarkedConfigurationManager;
class KisPropertiesConfiguration;
class KisToneMappingOperatorConfigurationWidget;
class QRect;
class QString;
class QWidget;
class KoColorSpace;

#include "kis_types.h"
#define PUBLISH_OPERATOR(name) \
    class OperatorFactory { \
        public: \
            OperatorFactory() \
            { \
                 KisToneMappingOperatorsRegistry::instance()->add(new name()); \
            } \
    }; \
    static OperatorFactory operatorFactory;

class KisToneMappingOperator {
    public:
        KisToneMappingOperator(QString _id, QString _name);
        virtual ~KisToneMappingOperator();
        QString id() const;
        QString name() const;
        virtual KisToneMappingOperatorConfigurationWidget* createConfigurationWidget(QWidget*) const;
        virtual void toneMap(KisPaintDeviceSP, KisPropertiesConfiguration* config) const = 0;
        /**
         * @return the color space used by this color space
         */
        virtual const KoColorSpace* colorSpace() const = 0;
        /**
         * @return the bookmark manager for this operator
         */
        KisBookmarkedConfigurationManager* bookmarkManager();
        /**
         * @return the bookmark manager for this operator
         */
        const KisBookmarkedConfigurationManager* bookmarkManager() const;
        /**
         * This function return the configuration set as the default by the user or the 
         * default configuration from the filter writer as returned by factoryConfiguration.
         * This configuration is used by default for the configuration widget and to the
         * process function if there is no configuration widget.
         * @return the default configuration of this widget
         */
        virtual KisPropertiesConfiguration * defaultConfiguration() const;
    protected:
        void applyLuminance(KisPaintDeviceSP src, KisPaintDeviceSP lumi, const QRect& r) const;
        /// @return the default configuration as defined by whoever wrote the plugin
        virtual KisPropertiesConfiguration* factoryConfiguration() const;
    private:
        QString configEntryGroup();
    private:
        struct Private;
        Private* const d;
};

#endif
