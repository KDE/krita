/*
 *  Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_GMIC_TESTS_H_
#define _KIS_GMIC_TESTS_H_

#include <QtTest>
#include <QImage>
#include <gmic.h>

// #define RUN_FILTERS

class KisGmicBlacklister;

class KisGmicFilterSetting;
class Component;

class FilterDescription;

class KisGmicTests : public QObject
{
    Q_OBJECT

private:
    bool filterWithGmic(KisGmicFilterSetting * gmicFilterSetting, const QString &fileName, gmic_list<float> &images);

    QString filePathify(const QString &filterName);
    bool isAlreadyThere(QString fileName);

    void verifyFilters(QVector<FilterDescription> filters);
    void generateXmlDump();

private:
    Component * m_root;
    QImage m_qimage;
    gmic_list<float> m_images;
    gmic_image<float> m_gmicImage;
    QString m_blacklistFilePath;
    QString m_filterDefinitionsXmlFilePath;
    KisGmicBlacklister * m_blacklister;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

#ifdef RUN_FILTERS
    void testColorizeFilter();
#endif
    /**
     * This test case tests our parser of @file gmic_def.gmic definitions. These definitions are translated to gmic command
     * and compared to similar output produced by G'MIC for GIMP plug-in.
     */
     void testCompareToGmicGimp();

    /**
     * This test case tests our parser of @file gmic_def.gmic definitions. These definitions are translated to gmic command
     * and compared to what our parser parsed . It helps to spot regressions when updating gmic.
     */
     void testCompareToKrita();

    /**
     * If you define RUN_FILTERS in compilation, it will try to run all filters on specified image.
     * This is off by default, because it takes longer time and it is important to run it like this only sometimes (e.g. when gmic is updated).
     * It is used for finding filters that might crash Krita due to unsupported feature required from gmic
     *
     */
    void testAllFilters();
    void testBlacklister();
    void testBlacklisterSearchByParamName();
    void testGatherLayers();

    void testConvertGrayScaleGmic();
    void testConvertGrayScaleAlphaGmic();
    void testConvertRGBgmic();
    void testConvertRGBAgmic();

    void testFilterOnlySelection();

    void testLoadingGmicCommands();


};

#endif
