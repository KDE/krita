/* This file is part of the KDE project
 * Copyright 2008 (C) Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kis_kra_saver.h"

#include "kis_kra_tags.h"
#include "kis_kra_save_visitor.h"
#include "kis_kra_savexml_visitor.h"

#include <QApplication>
#include <QMessageBox>
#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include <QStringList>
#include <QScopedPointer>

#include <QUrl>
#include <QBuffer>

#include <KoDocumentInfo.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoColor.h>
#include <KoColorSet.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <KoStore.h>
#include <KoStoreDevice.h>

#include <kis_annotation.h>
#include <kis_image.h>
#include <kis_image_animation_interface.h>
#include <KisImportExportManager.h>
#include <kis_group_layer.h>
#include <kis_layer.h>
#include <kis_adjustment_layer.h>
#include <kis_layer_composition.h>
#include <kis_painting_assistants_decoration.h>
#include <kis_psd_layer_style_resource.h>
#include "kis_png_converter.h"
#include "kis_keyframe_channel.h"
#include <kis_time_range.h>
#include "KisDocument.h"
#include <string>
#include "kis_dom_utils.h"
#include "kis_grid_config.h"
#include "kis_guides_config.h"
#include "KisProofingConfiguration.h"

#include <KisMirrorAxisConfig.h>

#include <QFileInfo>
#include <QDir>


using namespace KRA;

struct KisKraSaver::Private
{
public:
    KisDocument* doc;
    QMap<const KisNode*, QString> nodeFileNames;
    QMap<const KisNode*, QString> keyframeFilenames;
    QString imageName;
    QString filename;
    QStringList errorMessages;
};

KisKraSaver::KisKraSaver(KisDocument* document, const QString &filename)
        : m_d(new Private)
{
    m_d->doc = document;
    m_d->filename = filename;

    m_d->imageName = m_d->doc->documentInfo()->aboutInfo("title");
    if (m_d->imageName.isEmpty()) {
        m_d->imageName = i18n("Unnamed");
    }
}

KisKraSaver::~KisKraSaver()
{
    delete m_d;
}

QDomElement KisKraSaver::saveXML(QDomDocument& doc,  KisImageSP image)
{
    QDomElement imageElement = doc.createElement("IMAGE");

    Q_ASSERT(image);
    imageElement.setAttribute(NAME, m_d->imageName);
    imageElement.setAttribute(MIME, NATIVE_MIMETYPE);
    imageElement.setAttribute(WIDTH, KisDomUtils::toString(image->width()));
    imageElement.setAttribute(HEIGHT, KisDomUtils::toString(image->height()));
    imageElement.setAttribute(COLORSPACE_NAME, image->colorSpace()->id());
    imageElement.setAttribute(DESCRIPTION, m_d->doc->documentInfo()->aboutInfo("comment"));
    // XXX: Save profile as blob inside the image, instead of the product name.
    if (image->profile() && image->profile()-> valid()) {
        imageElement.setAttribute(PROFILE, image->profile()->name());
    }
    imageElement.setAttribute(X_RESOLUTION, KisDomUtils::toString(image->xRes()*72.0));
    imageElement.setAttribute(Y_RESOLUTION, KisDomUtils::toString(image->yRes()*72.0));
    //now the proofing options:
    if (image->proofingConfiguration()) {
        if (image->proofingConfiguration()->storeSoftproofingInsideImage) {
            imageElement.setAttribute(PROOFINGPROFILENAME, KisDomUtils::toString(image->proofingConfiguration()->proofingProfile));
            imageElement.setAttribute(PROOFINGMODEL, KisDomUtils::toString(image->proofingConfiguration()->proofingModel));
            imageElement.setAttribute(PROOFINGDEPTH, KisDomUtils::toString(image->proofingConfiguration()->proofingDepth));
            imageElement.setAttribute(PROOFINGINTENT, KisDomUtils::toString(image->proofingConfiguration()->intent));
            imageElement.setAttribute(PROOFINGADAPTATIONSTATE, KisDomUtils::toString(image->proofingConfiguration()->adaptationState));
        }
    }

    quint32 count = 1; // We don't save the root layer, but it does count
    KisSaveXmlVisitor visitor(doc, imageElement, count, m_d->doc->url().toLocalFile(), true);
    visitor.setSelectedNodes({m_d->doc->preActivatedNode()});

    image->rootLayer()->accept(visitor);
    m_d->errorMessages.append(visitor.errorMessages());

    m_d->nodeFileNames = visitor.nodeFileNames();
    m_d->keyframeFilenames = visitor.keyframeFileNames();

    saveBackgroundColor(doc, imageElement, image);
    saveAssistantsGlobalColor(doc, imageElement);
    saveWarningColor(doc, imageElement, image);
    saveCompositions(doc, imageElement, image);
    saveAssistantsList(doc, imageElement);
    saveGrid(doc, imageElement);
    saveGuides(doc, imageElement);
    saveMirrorAxis(doc, imageElement);
    saveAudio(doc, imageElement);
    savePalettesToXML(doc, imageElement);

    QDomElement animationElement = doc.createElement("animation");
    KisDomUtils::saveValue(&animationElement, "framerate", image->animationInterface()->framerate());
    KisDomUtils::saveValue(&animationElement, "range", image->animationInterface()->fullClipRange());
    KisDomUtils::saveValue(&animationElement, "currentTime", image->animationInterface()->currentUITime());
    imageElement.appendChild(animationElement);

    return imageElement;
}

bool KisKraSaver::savePalettes(KoStore *store, KisImageSP image, const QString &uri)
{
    Q_UNUSED(image);
    Q_UNUSED(uri);

    bool res = false;
    if (m_d->doc->paletteList().size() == 0) {
        return true;
    }
    for (const KoColorSet *palette : m_d->doc->paletteList()) {
        if (!palette->isGlobal()) {
            if (!store->open(m_d->imageName + PALETTE_PATH + palette->filename())) {
                m_d->errorMessages << i18nc("Error message when saving a .kra file", "Could not save palettes.");
                return false;
            }
            QByteArray ba = palette->toByteArray();
            qint64 nwritten = 0;
            if (!ba.isEmpty()) {
                nwritten = store->write(ba);
            } else {
                qWarning() << "Cannot save the palette to a byte array:" << palette->name();
            }
            res = store->close();
            res = res && (nwritten == ba.size());
        }
    }

    if (!res) {
        m_d->errorMessages << i18nc("Error message when saving a .kra file", "Could not save palettes.");
    }
    return res;
}


void KisKraSaver::savePalettesToXML(QDomDocument &doc, QDomElement &element)
{
    QDomElement ePalette = doc.createElement(PALETTES);
    for (const KoColorSet *palette : m_d->doc->paletteList()) {
        if (!palette->isGlobal()) {
            QDomElement eFile =  doc.createElement("palette");
            eFile.setAttribute("filename", palette->filename());
            ePalette.appendChild(eFile);
        }
    }
    element.appendChild(ePalette);
}

bool KisKraSaver::saveKeyframes(KoStore *store, const QString &uri, bool external)
{
    QMap<const KisNode*, QString>::iterator it;

    for (it = m_d->keyframeFilenames.begin(); it != m_d->keyframeFilenames.end(); it++) {
        const KisNode *node = it.key();
        QString filename = it.value();

        QString location =
                (external ? QString() : uri)
                + m_d->imageName + LAYER_PATH + filename;

        if (!saveNodeKeyframes(store, location, node)) {
            return false;
        }
    }

    return true;
}

bool KisKraSaver::saveNodeKeyframes(KoStore *store, QString location, const KisNode *node)
{
    QDomDocument doc = KisDocument::createDomDocument("krita-keyframes", "keyframes", "1.0");
    QDomElement root = doc.documentElement();

    KisKeyframeChannel *channel;
    Q_FOREACH (channel, node->keyframeChannels()) {
        QDomElement element = channel->toXML(doc, m_d->nodeFileNames[node]);
        root.appendChild(element);
    }

    bool success = true;
    if (store->open(location)) {
        QByteArray xml = doc.toByteArray();
        qint64 nwritten = store->write(xml);
        bool r = store->close();
        success = r && (nwritten == xml.size());
    } else {
        success = false;
    }
    if (!success) {
        m_d->errorMessages << i18nc("Error message on saving a .kra file", "Could not save keyframes.");
        return false;
    }

    return true;
}

bool KisKraSaver::saveBinaryData(KoStore* store, KisImageSP image, const QString &uri, bool external, bool autosave)
{
    QString location;

    // Save the layers data
    KisKraSaveVisitor visitor(store, m_d->imageName, m_d->nodeFileNames);

    if (external)
        visitor.setExternalUri(uri);

    image->rootLayer()->accept(visitor);

    m_d->errorMessages.append(visitor.errorMessages());
    if (!m_d->errorMessages.isEmpty()) {
        return false;
    }

    bool success = true;
    bool r = true;
    qint64 nwritten = 0;

    // saving annotations
    // XXX this only saves EXIF and ICC info. This would probably need
    // a redesign of the dtd of the krita file to do this more generally correct
    // e.g. have <ANNOTATION> tags or so.
    bool savingAnnotationsSuccess = true;
    KisAnnotationSP annotation = image->annotation("exif");
    if (annotation) {
        location = external ? QString() : uri;
        location += m_d->imageName + EXIF_PATH;
        if (store->open(location)) {
            nwritten = store->write(annotation->annotation());
            r = store->close();
            savingAnnotationsSuccess = savingAnnotationsSuccess && (nwritten == annotation->annotation().size()) && r;
        } else {
            savingAnnotationsSuccess = false;
        }
    }

    if (!savingAnnotationsSuccess) {
        m_d->errorMessages.append(i18nc("Saving .kra file error message", "Could not save annotations."));
    }

    success = success && savingAnnotationsSuccess;

    bool savingImageProfileSuccess = true;
    if (image->profile()) {
        const KoColorProfile *profile = image->profile();
        KisAnnotationSP annotation;
        if (profile) {
            QByteArray profileRawData = profile->rawData();
            if (!profileRawData.isEmpty()) {
                if (profile->type() == "icc") {
                    annotation = new KisAnnotation(ICC, profile->name(), profile->rawData());
                } else {
                    annotation = new KisAnnotation(PROFILE, profile->name(), profile->rawData());
                }
            }
        }

        if (annotation) {
            location = external ? QString() : uri;
            location += m_d->imageName + ICC_PATH;
            if (store->open(location)) {
                nwritten = store->write(annotation->annotation());
                r = store->close();
                savingImageProfileSuccess = savingImageProfileSuccess && (nwritten == annotation->annotation().size()) && r;
            } else {
                savingImageProfileSuccess = false;
            }
        }
    }

    if (!savingImageProfileSuccess) {
        m_d->errorMessages.append(i18nc("Saving .kra file error message", "Could not save image profile."));
    }
    success = success && savingImageProfileSuccess;

    //This'll embed the profile used for proofing into the kra file.
    bool savingSoftproofingProfileSuccess = true;
    if (image->proofingConfiguration()) {
        if (image->proofingConfiguration()->storeSoftproofingInsideImage) {
            const KoColorProfile *proofingProfile = KoColorSpaceRegistry::instance()->profileByName(image->proofingConfiguration()->proofingProfile);
            if (proofingProfile && proofingProfile->valid()) {
                QByteArray proofingProfileRaw = proofingProfile->rawData();
                if (!proofingProfileRaw.isEmpty()) {
                    annotation = new KisAnnotation(ICCPROOFINGPROFILE, proofingProfile->name(), proofingProfile->rawData());
                }
            }
            if (annotation) {
                location = external ? QString() : uri;
                location += m_d->imageName + ICC_PROOFING_PATH;
                if (store->open(location)) {
                    nwritten = store->write(annotation->annotation());
                    r = store->close();
                    savingSoftproofingProfileSuccess = savingSoftproofingProfileSuccess && (nwritten == annotation->annotation().size()) && r;
                } else {
                    savingSoftproofingProfileSuccess = false;
                }
            }
        }
    }

    if (!savingSoftproofingProfileSuccess) {
        m_d->errorMessages.append(i18nc("Saving .kra file error message", "Could not save softproofing color profile."));
    }

    success = success && savingSoftproofingProfileSuccess;

    bool savingLayerStylesSuccess = true;
    {
        KisPSDLayerStyleCollectionResource collection("not-nexists.asl");
        KIS_ASSERT_RECOVER_NOOP(!collection.valid());
        collection.collectAllLayerStyles(image->root());
        if (collection.valid()) {
            location = external ? QString() : uri;
            location += m_d->imageName + LAYER_STYLES_PATH;

            if (store->open(location)) {
                QBuffer aslBuffer;
                if (aslBuffer.open(QIODevice::WriteOnly)) {
                    collection.saveToDevice(&aslBuffer);
                    aslBuffer.close();
                    nwritten = store->write(aslBuffer.buffer());
                    savingLayerStylesSuccess = savingLayerStylesSuccess && (nwritten == aslBuffer.buffer().size());
                } else {
                    savingLayerStylesSuccess = false;
                }
                r = store->close();
                savingLayerStylesSuccess = savingLayerStylesSuccess && r;
            } else {
                savingLayerStylesSuccess = false;
            }
        }
    }

    if (!savingLayerStylesSuccess) {
        m_d->errorMessages.append(i18nc("Saving .kra file error message", "Could not save layer styles."));
    }

    success = success && savingLayerStylesSuccess;

    bool savingMergedImageSuccess = true;


    if (!autosave) {
        KisPaintDeviceSP dev = image->projection();
        store->setCompressionEnabled(false);
        r = KisPNGConverter::saveDeviceToStore("mergedimage.png", image->bounds(), image->xRes(), image->yRes(), dev, store);
        savingMergedImageSuccess = savingMergedImageSuccess && r;
        store->setCompressionEnabled(KisConfig(true).compressKra());
    }

    if (!savingMergedImageSuccess) {
        m_d->errorMessages.append(i18nc("Saving .kra file error message", "Could not save merged image."));
    }

    success = success && savingMergedImageSuccess;

    r = saveAssistants(store, uri,external);
    success = success && r;

    return success;
}

QStringList KisKraSaver::errorMessages() const
{
    return m_d->errorMessages;
}

void KisKraSaver::saveBackgroundColor(QDomDocument& doc, QDomElement& element, KisImageSP image)
{
    QDomElement e = doc.createElement(CANVASPROJECTIONCOLOR);
    KoColor color = image->defaultProjectionColor();
    QByteArray colorData = QByteArray::fromRawData((const char*)color.data(), color.colorSpace()->pixelSize());
    e.setAttribute(COLORBYTEDATA, QString(colorData.toBase64()));
    element.appendChild(e);
}

void KisKraSaver::saveAssistantsGlobalColor(QDomDocument& doc, QDomElement& element)
{
    QDomElement e = doc.createElement(GLOBALASSISTANTSCOLOR);
    QString colorString = KisDomUtils::qColorToQString(m_d->doc->assistantsGlobalColor());
    e.setAttribute(SIMPLECOLORDATA, QString(colorString));
    element.appendChild(e);
}

void KisKraSaver::saveWarningColor(QDomDocument& doc, QDomElement& element, KisImageSP image)
{
    if (image->proofingConfiguration()) {
        if (image->proofingConfiguration()->storeSoftproofingInsideImage) {
            QDomElement e = doc.createElement(PROOFINGWARNINGCOLOR);
            KoColor color = image->proofingConfiguration()->warningColor;
            color.toXML(doc, e);
            element.appendChild(e);
        }
    }
}

void KisKraSaver::saveCompositions(QDomDocument& doc, QDomElement& element, KisImageSP image)
{
    if (!image->compositions().isEmpty()) {
        QDomElement e = doc.createElement("compositions");
        Q_FOREACH (KisLayerCompositionSP composition, image->compositions()) {
            composition->save(doc, e);
        }
        element.appendChild(e);
    }
}

bool KisKraSaver::saveAssistants(KoStore* store, QString uri, bool external)
{
    QString location;
    QMap<QString, int> assistantcounters;
    QByteArray data;

    QList<KisPaintingAssistantSP> assistants =  m_d->doc->assistants();
    QMap<KisPaintingAssistantHandleSP, int> handlemap;

    bool success = true;
    if (!assistants.isEmpty()) {

        Q_FOREACH (KisPaintingAssistantSP assist, assistants){
            if (!assistantcounters.contains(assist->id())){
                assistantcounters.insert(assist->id(),0);
            }
            location = external ? QString() : uri;
            location += m_d->imageName + ASSISTANTS_PATH;
            location += QString(assist->id()+"%1.assistant").arg(assistantcounters[assist->id()]);

            data = assist->saveXml(handlemap);
            if (store->open(location)) {
                qint64 nwritten = store->write(data);
                bool r = store->close();
                success = success && r && (nwritten == data.size());
            } else {
                success = false;
            }
            assistantcounters[assist->id()]++;
        }
    }
    if (!success) {
        m_d->errorMessages.append(i18nc("Saving .kra file error message", "Could not save assistants."));
    }
    return true;
}

bool KisKraSaver::saveAssistantsList(QDomDocument& doc, QDomElement& element)
{
    int count_ellipse = 0, count_perspective = 0, count_ruler = 0, count_vanishingpoint = 0,count_infiniteruler = 0, count_parallelruler = 0, count_concentricellipse = 0, count_fisheyepoint = 0, count_spline = 0;
    QList<KisPaintingAssistantSP> assistants =  m_d->doc->assistants();
    if (!assistants.isEmpty()) {
        QDomElement assistantsElement = doc.createElement("assistants");
        Q_FOREACH (KisPaintingAssistantSP assist, assistants){
            if (assist->id() == "ellipse"){
                assist->saveXmlList(doc, assistantsElement, count_ellipse);
                count_ellipse++;
            }
            else if (assist->id() == "spline"){
                assist->saveXmlList(doc, assistantsElement, count_spline);
                count_spline++;
            }
            else if (assist->id() == "perspective"){
                assist->saveXmlList(doc, assistantsElement, count_perspective);
                count_perspective++;
            }
            else if (assist->id() == "vanishing point"){
                assist->saveXmlList(doc, assistantsElement, count_vanishingpoint);
                count_vanishingpoint++;
            }
            else if (assist->id() == "infinite ruler"){
                assist->saveXmlList(doc, assistantsElement, count_infiniteruler);
                count_infiniteruler++;
            }
            else if (assist->id() == "parallel ruler"){
                assist->saveXmlList(doc, assistantsElement, count_parallelruler);
                count_parallelruler++;
            }
            else if (assist->id() == "concentric ellipse"){
                assist->saveXmlList(doc, assistantsElement, count_concentricellipse);
                count_concentricellipse++;
            }
            else if (assist->id() == "fisheye-point"){
                assist->saveXmlList(doc, assistantsElement, count_fisheyepoint);
                count_fisheyepoint++;
            }
            else if (assist->id() == "ruler"){
                assist->saveXmlList(doc, assistantsElement, count_ruler);
                count_ruler++;
            }
        }
        element.appendChild(assistantsElement);
    }
    return true;
}

bool KisKraSaver::saveGrid(QDomDocument& doc, QDomElement& element)
{
    KisGridConfig config = m_d->doc->gridConfig();

    if (!config.isDefault()) {
        QDomElement gridElement = config.saveDynamicDataToXml(doc, "grid");
        element.appendChild(gridElement);
    }

    return true;
}

bool KisKraSaver::saveGuides(QDomDocument& doc, QDomElement& element)
{
    KisGuidesConfig guides = m_d->doc->guidesConfig();

    if (!guides.isDefault()) {
        QDomElement guidesElement = guides.saveToXml(doc, "guides");
        element.appendChild(guidesElement);
    }

    return true;
}

bool KisKraSaver::saveMirrorAxis(QDomDocument &doc, QDomElement &element)
{
    KisMirrorAxisConfig mirrorAxisConfig = m_d->doc->mirrorAxisConfig();

    if (!mirrorAxisConfig.isDefault()) {
        QDomElement mirrorAxisElement = mirrorAxisConfig.saveToXml(doc, MIRROR_AXIS);
        element.appendChild(mirrorAxisElement);
    }

    return true;
}

bool KisKraSaver::saveAudio(QDomDocument& doc, QDomElement& element)
{
    const KisImageAnimationInterface *interface = m_d->doc->image()->animationInterface();
    QString fileName = interface->audioChannelFileName();

    if (fileName.isEmpty()) return true;

    const QDir documentDir = QFileInfo(m_d->filename).absoluteDir();
    KIS_ASSERT_RECOVER_RETURN_VALUE(documentDir.exists(), false);

    fileName = documentDir.relativeFilePath(fileName);
    fileName = QDir::fromNativeSeparators(fileName);

    KIS_ASSERT_RECOVER_RETURN_VALUE(!fileName.isEmpty(), false);

    QDomElement audioElement = doc.createElement("audio");
    KisDomUtils::saveValue(&audioElement, "masterChannelPath", fileName);
    KisDomUtils::saveValue(&audioElement, "audioMuted", interface->isAudioMuted());
    KisDomUtils::saveValue(&audioElement, "audioVolume", interface->audioVolume());
    element.appendChild(audioElement);

    return true;
}

