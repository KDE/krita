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

#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <KoColorSpaceRegistry.h>
#include <kis_image.h>
#include <KoColorSpace.h>
#include <KoResourcePaths.h>
#include <KoColorModelStandardIds.h>
#include <KoColor.h>
#include <kis_surrogate_undo_adapter.h>

#include <QTest>
#include <QImage>
#include <QTextDocument>


#include <kis_gmic_parser.h>
#include <Component.h>
#include <kis_gmic_filter_model.h>
#include <Command.h>
#include <kis_gmic_simple_convertor.h>
#include <kis_gmic_blacklister.h>
#include <kis_input_output_mapper.h>
#include <kis_gmic_applicator.h>


#include <KoResourcePaths.h>
#include <QDomDocument>

#include <kis_group_layer.h>
#include <kis_painter.h>
#include <kis_selection.h>
#include <commands/kis_set_global_selection_command.h>
#include <kis_processing_applicator.h>
#include <testutil.h>

#include "kis_gmic_tests.h"
#include <gmic.h>
#include <Category.h>


#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif


class FilterDescription
{
public:
        QString filterName;
        QString category;
        QString gmicCommand;
};

using namespace cimg_library;

const static QString EXTENSION = ".png";
const static QString STANDARD_SETTINGS("update1610.gmic");
const static QString BLACKLIST("gmic_def.gmic.blacklist");

void KisGmicTests::initTestCase()
{
    KoResourcePaths::addResourceType("gmic_definitions", "data", "krita/gmic/");

    QString standardSettings("gmic_def.gmic");
    QString definitionFilePath = KoResourcePaths::findResource("gmic_definitions", STANDARD_SETTINGS);
    m_blacklistFilePath = KoResourcePaths::findResource("gmic_definitions", STANDARD_SETTINGS + ".blacklist");

    QStringList filePaths;
    filePaths << definitionFilePath;

    KisGmicParser parser(filePaths);
    m_root = parser.createFilterTree();

    m_blacklistFilePath = KoResourcePaths::findResource("gmic_definitions", BLACKLIST);
    m_blacklister = new KisGmicBlacklister(m_blacklistFilePath);

    m_qimage = QImage(QString(FILES_DATA_DIR)+"/"+"poster_rodents_bunnysize.jpg");
    m_qimage = m_qimage.convertToFormat(QImage::Format_ARGB32);

    m_gmicImage.assign(m_qimage.width(),m_qimage.height(),1,4); // rgba

    KisGmicSimpleConvertor::convertFromQImage(m_qimage, m_gmicImage);

    m_images.assign(1);


}

void KisGmicTests::cleanupTestCase()
{
    const unsigned int zero(0);
    m_images.assign(zero);
    delete m_blacklister;
    delete m_root;
}

#ifdef RUN_FILTERS
void KisGmicTests::testColorizeFilter()
{
    QString filterName = "Colorize [comics]";
    QString filterCategory = "Black & white";

    Command * cmd = KisGmicBlacklister::findFilter(m_root, filterCategory, filterName);
    KisGmicFilterSetting filterSettings;
    if (cmd == 0)
    {
            dbgKrita << "Filter not found!";
    }else
    {
        cmd->setParameter("Input layers", "Lineart + color spots");
        cmd->setParameter("Output layers", "Lineart + color spots + extrapolated colors");
        cmd->setParameter("Smoothness", "0.05");
        cmd->writeConfiguration(&filterSettings);
    }

    QVERIFY(!filterSettings.gmicCommand().isEmpty());

    // load 3 layers here
    QStringList layerNames;
    layerNames << "02_Artline.png";
    layerNames << "01_ColorMarks.png";
    layerNames << "00_BG.png";

    m_images.assign(layerNames.size());

    int layerIndex = 0;
    for (int i = 0; i < layerNames.size(); i++)
    {
        const QString& layerName = layerNames.at(i);
        QImage layerImage = QImage(QString(FILES_DATA_DIR) + QDir::separator() + layerName).convertToFormat(QImage::Format_ARGB32);
        gmic_image<float> gmicImage;
        gmicImage.assign(layerImage.width(), layerImage.height(), 1, 4);

        KisGmicSimpleConvertor::convertFromQImage(layerImage, gmicImage);
        m_images[layerIndex] = gmicImage;
        layerIndex++;
    }

    filterWithGmic(&filterSettings, "multi_colorize", m_images);
}
#endif


