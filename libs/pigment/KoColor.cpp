/*
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "KoColor.h"

#include <QColor>

#include <QDomDocument>
#include <QRegExp>

#include "DebugPigment.h"

#include "KoColorModelStandardIds.h"
#include "KoColorProfile.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"
#include "KoChannelInfo.h"
#include "kis_assert.h"

#include <QGlobalStatic>

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

namespace {

struct DefaultKoColorInitializer
{
    DefaultKoColorInitializer() {
        const KoColorSpace *defaultColorSpace = KoColorSpaceRegistry::instance()->rgb16(0);
        KIS_ASSERT(defaultColorSpace);

        value = new KoColor(Qt::black, defaultColorSpace);
#ifndef NODEBUG
#ifndef QT_NO_DEBUG
        // warn about rather expensive checks in assertPermanentColorspace().
        qWarning() << "KoColor debug runtime checks are active.";
#endif
#endif

        initializeMetatype();
    }

    void initializeMetatype() {
        qRegisterMetaType<KoColor>();

        /**
         * We want KoColor to be comparable inside QVariant,
         * so we should generate comparators.
         */
        QMetaType::registerEqualsComparator<KoColor>();
    }

    ~DefaultKoColorInitializer() {
        delete value;
    }

    KoColor *value = 0;
};

Q_GLOBAL_STATIC(DefaultKoColorInitializer, s_defaultKoColor)

}


KoColor::KoColor() {
    *this = *s_defaultKoColor->value;
}

KoColor::KoColor(const KoColorSpace * colorSpace)
{
    Q_ASSERT(colorSpace);
    m_colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(colorSpace);
    m_size = m_colorSpace->pixelSize();
    Q_ASSERT(m_size <= MAX_PIXEL_SIZE);
    memset(m_data, 0, m_size);
}

KoColor::KoColor(const QColor & color, const KoColorSpace * colorSpace)
{
    Q_ASSERT(color.isValid());
    Q_ASSERT(colorSpace);
    m_colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(colorSpace);

    m_size = m_colorSpace->pixelSize();
    Q_ASSERT(m_size <= MAX_PIXEL_SIZE);
    memset(m_data, 0, m_size);

    m_colorSpace->fromQColor(color, m_data);
}

KoColor::KoColor(const quint8 * data, const KoColorSpace * colorSpace)
{
    Q_ASSERT(colorSpace);
    Q_ASSERT(data);
    m_colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(colorSpace);
    m_size = m_colorSpace->pixelSize();
    Q_ASSERT(m_size <= MAX_PIXEL_SIZE);
    memmove(m_data, data, m_size);
}


KoColor::KoColor(const KoColor &src, const KoColorSpace * colorSpace)
{
    Q_ASSERT(colorSpace);
    m_colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(colorSpace);
    m_size = m_colorSpace->pixelSize();
    Q_ASSERT(m_size <= MAX_PIXEL_SIZE);
    memset(m_data, 0, m_size);

    src.colorSpace()->convertPixelsTo(src.m_data, m_data, colorSpace, 1, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
}

bool KoColor::operator==(const KoColor &other) const {
    if (*colorSpace() != *other.colorSpace()) {
        return false;
    }
    if (m_size != other.m_size) {
        return false;
    }
    return memcmp(m_data, other.m_data, m_size) == 0;
}

void KoColor::convertTo(const KoColorSpace * cs, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    //dbgPigment <<"Our colormodel:" << d->colorSpace->id().name()
    //      << ", new colormodel: " << cs->id().name() << "\n";

    if (*m_colorSpace == *cs)
        return;

    quint8 data[MAX_PIXEL_SIZE];
    const size_t size = cs->pixelSize();
    Q_ASSERT(size <= MAX_PIXEL_SIZE);
    memset(data, 0, size);

    m_colorSpace->convertPixelsTo(m_data, data, cs, 1, renderingIntent, conversionFlags);

    memcpy(m_data, data, size);
    m_size = size;
    m_colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(cs);
}

void KoColor::convertTo(const KoColorSpace * cs)
{
    convertTo(cs,
              KoColorConversionTransformation::internalRenderingIntent(),
              KoColorConversionTransformation::internalConversionFlags());
}

KoColor KoColor::convertedTo(const KoColorSpace *cs, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags) const
{
    KoColor result(*this);
    result.convertTo(cs, renderingIntent, conversionFlags);
    return result;
}

KoColor KoColor::convertedTo(const KoColorSpace *cs) const
{
    return convertedTo(cs,
                       KoColorConversionTransformation::internalRenderingIntent(),
                       KoColorConversionTransformation::internalConversionFlags());
}

void KoColor::setProfile(const KoColorProfile *profile)
{
    const KoColorSpace *dstColorSpace =
            KoColorSpaceRegistry::instance()->colorSpace(colorSpace()->colorModelId().id(), colorSpace()->colorDepthId().id(), profile);
    if (!dstColorSpace) return;

    m_colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(dstColorSpace);
}

void KoColor::setColor(const quint8 * data, const KoColorSpace * colorSpace)
{
    Q_ASSERT(colorSpace);

    m_size = colorSpace->pixelSize();
    Q_ASSERT(m_size <= MAX_PIXEL_SIZE);

    memcpy(m_data, data, m_size);
    m_colorSpace = KoColorSpaceRegistry::instance()->permanentColorspace(colorSpace);
}

// To save the user the trouble of doing color->colorSpace()->toQColor(color->data(), &c, &a, profile
void KoColor::toQColor(QColor *c) const
{
    Q_ASSERT(c);
    if (m_colorSpace) {
        m_colorSpace->toQColor(m_data, c);
    }
}

QColor KoColor::toQColor() const
{
    QColor c;
    toQColor(&c);
    return c;
}

void KoColor::fromQColor(const QColor& c)
{
    if (m_colorSpace) {
        m_colorSpace->fromQColor(c, m_data);
    }
}

void KoColor::subtract(const KoColor &value)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(*m_colorSpace == *value.colorSpace());

    QVector<float> channels1(m_colorSpace->channelCount());
    QVector<float> channels2(m_colorSpace->channelCount());

    m_colorSpace->normalisedChannelsValue(m_data, channels1);
    m_colorSpace->normalisedChannelsValue(value.data(), channels2);

    for (int i = 0; i < channels1.size(); i++) {
        channels1[i] -= channels2[i];
    }

    m_colorSpace->fromNormalisedChannelsValue(m_data, channels1);
}

