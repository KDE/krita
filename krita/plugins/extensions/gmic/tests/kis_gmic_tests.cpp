/*
 *  Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_gmic_tests.h"


#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <KoColorSpaceRegistry.h>
#include <kis_image.h>
#include <KoColorSpace.h>
#include <kis_surrogate_undo_adapter.h>

#include <qtest_kde.h>
#include <QImage>
#include <QTextDocument>

#include <gmic.h>
#include <kis_gmic_parser.h>
#include <Component.h>
#include <kis_gmic_filter_model.h>
#include <Command.h>
#include <kis_gmic_simple_convertor.h>
#include <kis_gmic_blacklister.h>

#include <kglobal.h>
#include <kstandarddirs.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif

//#define RUN_FILTERS

using namespace cimg_library;

const static QString EXTENSION = ".bmp";

void KisGmicTests::initTestCase()
{
    KGlobal::dirs()->addResourceType("gmic_definitions", "data", "krita/gmic/");

    QString standardSettings("gmic_def.gmic");
    QString definitionFilePath = KGlobal::mainComponent().dirs()->findResource("gmic_definitions", standardSettings);
    m_blacklistFilePath = KGlobal::mainComponent().dirs()->findResource("gmic_definitions", standardSettings + ".blacklist");

    KisGmicParser parser(definitionFilePath);
    m_root = parser.createFilterTree();

    m_blacklister = new KisGmicBlacklister(m_blacklistFilePath);

    m_qimage = QImage(QString(FILES_DATA_DIR)+"/"+"poster_rodents_bunnysize.jpg");
    m_qimage = m_qimage.convertToFormat(QImage::Format_ARGB32);

    m_gmicImage.assign(m_qimage.width(),m_qimage.height(),1,4); // rgba

    KisGmicSimpleConvertor::convertFromQImage(m_qimage, m_gmicImage);;

    m_images.assign(1);
}

void KisGmicTests::cleanupTestCase()
{
    const unsigned int zero(0);
    m_images.assign(zero);
    delete m_blacklister;
    delete m_root;
}

void KisGmicTests::testAllFilters()
{

    QQueue<Component *> q;
    q.enqueue(m_root);

    KisGmicFilterSetting filterSettings;
    int filters = 0;

    int failed = 0;
    int success = 0;
    while (!q.isEmpty())
    {
        Component * c = q.dequeue();
        if (c->childCount() == 0)
        {
            Command * cmd = static_cast<Command *>(c);
            cmd->writeConfiguration(&filterSettings);
            //qDebug() << "Filter: " << c->name() << filterSettings.gmicCommand();
            if (!filterSettings.gmicCommand().startsWith("-_none_"))
            {
                filters++;

#ifdef RUN_FILTERS
                QString filterName = toPlainText(cmd->name());
                QString categoryName = toPlainText(cmd->parent()->name()); // parent is category

                if (isAlreadyThere( filePathify( filterName ) ))
                {
                    qDebug() << "Already works, skipping filter" << filterName;
                    success++;
                }
                else if (m_blacklister->isBlacklisted(filterName, categoryName))
                {
                    qDebug() << "Blacklisted filter, increase fails" << filterName;
                    failed++;
                }
                else
                {
                    qDebug() << "Filtering with:";
                    qDebug() << QString("<category name=\"%0\">").arg(categoryName);
                    qDebug() << QString("<filter name=\"%0\" />").arg(filterName);
                    bool result = filterWithGmic(&filterSettings, filterName);
                    result ? success++ : failed++;
                    qDebug() << "Progress status:" << "Failed:" << failed << " Success: " << success;
                }
#endif
            }
        }
        else
        {
            for (int i=0; i < c->childCount(); i++)
            {
                q.enqueue(c->child(i));
            }
        }
    }

    QCOMPARE(filters,260);
}


bool KisGmicTests::filterWithGmic(KisGmicFilterSetting* gmicFilterSetting, const QString &filterName)
{
    QString fileName = filePathify(filterName);
    qDebug() << "Filename for filter " << filterName << " : " << fileName;
    // clear previous data?
    m_images.assign(1);
    // copy?
    m_images._data[0] = m_gmicImage;

    // Second step : Call G'MIC API to process input images.
    //------------------------------------------------------
    std::fprintf(stderr,"\n- 2st step : Call G'MIC interpreter.\n");

    gmic_list<char> images_names;
    try
    {
        QString gmicCmd = "-+ -n 0,255 ";
        gmicCmd.append(gmicFilterSetting->gmicCommand());
        gmic(gmicCmd.toLocal8Bit().constData(), m_images, images_names);
    }
    // Catch exception, if an error occured in the interpreter.
    catch (gmic_exception &e)
    {
        dbgPlugins << "\n- Error encountered when calling G'MIC : '%s'\n" << e.what();
        return false;
    }

    // Third step : get back modified image data.
    //-------------------------------------------
    std::fprintf(stderr,"\n- 3st step : Returned %u output images.\n",m_images._width);
    for (unsigned int i = 0; i<m_images._width; ++i)
    {
        std::fprintf(stderr,"   Output image %u = %ux%ux%ux%u, buffer : %p\n",i,
                    m_images._data[i]._width,
                    m_images._data[i]._height,
                    m_images._data[i]._depth,
                    m_images._data[i]._spectrum,
                    m_images._data[i]._data);
    }

    // Forth step : convert to QImage and save
    KisGmicSimpleConvertor convertor;
    bool preserveAlpha(true);
    KisPaintDeviceSP device = convertor.convertFromGmicImage(m_images._data[0], preserveAlpha);
    QImage result = device->convertToQImage(0);
    bool isSaved = result.save(QString(FILES_OUTPUT_DIR) + QDir::separator() + fileName + EXTENSION);
    if (!isSaved)
    {
        qDebug() << "Saving " << fileName  << "failed";
    }

    return true;
}

QString KisGmicTests::filePathify(const QString& fileName)
{
    QStringList illegalCharacters;
    illegalCharacters << QLatin1String("/")
    << QLatin1String("\\")
    << QLatin1String("?")
    << QLatin1String(":");

    QString result = fileName;
    foreach(const QString& item, illegalCharacters)
    {
        result = result.replace(item ,"_");
    }
    return result;
}

void KisGmicTests::testBlacklister()
{
    QString category("Arrays & tiles");
    QString filterName("Tileable rotation");
    m_blacklister->dump();
    QVERIFY(m_blacklister->isBlacklisted(filterName, category));
}

bool KisGmicTests::isAlreadyThere(QString fileName)
{
    QString path = QString(FILES_OUTPUT_DIR) + QDir::separator() + fileName + EXTENSION;
    QFileInfo info(path);
    return info.exists();
}


QString KisGmicTests::toPlainText(const QString& htmlText)
{
    QTextDocument doc;
    doc.setHtml(htmlText);
    return doc.toPlainText();
}


QTEST_KDEMAIN(KisGmicTests, NoGUI)

#include "kis_gmic_tests.moc"