#define VERBOSE
typedef QPair<QString, QString> StringPair;
void KisGmicTests::testBlacklisterSearchByParamName()
{
    QVector<StringPair> paramFilters;
    paramFilters << StringPair("1st additional palette (.gpl file)","Colorize [interactive]");
    paramFilters << StringPair("2nd additional palette (.gpl file)","Colorize [interactive]");
    paramFilters << StringPair("Clut filename","User-defined");
    paramFilters << StringPair("Filename","Import data");

    bool beVerbose = false;
#ifdef VERBOSE
    beVerbose = true;
#endif


    for (int i = 0; i < paramFilters.size(); i++)
    {
        const QString& paramName = paramFilters.at(i).first;
        const QString& expectedFilterName = paramFilters.at(i).second;
        if (beVerbose)
        {
            dbgKrita << "Searching for filter by parameter name :" << paramName;
        }

        QList<Command *> filters = KisGmicBlacklister::findFilterByParamName(m_root, paramName, "file");
        if (filters.size() == 0)
        {
            dbgKrita << "Can't find filter by param name: " << paramName;
        }
        QCOMPARE(filters.size(), 1);

        Command * c = filters.at(0);
        QCOMPARE(c->name(), expectedFilterName);
        if (beVerbose)
        {
            dbgKrita << "FilterName: << \"" + c->name() + "\" << \"" + KisGmicBlacklister::toPlainText(c->parent()->name()) + "\"" << " passed!";
        }
    }
}

void KisGmicTests::testFindFilterByPath()
{
    Category * path = new Category;
    path->setName("<i>About</i>");

    Command * cmd = new Command(path);
    cmd->setName("Contributors");
    path->add(cmd);

    Component * c = KisGmicBlacklister::findFilterByPath(m_root, path);
    QVERIFY(c != 0);
    QVERIFY(c->name() == cmd->name());

    delete path;
}



void KisGmicTests::testGatherLayers()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();

    QImage background(QString(FILES_DATA_DIR) + QDir::separator() + "00_BG.png");
    QImage colorMarks(QString(FILES_DATA_DIR) + QDir::separator() + "01_ColorMarks.png");
    QImage artLine(QString(FILES_DATA_DIR) + QDir::separator() + "02_Artline.png");

    KisImageSP image = new KisImage(0, 2408, 3508, colorSpace, "filter test");

    KisPaintDeviceSP device1 = new KisPaintDevice(colorSpace);
    KisPaintDeviceSP device2 = new KisPaintDevice(colorSpace);
    KisPaintDeviceSP device3 = new KisPaintDevice(colorSpace);
    device1->convertFromQImage(background, 0, 0, 0);
    device2->convertFromQImage(colorMarks, 0, 0, 0);
    device3->convertFromQImage(artLine, 0, 0, 0);

    KisLayerSP paintLayer1 = new KisPaintLayer(image, "background", OPACITY_OPAQUE_U8, device1);
    KisLayerSP paintLayer2 = new KisPaintLayer(image, "colorMarks", OPACITY_OPAQUE_U8, device2);
    KisLayerSP paintLayer3 = new KisPaintLayer(image, "artLine", OPACITY_OPAQUE_U8, device3);

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(paintLayer2, image->rootLayer());
    image->addNode(paintLayer3, image->rootLayer());

    KisNodeSP activeNode = static_cast<KisNodeSP>(paintLayer2);

    KisNodeListSP result;
    KisInputOutputMapper mapper(image, activeNode);

    result = mapper.inputNodes(ACTIVE_LAYER);
    QCOMPARE(result->at(0)->name(), activeNode->name());

    result = mapper.inputNodes(ACTIVE_LAYER_ABOVE_LAYER);
    QCOMPARE(result->size(), 2);
    QCOMPARE(result->at(0)->name(), activeNode->name());
    QCOMPARE(result->at(1)->name(), paintLayer3->name());

    result = mapper.inputNodes(ACTIVE_LAYER_BELOW_LAYER);
    QCOMPARE(result->size(), 2);
    QCOMPARE(result->at(0)->name(), activeNode->name());
    QCOMPARE(result->at(1)->name(), paintLayer1->name());

    result = mapper.inputNodes(ALL_LAYERS);
    QCOMPARE(result->size(), 3);
    QCOMPARE(result->at(0)->name(), paintLayer3->name());
    QCOMPARE(result->at(1)->name(), paintLayer2->name());
    QCOMPARE(result->at(2)->name(), paintLayer1->name());
}

