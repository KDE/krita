/*
 *  SPDX-FileCopyrightText: 2016 Laszlo Fazekas <mneko@freemail.hu>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "csv_loader.h"

#include <QDebug>
#include <QApplication>

#include <QFile>
#include <QVector>
#include <QIODevice>
#include <QStatusBar>
#include <QFileInfo>

#include <KisPart.h>
#include <KisView.h>
#include <KisDocument.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>

#include <kis_debug.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_raster_keyframe_channel.h>
#include <kis_image_animation_interface.h>
#include <kis_time_span.h>

#include "csv_read_line.h"
#include "csv_layer_record.h"

CSVLoader::CSVLoader(KisDocument *doc, bool batchMode)
    : m_image(0)
    , m_doc(doc)
    , m_batchMode(batchMode)
    , m_stop(false)
{
}

CSVLoader::~CSVLoader()
{
}

KisImportExportErrorCode CSVLoader::decode(QIODevice *io, const QString &filename)
{
    QString     field;
    int         idx;
    int         frame = 0;

    QString     projName;
    int         width = 0;
    int         height = 0;
    int         frameCount = 1;
    float       framerate = 24.0;
    float       pixelRatio = 1.0;

    int         projNameIdx = -1;
    int         widthIdx = -1;
    int         heightIdx = -1;
    int         frameCountIdx = -1;
    int         framerateIdx = -1;
    int         pixelRatioIdx = -1;

    QVector<CSVLayerRecord*> layers;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    idx = filename.lastIndexOf(QRegExp("[\\/]"));
    QString base = (idx == -1) ? QString() : filename.left(idx + 1); //include separator
    QString path = filename;

    if (path.right(4).toUpper() == ".CSV")
        path = path.left(path.size() - 4);

    //according to the QT docs, the slash is a universal directory separator
    path.append(".frames/");

    KisImportExportErrorCode retval = ImportExportCodes::OK;

    dbgFile << "pos:" << io->pos();

    CSVReadLine readLine;
    QScopedPointer<KisDocument> importDoc(KisPart::instance()->createDocument());
    importDoc->setInfiniteAutoSaveInterval();
    importDoc->setFileBatchMode(true);

    KisView *setView(0);

    if (!m_batchMode) {
        // TODO: use other systems of progress reporting (KisViewManager::createUnthreadedUpdater()

//        //show the statusbar message even if no view
//        Q_FOREACH (KisView* view, KisPart::instance()->views()) {
//            if (view && view->document() == m_doc) {
//                setView = view;
//                break;
//            }
//        }

//        if (!setView) {
//            QStatusBar *sb = KisPart::instance()->currentMainwindow()->statusBar();
//            if (sb) {
//                sb->showMessage(i18n("Loading CSV file..."));
//            }
//        } else {
//            emit m_doc->statusBarMessage(i18n("Loading CSV file..."));
//        }

//        emit m_doc->sigProgress(0);
//        connect(m_doc, SIGNAL(sigProgressCanceled()), this, SLOT(cancel()));
    }
    int step = 0;

    do {
        qApp->processEvents();

        if (m_stop) {
            retval = ImportExportCodes::Cancelled;
            break;
        }

        if ((idx = readLine.nextLine(io)) <= 0) {
            if ((idx < 0) ||(step < 5))
                retval = ImportExportCodes::FileFormatIncorrect;
            break;
        }
        field = readLine.nextField(); //first field of the line

        if (field.isNull()) continue; //empty row

        switch (step) {

        case 0 :    //skip first row
            step = 1;
            break;

        case 1 :    //scene header names
            step = 2;

            for (idx = 0; !field.isNull(); idx++) {
                if (field == "Project Name") {
                    projNameIdx = idx;

                } else if (field == "Width") {
                    widthIdx = idx;

                } else if (field == "Height") {
                    heightIdx = idx;

                } else if (field == "Frame Count") {
                    frameCountIdx = idx;

                } else if (field == "Frame Rate") {
                    framerateIdx = idx;

                } else if (field == "Pixel Aspect Ratio") {
                    pixelRatioIdx = idx;
                }
                field= readLine.nextField();
            }
            break;

        case 2 :    //scene header values
            step= 3;

            for (idx= 0; !field.isNull(); idx++) {
                if (idx == projNameIdx) {
                    projName = field;

                } else if (idx == widthIdx) {
                    width = field.toInt();

                } else if (idx == heightIdx) {
                    height = field.toInt();

                } else if (idx == frameCountIdx) {
                    frameCount = field.toInt();

                    if (frameCount < 1) frameCount= 1;

                } else if (idx == framerateIdx) {
                    framerate = field.toFloat();

                } else if (idx == pixelRatioIdx) {
                    pixelRatio = field.toFloat();

                }
                field= readLine.nextField();
            }

            if ((width < 1) || (height < 1)) {
               retval = ImportExportCodes::Failure;
               break;
            }

            retval = createNewImage(width, height, pixelRatio, projName.isNull() ? filename : projName);
            break;

        case 3 :    //create level headers
            if (field[0] != '#') break;

            for (; !(field = readLine.nextField()).isNull(); ) {
                CSVLayerRecord* layerRecord = new CSVLayerRecord();
                layers.append(layerRecord);
            }
            readLine.rewind();
            field = readLine.nextField();
            step = 4;
            Q_FALLTHROUGH();

        case 4 :    //level header

            if (field == "#Layers") {
                //layer name
                for (idx = 0; !(field = readLine.nextField()).isNull() && (idx < layers.size()); idx++)
                    layers.at(idx)->name = field;

                break;
            }
            if (field == "#Density") {
                //layer opacity
                for (idx = 0; !(field = readLine.nextField()).isNull() && (idx < layers.size()); idx++)
                    layers.at(idx)->density = field.toFloat();

                break;
            }
            if (field == "#Blending") {
                //layer blending mode
                for (idx = 0; !(field = readLine.nextField()).isNull() && (idx < layers.size()); idx++)
                    layers.at(idx)->blending = field;

                break;
            }
            if (field == "#Visible") {
                //layer visibility
                for (idx = 0; !(field = readLine.nextField()).isNull() && (idx < layers.size()); idx++)
                    layers.at(idx)->visible = field.toInt();

                break;
            }
            if (field == "#Folder") {
                //CSV 1.1 folder location
                for (idx = 0; !(field = readLine.nextField()).isNull() && (idx < layers.size()); idx++)
                    layers.at(idx)->path = validPath(field, base);

                break;
            }
            if ((field.size() < 2) || (field[0] != '#') || !field[1].isDigit()) break;

            step = 5;

            Q_FALLTHROUGH();

        case 5 :    //frames

            if ((field.size() < 2) || (field[0] != '#') || !field[1].isDigit()) break;

            for (idx = 0; !(field = readLine.nextField()).isNull() && (idx < layers.size()); idx++) {
                CSVLayerRecord* layer = layers.at(idx);

                if (layer->last != field) {
                    if (!m_batchMode) {
                        //emit m_doc->sigProgress((frame * layers.size() + idx) * 100 /
                        //                        (frameCount * layers.size()));
                    }
                    retval = setLayer(layer, importDoc.data(), path);
                    layer->last = field;
                    layer->frame = frame;
                }
            }
            frame++;
            break;
        }
    } while (retval.isOk());

    //finish the layers

    if (retval.isOk()) {
        if (m_image) {
            KisImageAnimationInterface *animation = m_image->animationInterface();

            if (frame > frameCount)
                frameCount = frame;

            animation->setFullClipRange(KisTimeSpan::fromTimeToTime(0,frameCount - 1));
            animation->setFramerate((int)framerate);
        }

        for (idx = 0; idx < layers.size(); idx++) {
            CSVLayerRecord* layer = layers.at(idx);
            //empty layers without any pictures are dropped

            if ((layer->frame > 0) || !layer->last.isEmpty()) {
                retval = setLayer(layer, importDoc.data(), path);

                if (!retval.isOk())
                    break;
            }
        }
    }

    if (m_image) {
        //insert the existing layers by the right order
        for (idx = layers.size() - 1; idx >= 0; idx--) {
            CSVLayerRecord* layer = layers.at(idx);

            if (layer->layer) {
                m_image->addNode(layer->layer, m_image->root());
            }
        }
        m_image->unlock();
    }
    qDeleteAll(layers);
    io->close();

    if (!m_batchMode) {
        // disconnect(m_doc, SIGNAL(sigProgressCanceled()), this, SLOT(cancel()));
        // emit m_doc->sigProgress(100);

        if (!setView) {
            QStatusBar *sb = KisPart::instance()->currentMainwindow()->statusBar();
            if (sb) {
                sb->clearMessage();
            }
        } else {
            emit m_doc->clearStatusBarMessage();
        }
    }
    QApplication::restoreOverrideCursor();
    return retval;
}

QString CSVLoader::convertBlending(const QString &blending)
{
    if (blending == "Color") return COMPOSITE_OVER;
    if (blending == "Behind") return COMPOSITE_BEHIND;
    if (blending == "Erase") return COMPOSITE_ERASE;
    // "Shade"
    if (blending == "Light") return COMPOSITE_LINEAR_LIGHT;
    if (blending == "Colorize") return COMPOSITE_COLORIZE;
    if (blending == "Hue") return COMPOSITE_HUE;
    if (blending == "Add") return COMPOSITE_ADD;
    if (blending == "Sub") return COMPOSITE_INVERSE_SUBTRACT;
    if (blending == "Multiply") return COMPOSITE_MULT;
    if (blending == "Screen") return COMPOSITE_SCREEN;
    // "Replace"
    // "Substitute"
    if (blending == "Difference") return COMPOSITE_DIFF;
    if (blending == "Divide") return COMPOSITE_DIVIDE;
    if (blending == "Overlay") return COMPOSITE_OVERLAY;
    if (blending == "Light2") return COMPOSITE_DODGE;
    if (blending == "Shade2") return COMPOSITE_BURN;
    if (blending == "HardLight") return COMPOSITE_HARD_LIGHT;
    if (blending == "SoftLight") return COMPOSITE_SOFT_LIGHT_PHOTOSHOP;
    if (blending == "GrainExtract") return COMPOSITE_GRAIN_EXTRACT;
    if (blending == "GrainMerge") return COMPOSITE_GRAIN_MERGE;
    if (blending == "Sub2") return COMPOSITE_SUBTRACT;
    if (blending == "Darken") return COMPOSITE_DARKEN;
    if (blending == "Lighten") return COMPOSITE_LIGHTEN;
    if (blending == "Saturation") return COMPOSITE_SATURATION;

    return COMPOSITE_OVER;
}

QString CSVLoader::validPath(const QString &path,const QString &base)
{
    //replace Windows directory separators with the universal /

    QString tryPath= QString(path).replace(QString("\\"), QString("/"));
    int i = tryPath.lastIndexOf("/");

    if (i == (tryPath.size() - 1))
        tryPath= tryPath.left(i); //remove the ending separator if exists

    if (QFileInfo(tryPath).isDir())
        return tryPath.append("/");

    QString scan(tryPath);
    i = -1;

    while ((i= (scan.lastIndexOf("/",i) - 1)) > 0) {
        //avoid testing if the next level will be the default xxxx.layers folder

        if ((i >= 6) && (scan.mid(i - 6, 7) == ".layers")) continue;

        tryPath= QString(base).append(scan.mid(i + 2)); //base already ending with a /

        if (QFileInfo(tryPath).isDir())
            return tryPath.append("/");
    }
    return QString(); //NULL string
}

KisImportExportErrorCode CSVLoader::setLayer(CSVLayerRecord* layer, KisDocument *importDoc, const QString &path)
{
    bool result = true;

    if (layer->channel == 0) {
        //create a new document layer

        float opacity = layer->density;

        if (opacity > 1.0)
            opacity = 1.0;
        else if (opacity < 0.0)
            opacity = 0.0;

        const KoColorSpace* cs = m_image->colorSpace();
        const QString layerName = (layer->name).isEmpty() ? m_image->nextLayerName() : layer->name;

        KisPaintLayer* paintLayer = new KisPaintLayer(m_image, layerName,
                                                       (quint8)(opacity * OPACITY_OPAQUE_U8), cs);

        paintLayer->setCompositeOpId(convertBlending(layer->blending));
        paintLayer->setVisible(layer->visible);
        paintLayer->enableAnimation();

        layer->layer = paintLayer;
        layer->channel = qobject_cast<KisRasterKeyframeChannel*>
            (paintLayer->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true));
    }


    if (!layer->last.isEmpty()) {
        //png image
        QString filename = layer->path.isNull() ? path : layer->path;
        filename.append(layer->last);

        result = importDoc->openUrl(QUrl::fromLocalFile(filename),
                                    KisDocument::DontAddToRecent);
        if (result)
            layer->channel->importFrame(layer->frame, importDoc->image()->projection(), 0);

    } else {
        //blank
        layer->channel->addKeyframe(layer->frame);
    }
    return (result) ? ImportExportCodes::OK : ImportExportCodes::Failure;
}

KisImportExportErrorCode CSVLoader::createNewImage(int width, int height, float ratio, const QString &name)
{
    //the CSV is RGBA 8bits, sRGB

    if (!m_image) {
        const KoColorSpace* cs = KoColorSpaceRegistry::instance()->colorSpace(
                                RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), 0);

        if (cs) m_image = new KisImage(m_doc->createUndoStore(), width, height, cs, name);

        if (!m_image) return ImportExportCodes::Failure;

        m_image->setResolution(ratio, 1.0);
        m_image->barrierLock();
    }
    return ImportExportCodes::OK;
}

KisImportExportErrorCode CSVLoader::buildAnimation(QIODevice *io, const QString &filename)
{
    return decode(io, filename);
}

KisImageSP CSVLoader::image()
{
    return m_image;
}

void CSVLoader::cancel()
{
    m_stop = true;
}