KoColor KoColor::subtracted(const KoColor &value) const
{
    KoColor result(*this);
    result.subtract(value);
    return result;
}

void KoColor::add(const KoColor &value)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(*m_colorSpace == *value.colorSpace());

    QVector<float> channels1(m_colorSpace->channelCount());
    QVector<float> channels2(m_colorSpace->channelCount());

    m_colorSpace->normalisedChannelsValue(m_data, channels1);
    m_colorSpace->normalisedChannelsValue(value.data(), channels2);

    for (int i = 0; i < channels1.size(); i++) {
        channels1[i] += channels2[i];
    }

    m_colorSpace->fromNormalisedChannelsValue(m_data, channels1);
}

KoColor KoColor::added(const KoColor &value) const
{
    KoColor result(*this);
    result.add(value);
    return result;
}

#ifndef NDEBUG
void KoColor::dump() const
{
    dbgPigment <<"KoColor (" << this <<")," << m_colorSpace->id() <<"";
    QList<KoChannelInfo *> channels = m_colorSpace->channels();

    QList<KoChannelInfo *>::const_iterator begin = channels.constBegin();
    QList<KoChannelInfo *>::const_iterator end = channels.constEnd();

    for (QList<KoChannelInfo *>::const_iterator it = begin; it != end; ++it) {
        KoChannelInfo * ch = (*it);
        // XXX: setNum always takes a byte.
        if (ch->size() == sizeof(quint8)) {
            // Byte
            dbgPigment <<"Channel (byte):" << ch->name() <<":" << QString().setNum(m_data[ch->pos()]) <<"";
        } else if (ch->size() == sizeof(quint16)) {
            // Short (may also by an nvidia half)
            dbgPigment <<"Channel (short):" << ch->name() <<":" << QString().setNum(*((const quint16 *)(m_data+ch->pos())))  <<"";
        } else if (ch->size() == sizeof(quint32)) {
            // Integer (may also be float... Find out how to distinguish these!)
            dbgPigment <<"Channel (int):" << ch->name() <<":" << QString().setNum(*((const quint32 *)(m_data+ch->pos())))  <<"";
        }
    }
}
#endif