void KisGmicTests::testAllFilters()
{

    QQueue<Component *> q;
    q.enqueue(m_root);

    KisGmicFilterSetting filterSettings;
    int filterCount = 0;

    int failed = 0;
    int success = 0;

    QVector<QString> failedFilters;

    while (!q.isEmpty())
    {
        Component * c = q.dequeue();
        if (c->childCount() == 0)
        {
            Command * cmd = static_cast<Command *>(c);
            cmd->writeConfiguration(&filterSettings);
            //dbgKrita << "Filter: " << c->name() << filterSettings.gmicCommand();
            if (!filterSettings.gmicCommand().startsWith("-_none_"))
            {
                filterCount++;

#ifdef RUN_FILTERS
                QString filterName = KisGmicBlacklister::toPlainText(cmd->name());
                QString categoryName = KisGmicBlacklister::toPlainText(cmd->parent()->name()); // parent is category

                if (isAlreadyThere( filePathify( filterName ) ))
                {
                    dbgKrita << "Already works, skipping filter" << filterName;
                    success++;
                }
                else if (m_blacklister->isBlacklisted(filterName, categoryName))
                {
                    dbgKrita << "Blacklisted filter, increase fails" << filterName;
                    failed++;
                    failedFilters.append(categoryName+":"+filterName);
                }
                else
                {
                    dbgKrita << "Filtering with:";
                    dbgKrita << QString("<category name=\"%0\">").arg(categoryName);
                    dbgKrita << QString("<filter name=\"%0\" />").arg(filterName);

                    // clear previous data?
                    m_images.assign(1);
                    // copy?
                    m_images._data[0] = m_gmicImage;
                    bool result = filterWithGmic(&filterSettings, filterName, m_images);
                    result ? success++ : failed++;
                    dbgKrita << "Progress status:" << "Failed:" << failed << " Success: " << success;
                    if (result == false)
                    {
                        failedFilters.append(categoryName+":"+filterName);
                    }
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

#ifdef RUN_FILTERS
    dbgKrita << "Finish status:" << "Failed:" << failed << " Success: " << success;
    dbgKrita << "== Failed filters ==";
    foreach (const QString &item, failedFilters)
    {
        dbgKrita << item;
    }


#else
    Q_UNUSED(success);
    Q_UNUSED(failed);
#endif
}


bool KisGmicTests::filterWithGmic(KisGmicFilterSetting* gmicFilterSetting, const QString &filterName, gmic_list<float> &images)
{
    QString fileName = filePathify(filterName);
    dbgKrita << "Filename for filter " << filterName << " : " << fileName;
    // Second step : Call G'MIC API to process input images.
    //------------------------------------------------------
    std::fprintf(stderr,"\n- 2st step : Call G'MIC interpreter.\n");
    gmic_list<char> images_names;
    try
    {
        QString gmicCmd = "-* 255 ";
        gmicCmd.append(gmicFilterSetting->gmicCommand());
        gmic(gmicCmd.toLocal8Bit().constData(), images, images_names);
    }
    // Catch exception, if an error occured in the interpreter.
    catch (gmic_exception &e)
    {
        dbgPlugins << "\n- Error encountered when calling G'MIC : '%s'\n" << e.what();
        return false;
    }

    // Third step : get back modified image data.
    //-------------------------------------------
    std::fprintf(stderr,"\n- 3st step : Returned %u output images.\n", images._width);
    for (unsigned int i = 0; i < images._width; ++i)
    {
        std::fprintf(stderr,"   Output image %u = %ux%ux%ux%u, buffer : %p\n",i,
                    images._data[i]._width,
                    images._data[i]._height,
                    images._data[i]._depth,
                    images._data[i]._spectrum,
                    images._data[i]._data);
    }

    // Forth step : convert to QImage and save
    for (unsigned int i = 0; i < images._width; ++i)
    {
        KisGmicSimpleConvertor convertor;
        KisPaintDeviceSP device = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
        convertor.convertFromGmicImage(images._data[i], device, 255.0f);
        QImage result = device->convertToQImage(0, 0,0,images._data[i]._width, images._data[i]._height);
        QString indexString("_%1");
        if (images._width > 1)
        {
            indexString = indexString.arg(i,4, 10, QLatin1Char('_'));
        }else
        {
            indexString = QString();
        }
        QString fullFileName(QString(FILES_OUTPUT_DIR) + QDir::separator() + fileName + indexString + ".png");
        QFileInfo info(fullFileName);
        if (info.exists())
        {
            continue;
        }

        bool isSaved = result.save(fullFileName);
        if (!isSaved)
        {
            dbgKrita << "Saving " << fileName  << "failed";
            return false;
        }else
        {
            dbgKrita << "Saved " << fullFileName << " -- OK";
        }
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
    QString category("Colors");
    QString filterName("User-defined");
    QVERIFY(m_blacklister->isBlacklisted(filterName, category));
}

bool KisGmicTests::isAlreadyThere(QString fileName)
{
    QString path = QString(FILES_OUTPUT_DIR) + QDir::separator() + fileName + EXTENSION;
    QFileInfo info(path);
    return info.exists();
}

void KisGmicTests::testConvertGrayScaleGmic()
{
    KisPaintDeviceSP resultDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    KisPaintDeviceSP resultDevFast = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    gmic_image<float> gmicImage;
    gmicImage.assign(m_qimage.width(),m_qimage.height(),1,1);

    KisGmicSimpleConvertor::convertFromQImage(m_qimage, gmicImage, 1.0);
    KisGmicSimpleConvertor::convertFromGmicImage(gmicImage, resultDev, 1.0);
    KisGmicSimpleConvertor::convertFromGmicFast(gmicImage, resultDevFast, 1.0);


    QImage slowQImage = resultDev->convertToQImage(0,0,0,gmicImage._width, gmicImage._height);
    QImage fastQImage = resultDevFast->convertToQImage(0,0,0,gmicImage._width, gmicImage._height);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint,slowQImage,fastQImage))
    {
        QFAIL(QString("Slow method produces different result then fast to convert gmic grayscale pixel format, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
        slowQImage.save("grayscale.bmp");
        fastQImage.save("grayscale_fast.bmp");
    }
}

void KisGmicTests::testConvertGrayScaleAlphaGmic()
{
    KisPaintDeviceSP resultDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    KisPaintDeviceSP resultDevFast = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    gmic_image<float> gmicImage;
    gmicImage.assign(m_qimage.width(),m_qimage.height(),1,2);

    KisGmicSimpleConvertor::convertFromQImage(m_qimage, gmicImage, 1.0);
    KisGmicSimpleConvertor::convertFromGmicImage(gmicImage, resultDev, 1.0);
    KisGmicSimpleConvertor::convertFromGmicFast(gmicImage, resultDevFast, 1.0);

    QImage slowQImage = resultDev->convertToQImage(0, 0,0,gmicImage._width, gmicImage._height);
    QImage fastQImage = resultDevFast->convertToQImage(0,0,0,gmicImage._width, gmicImage._height);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint,slowQImage,fastQImage))
    {
        QFAIL(QString("Slow method produces different result then fast to convert gmic grayscale pixel format, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
        slowQImage.save("grayscale.bmp");
        fastQImage.save("grayscale_fast.bmp");
    }
}

void KisGmicTests::testConvertRGBgmic()
{
    KisPaintDeviceSP resultDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    KisPaintDeviceSP resultDevFast = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    gmic_image<float> gmicImage;
    gmicImage.assign(m_qimage.width(),m_qimage.height(),1,3);

    KisGmicSimpleConvertor::convertFromQImage(m_qimage, gmicImage, 1.0);
    KisGmicSimpleConvertor::convertFromGmicImage(gmicImage, resultDev, 1.0);
    KisGmicSimpleConvertor::convertFromGmicFast(gmicImage, resultDevFast, 1.0);

    QImage slowQImage = resultDev->convertToQImage(0,0,0,gmicImage._width, gmicImage._height);
    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint,slowQImage,m_qimage))
    {
        QFAIL(QString("Slow method failed to convert gmic RGB pixel format, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
        slowQImage.save("RGB.bmp");
    }

    QImage fastQImage = resultDevFast->convertToQImage(0,0,0,gmicImage._width, gmicImage._height);
    if (!TestUtil::compareQImages(errpoint,fastQImage,m_qimage))
    {
        QFAIL(QString("Fast method failed to convert gmic RGB pixel format, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
        fastQImage.save("RGB_fast.bmp");
    }
}


void KisGmicTests::testConvertRGBAgmic()
{
    KisPaintDeviceSP resultDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    KisPaintDeviceSP resultDevFast = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());

    gmic_image<float> gmicImage;
    gmicImage.assign(m_qimage.width(),m_qimage.height(),1,4);

    KisGmicSimpleConvertor::convertFromQImage(m_qimage, gmicImage, 1.0);
    KisGmicSimpleConvertor::convertFromGmicImage(gmicImage, resultDev, 1.0);
    KisGmicSimpleConvertor::convertFromGmicFast(gmicImage, resultDevFast, 1.0);

    QImage slowQImage = resultDev->convertToQImage(0,0,0,gmicImage._width, gmicImage._height);
    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint,slowQImage,m_qimage))
    {
        QFAIL(QString("Slow method failed to convert gmic RGBA pixel format, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
        slowQImage.save("RGBA.bmp");
    }

    QImage fastQImage = resultDevFast->convertToQImage(0,0,0,gmicImage._width, gmicImage._height);
    if (!TestUtil::compareQImages(errpoint,fastQImage,m_qimage))
    {
        QFAIL(QString("Fast method failed to convert gmic RGBA pixel format, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
        fastQImage.save("RGBA_fast.bmp");
    }
}


void KisGmicTests::testFilterOnlySelection()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
    int width = 3840;
    int height = 2400;
    KisImageSP image = new KisImage(undoStore, width, height, colorSpace, "filter selection test");

    QImage simpleImage(QString(FILES_DATA_DIR) + QDir::separator() + "poster_rodents_bunnysize.jpg");
    KisPaintDeviceSP device = new KisPaintDevice(colorSpace);
    device->convertFromQImage(simpleImage, 0, 0, 0);

    KisLayerSP paintLayer = new KisPaintLayer(image, "background", OPACITY_OPAQUE_U8, device);
    image->addNode(paintLayer, image->rootLayer());

    // add global selection

    QPoint topLeft(0.25 * width, 0.25 * height);
    QPoint bottomRight(0.75 * width, 0.75 * height);

    QRect selectionRect(topLeft, bottomRight);
    KisSelectionSP selection = new KisSelection(new KisSelectionDefaultBounds(0, image));
    KisPixelSelectionSP pixelSelection = selection->pixelSelection();
    pixelSelection->select(selectionRect);

    KUndo2Command *cmd = new KisSetGlobalSelectionCommand(image, selection);
    image->undoAdapter()->addCommand(cmd);

    QRect selected = image->globalSelection()->selectedExactRect();
    QCOMPARE(selectionRect, selected);

    // filter kis image with gmic

    KisNodeListSP kritaNodes(new QList< KisNodeSP >());
    kritaNodes->append(static_cast<KisNodeSP>(paintLayer));

    // select some simple filter
    QString filterName = "Sepia";
    QString filterCategory = "Colors";

    Command * gmicCmd = KisGmicBlacklister::findFilter(m_root, filterCategory, filterName);

    KisGmicFilterSetting filterSettings;
    if (gmicCmd == 0)
    {
            dbgKrita << "Filter not found!";
    }
    else
    {
        gmicCmd->writeConfiguration(&filterSettings);
    }

    QVERIFY(!filterSettings.gmicCommand().isEmpty());

    QString gmicCommand = filterSettings.gmicCommand();

    KisGmicApplicator applicator;
    applicator.setProperties(image, image->root(), kundo2_noi18n("Gmic filter"), kritaNodes, gmicCommand);
    applicator.preview();
    applicator.finish();
    image->waitForDone();

    //image->convertToQImage(image->bounds(), 0).save("filteredSelection.png");
    //TODO: check with expected image!

}


void KisGmicTests::testLoadingGmicCommands()
{
    QString definitionFilePath = KoResourcePaths::findResource("gmic_definitions", STANDARD_SETTINGS);
    QByteArray data = KisGmicParser::extractGmicCommandsOnly(definitionFilePath);
    QVERIFY(data.size() > 0);
}


void KisGmicTests::generateXmlDump(const QString &outputFilePath)
{
    QDomDocument doc = KisGmicBlacklister::dumpFiltersToXML(m_root);
    QFile outputFile(outputFilePath);
    if (outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream ts(&outputFile);
        ts << doc.toString();
        outputFile.close();
    }
}

void KisGmicTests::testCompareToGmicGimp()
{
    QVector<FilterDescription> filterDescriptions;
    QDomDocument doc("myDoc");

    QFile file(QString(FILES_DATA_DIR)+"/"+"gmic_def_1610_gimp.xml");
    QVERIFY(file.open(QIODevice::ReadOnly));
    QVERIFY(doc.setContent(&file));

    QDomNodeList children = doc.documentElement().childNodes();
    for (int i = 0; i < children.size(); i++)
    {
        if (children.at(i).isElement())
        {
            QDomElement elem = children.at(i).toElement();
            QDomElement cmdElem = elem.firstChildElement("gmicCommand");
            QVERIFY(!cmdElem.isNull());

            FilterDescription fd;
            fd.filterName = elem.attribute("name");
            fd.category = QString();
            fd.gmicCommand = cmdElem.text();
            // symbolic home to real directory
            fd.gmicCommand = fd.gmicCommand.replace("[[:home:]]", QDir::homePath());
            fd.gmicCommand = fd.gmicCommand.replace("\n", "\\n");
            filterDescriptions.append(fd);
        }
    }

    verifyFilters(filterDescriptions);
}

typedef QPair<Component *, QString> FilterPair;
void KisGmicTests::testCompareToKrita()
{
    QString fileName = "gmic_def_" + QString::number(int(gmic_version)) + "_krita.xml";
    QString filePath = QString(FILES_DATA_DIR)+"/"+fileName;

    QFileInfo info(filePath);
    if (!info.exists())
    {
        warnKrita << "Reference xml file for the krita parser does not exist, creating one!";
        warnKrita << "Creating it at " << filePath;
        generateXmlDump(filePath);
    }

    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));

    // Load the xml file that Krita produced
    // select filter, category, command
    QDomDocument doc("mydocument");
    QVERIFY(doc.setContent(&file));

    QVector<FilterPair> filterDescriptions;

    QDomElement child = doc.documentElement().firstChildElement();
    QQueue<QDomElement> queue;
    queue.enqueue(child);
    while (!queue.isEmpty())
    {
        QDomElement elem = queue.dequeue();
        if (elem.tagName() == "filter")
        {
            if (elem.parentNode().isElement())
            {
                QDomElement cmdElem = elem.firstChildElement("gmicCommand");
                QVERIFY(!cmdElem.isNull());
                QString gmicCommand = cmdElem.text().replace("[[:home:]]", QDir::homePath());

                Command * command = new Command;
                command->setName(elem.attribute("name"));

                QDomElement iter = elem.parentNode().toElement();
                Category * currentCategory = 0;
                while (iter.isElement())
                {

                    if (iter.tagName() == "category")
                    {
                        if (iter.attribute("name") == "Filters")
                        {
                            break;
                        }
                        Category * category = new Category;
                        category->setName(iter.attribute("name"));
                        if (currentCategory == 0)
                        {
                            category->add(command);
                            command->setParent(category);
                        }
                        else
                        {
                            category->add(currentCategory);
                            currentCategory->setParent(category);
                        }
                        currentCategory = category;
                    }
                    else
                    {
                            warnKrita << "Unexpected element #20151819 " << iter.tagName();
                    }
                    iter = iter.parentNode().toElement();
                }

                FilterPair pair = FilterPair(currentCategory, gmicCommand);
                filterDescriptions.append(pair);
            }
        }
        else
        {
            QDomNodeList children = elem.childNodes();
            for (int i = 0; i < children.size(); i++)
            {
                if (children.at(i).isElement())
                {
                    queue.enqueue(children.at(i).toElement());
                }
            }
        }
    }

    //verifyFilters(filterDescriptions);

    int success = 0;
    for (int i = 0; i < filterDescriptions.size(); i++)
    {
        Component * path = filterDescriptions.at(i).first;
        const QString &gmicCommand = filterDescriptions.at(i).second;

        Component * component = KisGmicBlacklister::findFilterByPath(m_root, path);
        if (!component)
        {
            qDebug () << "Can't find filter: " << pathToString(path);
        }
        else
        {
            KisGmicFilterSetting filterSettings;
            static_cast<Command *>(component)->writeConfiguration(&filterSettings);

            if (filterSettings.gmicCommand() != gmicCommand)
            {
                dbgKrita << "Filter  : " <<  pathToString(path);
                dbgKrita << "  Actual: " << filterSettings.gmicCommand();
                dbgKrita << "Expected: " << gmicCommand;
            }
            else
            {
                success++;
            }
        }
    }

    int count = filterDescriptions.size();
    if (success != count)
    {
        dbgKrita << "Number of failed filters: " << count - success;
    }

    QCOMPARE(success, count);

    for (int i = 0; i < filterDescriptions.size(); i++)
    {
        delete filterDescriptions[i].first;
    }
}

QString KisGmicTests::pathToString(Component* c)
{
    QStringList pathItems;
    Component * iter = c;
    while (iter->childCount() > 0)
    {
        pathItems << iter->name();
        iter = iter->child(0);
    }
    pathItems << iter->name();


    return pathItems.join(" / ");
}


void KisGmicTests::verifyFilters(QVector< FilterDescription > filters)
{
    // without knowing the category (the dumped xml file for gmic gimp does not contain
    // this information, because it is non-trivial to extract it automatically)
    // we will try to find matching filters

    int count = filters.size();


    // Expected failures due to buggy filter definitions
    QSet<QString> expectedFilterFailures;
    expectedFilterFailures
    << "Ellipsionism" // MINOR: buggy filter definition: default value bigger than max
    << "Simple local contrast" //  TODO: bug in our parser
    << "Granular texture" // MINOR
    << "Repair scanned document" // MINOR
    << "Cartoon [animated]" // MINOR
    << "3d video conversion" // TODO: bug in our parser
    << "Luminance to alpha" //buggy filter definition
    << "Generate film data" // buggy filter definition (folder param)
    <<  "Motifs degrades cie"; // buggy filter definition (default value bigger than max)

    int success = 0;
    int expectedFail = 0;
    for (int i = 0; i < count; i++)
    {
        FilterDescription fd = filters.at(i);

        QVector<Command *> commands;
        if (fd.category.isEmpty())
        {
            commands = KisGmicBlacklister::filtersByName(m_root, fd.filterName);
            if (commands.size() == 0)
            {
                warnKrita << "Can't find "<< "filterName: " << fd.filterName;
                continue;
            }

            #if 0
            if (commands.size() > 1)
            {

                dbgKrita << "Multiple entries found:";
                foreach (Command * c, commands)
                {
                    dbgKrita << "filter: " << c->name() << "category: " << c->parent()->name();
                }
            }
            #endif
        }
        else
        {
            Command * c = KisGmicBlacklister::findFilter(m_root, fd.category, fd.filterName);
            if (c == 0)
            {
                warnKrita << "Can't find "<< "filterName: " << fd.filterName << "category: " << fd.category;
                continue;
            }else
            {
                commands.append(c);
            }
        }

        bool foundMatch = false;
        QString lastGmicCommand;

        QString errors;
        for (int i = 0; i < commands.size(); i++)
        {
            QTextStream errorLog(&errors);
            Component * c = commands[i];
            QVERIFY(c->childCount() == 0);
            Command * cmd = dynamic_cast<Command *>(c);
            QVERIFY(cmd != 0);

            KisGmicFilterSetting filterSettings;
            cmd->writeConfiguration(&filterSettings);
            lastGmicCommand = filterSettings.gmicCommand();

            if (filterSettings.gmicCommand() == fd.gmicCommand)
            {
                foundMatch = true;
            }
        }

        if (foundMatch)
        {
            success++;
        }
        else
        {
            if (expectedFilterFailures.contains(fd.filterName))
            {
                expectedFail++;
            }
            else
            {
                dbgKrita << "Filter " << fd.category << " / " << fd.filterName << " does not match any  of " << commands.size() << " filter definitions!";
                dbgKrita << "Expected: " << fd.gmicCommand;
                if (commands.size() == 1)
                {
                    dbgKrita << "  Actual: " << lastGmicCommand;
                }
                else
                {
                    dbgKrita << "=== BEGIN ==";
                    dbgKrita << errors;
                    dbgKrita << "===  END  ===";
                }
            }

        }
    }

    if (success != count)
    {
        dbgKrita << "=== Stats ===";
        dbgKrita << "Number of failed filters (with expected failures): " << count - success;
        dbgKrita << "Number of expected failures:" << expectedFail;
    }

    int realSuccess = success + expectedFail;
    QCOMPARE(realSuccess,count);
}

void KisGmicTests::testFindFilterByName()
{
    QVector<Command *> commands = KisGmicBlacklister::filtersByName(m_root, "B&W stencil");
    QVERIFY(commands.size() > 0);
}



QTEST_MAIN(KisGmicTests)