void KoColor::fromKoColor(const KoColor& src)
{
    src.colorSpace()->convertPixelsTo(src.m_data, m_data, colorSpace(), 1, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
}

const KoColorProfile *KoColor::profile() const
{
    return m_colorSpace->profile();
}

void KoColor::toXML(QDomDocument& doc, QDomElement& colorElt) const
{
    m_colorSpace->colorToXML(m_data, doc, colorElt);
}

void KoColor::setOpacity(quint8 alpha)
{
    m_colorSpace->setOpacity(m_data, alpha, 1);
}
void KoColor::setOpacity(qreal alpha)
{
    m_colorSpace->setOpacity(m_data, alpha, 1);
}
quint8 KoColor::opacityU8() const
{
    return m_colorSpace->opacityU8(m_data);
}
qreal KoColor::opacityF() const
{
    return m_colorSpace->opacityF(m_data);
}

KoColor KoColor::fromXML(const QDomElement& elt, const QString& channelDepthId)
{
    bool ok;
    return fromXML(elt, channelDepthId, &ok);
}

KoColor KoColor::fromXML(const QDomElement& elt, const QString& channelDepthId, bool* ok)
{
    *ok = true;
    QString modelId;
    if (elt.tagName() == "CMYK") {
        modelId = CMYKAColorModelID.id();
    } else if (elt.tagName() == "RGB") {
        modelId = RGBAColorModelID.id();
    } else if (elt.tagName() == "sRGB") {
        modelId = RGBAColorModelID.id();
    } else if (elt.tagName() == "Lab") {
        modelId = LABAColorModelID.id();
    } else if (elt.tagName() == "XYZ") {
        modelId = XYZAColorModelID.id();
    } else if (elt.tagName() == "Gray") {
        modelId = GrayAColorModelID.id();
    } else if (elt.tagName() == "YCbCr") {
        modelId = YCbCrAColorModelID.id();
    }
    QString profileName;
    if (elt.tagName() != "sRGB") {
        profileName = elt.attribute("space", "");
        if (!KoColorSpaceRegistry::instance()->profileByName(profileName)) {
            profileName.clear();
        }
    } else {
        profileName = KoColorSpaceRegistry::instance()->p709SRGBProfile()->name();
    }
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->colorSpace(modelId, channelDepthId, profileName);
    if (cs == 0) {
        QList<KoID> list =  KoColorSpaceRegistry::instance()->colorDepthList(modelId, KoColorSpaceRegistry::AllColorSpaces);
        if (!list.empty()) {
            cs = KoColorSpaceRegistry::instance()->colorSpace(modelId, list[0].id(), profileName);
        }
    }
    if (cs) {
        KoColor c(cs);
        // TODO: Provide a way for colorFromXML() to notify the caller if parsing failed. Currently it returns default values on failure.
        cs->colorFromXML(c.data(), elt);
        return c;
    } else {
        *ok = false;
        return KoColor();
    }
}

QString KoColor::toXML() const
{
    QDomDocument cdataDoc = QDomDocument("color");
    QDomElement cdataRoot = cdataDoc.createElement("color");
    cdataDoc.appendChild(cdataRoot);
    cdataRoot.setAttribute("channeldepth", colorSpace()->colorDepthId().id());
    toXML(cdataDoc, cdataRoot);
    return cdataDoc.toString();
}

KoColor KoColor::fromXML(const QString &xml)
{
    KoColor c;
    QDomDocument doc;
    if (doc.setContent(xml)) {
        QDomElement e = doc.documentElement().firstChild().toElement();
        QString channelDepthID = doc.documentElement().attribute("channeldepth", Integer16BitsColorDepthID.id());
        bool ok;
        if (e.hasAttribute("space") || e.tagName().toLower() == "srgb") {
            c = KoColor::fromXML(e, channelDepthID, &ok);
        } else if (doc.documentElement().hasAttribute("space") || doc.documentElement().tagName().toLower() == "srgb"){
            c = KoColor::fromXML(doc.documentElement(), channelDepthID, &ok);
        } else {
            qWarning() << "Cannot parse color from xml" << xml;
        }
    }
    return c;
}

QString KoColor::toSVG11(QHash<QString, const KoColorProfile *> *profileList) const
{
    QStringList colorDefinitions;
    colorDefinitions.append(toQColor().name());

    QVector<float> channelValues(colorSpace()->channelCount());
    channelValues.fill(0.0);
    colorSpace()->normalisedChannelsValue(data(), channelValues);

    bool sRGB = colorSpace()->profile()->uniqueId() == KoColorSpaceRegistry::instance()->p709SRGBProfile()->uniqueId();

    // We don't write a icc-color definition for XYZ and 8bit sRGB.
    if (!(sRGB && colorSpace()->colorDepthId() == Integer8BitsColorDepthID) &&
            colorSpace()->colorModelId() != XYZAColorModelID) {
        QStringList iccColor;
        QString csName = colorSpace()->profile()->name();
        // remove forbidden characters
        // https://www.w3.org/TR/SVG11/types.html#DataTypeName
        csName.remove(QRegExp("[\\(\\),\\s]"));

        //reuse existing name if possible. We're looking for the color profile, because svg doesn't care about depth.
        csName = profileList->key(colorSpace()->profile(), csName);

        if (sRGB) {
            csName = "sRGB";
        }

        iccColor.append(csName);

        if (colorSpace()->colorModelId() == LABAColorModelID) {
            QDomDocument doc;
            QDomElement el = doc.createElement("color");
            toXML(doc, el);
            QDomElement lab = el.firstChildElement();
            iccColor.append(lab.attribute("L", "0.0"));
            iccColor.append(lab.attribute("a", "0.0"));
            iccColor.append(lab.attribute("b", "0.0"));
        } else {
            for (int i = 0; i < channelValues.size(); i++) {
                int location = KoChannelInfo::displayPositionToChannelIndex(i, colorSpace()->channels());
                if (i != int(colorSpace()->alphaPos())) {
                    iccColor.append(QString::number(channelValues.at(location), 'g', 10));
                }
            }
        }
        colorDefinitions.append(QString("icc-color(%1)").arg(iccColor.join(", ")));
        if (!profileList->contains(csName) && !sRGB) {
            profileList->insert(csName, colorSpace()->profile());
        }
    }

    return colorDefinitions.join(" ");
}

KoColor KoColor::fromSVG11(const QString value, QHash<QString, const KoColorProfile *> profileList, KoColor current)
{
    KoColor parsed(KoColorSpaceRegistry::instance()->rgb16(KoColorSpaceRegistry::instance()->p709SRGBProfile()));

    if (value.toLower() == "none") {
        return parsed;
    }

    // add the sRGB default name.
    profileList.insert("sRGB", KoColorSpaceRegistry::instance()->p709SRGBProfile());
    // first, try to split at \w\d\) space.
    // we want to split up a string like... colorcolor none rgb(0.8, 0.1, 200%) #ff0000 icc-color(blah, 0.0, 1.0, 1.0, 0.0);
    QRegExp splitDefinitions("(#?\\w+|[\\w\\-]*\\(.+\\))\\s");
    int pos = 0;
    int pos2 = 0;
    QStringList colorDefinitions;
    while ((pos2 = splitDefinitions.indexIn(value, pos)) != -1) {
        colorDefinitions.append(splitDefinitions.cap(1).trimmed());
        pos = pos2 + splitDefinitions.matchedLength();
    }
    if (pos < value.length()) {
        QString remainder = value.right(value.length()-pos);
        remainder.remove(";");
        colorDefinitions.append(remainder);
    }
    dbgPigment << "Color definitions found during svg11parsing" << colorDefinitions;

    for (QString def : colorDefinitions) {
        if (def.toLower() == "currentcolor") {
            parsed = current;
        } else if (QColor::isValidColor(def)) {
            parsed.fromQColor(QColor(def));
        } else if (def.toLower().startsWith("rgb")) {
            QString parse = def.trimmed();
            QStringList colors = parse.split(',');
            QString r = colors[0].right((colors[0].length() - 4));
            QString g = colors[1];
            QString b = colors[2].left((colors[2].length() - 1));

            if (r.contains('%')) {
                r = r.left(r.length() - 1);
                r = QString::number(int((double(255 * r.toDouble()) / 100.0)));
            }

            if (g.contains('%')) {
                g = g.left(g.length() - 1);
                g = QString::number(int((double(255 * g.toDouble()) / 100.0)));
            }

            if (b.contains('%')) {
                b = b.left(b.length() - 1);
                b = QString::number(int((double(255 * b.toDouble()) / 100.0)));
            }
            parsed.fromQColor(QColor(r.toInt(), g.toInt(), b.toInt()));

        } else if (def.toLower().startsWith("icc-color")) {
            QStringList values = def.split(",");
            QString iccprofilename = values.first().split("(").last();
            values.removeFirst();

            // svg11 docs say that searching the name should be caseinsentive.
            QStringList entry = QStringList(profileList.keys()).filter(iccprofilename, Qt::CaseInsensitive);
            if (entry.empty()) {
                continue;
            }
            const KoColorProfile *profile = profileList.value(entry.first());
            if (!profile) {
                continue;
            }
            QString colormodel = profile->colorModelID();
            QString depth = "F32";
            if (colormodel == LABAColorModelID.id()) {
                // let our xml handling deal with lab
                QVector<float> labV(3);
                for (int i = 0; i < values.size(); i++) {
                    if (i<labV.size()) {
                        QString entry = values.at(i);
                        entry = entry.split(")").first();
                        labV[i] = entry.toDouble();
                    }
                }
                QString lab = QString("<Lab space='%1' L='%2' a='%3' b='%4' />")
                        .arg(profile->name())
                        .arg(labV[0])
                        .arg(labV[1])
                        .arg(labV[2]);
                QDomDocument doc;
                doc.setContent(lab);
                parsed = KoColor::fromXML(doc.documentElement(), "U16");
                continue;
            } else if (colormodel == CMYKAColorModelID.id()) {
                depth = "U16";
            } else if (colormodel == XYZAColorModelID.id()) {
                // Inkscape decided to have X and Z go from 0 to 2, and I can't for the live of me figure out why.
                // So we're just not parsing XYZ.
                continue;
            }
            const KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace(colormodel, depth, profile);
            if (!cs) {
                continue;
            }
            parsed = KoColor(cs);
            QVector<float> channelValues(parsed.colorSpace()->channelCount());
            channelValues.fill(0.0);
            channelValues[parsed.colorSpace()->alphaPos()] = 1.0;
            for (int channel = 0; channel < values.size(); channel++) {
                int location = KoChannelInfo::displayPositionToChannelIndex(channel, parsed.colorSpace()->channels());
                QString entry = values.at(channel);
                entry = entry.split(")").first();
                channelValues[location] = entry.toFloat();
            }
            parsed.colorSpace()->fromNormalisedChannelsValue(parsed.data(), channelValues);
        }
    }

    return parsed;
}

QString KoColor::toQString(const KoColor &color)
{
    QStringList ls;
    Q_FOREACH (KoChannelInfo *channel, KoChannelInfo::displayOrderSorted(color.colorSpace()->channels())) {
        int realIndex = KoChannelInfo::displayPositionToChannelIndex(channel->displayPosition(), color.colorSpace()->channels());
        ls << channel->name();
        ls << color.colorSpace()->channelValueText(color.data(), realIndex);
    }
    return ls.join(" ");
}

QDebug operator<<(QDebug dbg, const KoColor &color)
{
    dbg.nospace() << "KoColor (" << color.colorSpace()->id();

    QList<KoChannelInfo*> channels = color.colorSpace()->channels();
    for (auto it = channels.constBegin(); it != channels.constEnd(); ++it) {

        KoChannelInfo *ch = (*it);

        dbg.nospace() << ", " << ch->name() << ":";

        switch (ch->channelValueType()) {
        case KoChannelInfo::UINT8: {
            const quint8 *ptr = reinterpret_cast<const quint8*>(color.data() + ch->pos());
            dbg.nospace() << *ptr;
            break;
        } case KoChannelInfo::UINT16: {
            const quint16 *ptr = reinterpret_cast<const quint16*>(color.data() + ch->pos());
            dbg.nospace() << *ptr;
            break;
        } case KoChannelInfo::UINT32: {
            const quint32 *ptr = reinterpret_cast<const quint32*>(color.data() + ch->pos());
            dbg.nospace() << *ptr;
            break;
        } case KoChannelInfo::FLOAT16: {

#ifdef HAVE_OPENEXR
            const half *ptr = reinterpret_cast<const half*>(color.data() + ch->pos());
            dbg.nospace() << *ptr;
#else
            const quint16 *ptr = reinterpret_cast<const quint16*>(color.data() + ch->pos());
            dbg.nospace() << "UNSUPPORTED_F16(" << *ptr << ")";
#endif
            break;
        } case KoChannelInfo::FLOAT32: {
            const float *ptr = reinterpret_cast<const float*>(color.data() + ch->pos());
            dbg.nospace() << *ptr;
            break;
        } case KoChannelInfo::FLOAT64: {
            const double *ptr = reinterpret_cast<const double*>(color.data() + ch->pos());
            dbg.nospace() << *ptr;
            break;
        } case KoChannelInfo::INT8: {
            const qint8 *ptr = reinterpret_cast<const qint8*>(color.data() + ch->pos());
            dbg.nospace() << *ptr;
            break;
        } case KoChannelInfo::INT16: {
            const qint16 *ptr = reinterpret_cast<const qint16*>(color.data() + ch->pos());
            dbg.nospace() << *ptr;
            break;
        } case KoChannelInfo::OTHER: {
            const quint8 *ptr = reinterpret_cast<const quint8*>(color.data() + ch->pos());
            dbg.nospace() << "undef(" << *ptr << ")";
            break;
        }
        }
    }
    dbg.nospace() << ")";
    return dbg.space();
}
