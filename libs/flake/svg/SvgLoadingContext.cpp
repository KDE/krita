/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "SvgLoadingContext.h"

#include <QStack>
#include <QFileInfo>
#include <QDir>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceEngine.h>
#include <KoColorProfile.h>
#include <KoDocumentResourceManager.h>

#include <FlakeDebug.h>

#include "SvgGraphicContext.h"
#include "SvgUtil.h"
#include "SvgCssHelper.h"
#include "SvgStyleParser.h"
#include "kis_debug.h"


class Q_DECL_HIDDEN SvgLoadingContext::Private
{
public:
    Private()
        : zIndex(0), styleParser(0)
    {

    }

    ~Private()
    {
        if (! gcStack.isEmpty() && !gcStack.top()->isResolutionFrame) {
            // Resolution frame is usually the first and is not removed.
            warnFlake << "the context stack is not empty (current count" << gcStack.size() << ", expected 0)";
        }
        qDeleteAll(gcStack);
        gcStack.clear();
        delete styleParser;
    }
    QStack<SvgGraphicsContext*> gcStack;
    QString initialXmlBaseDir;
    int zIndex;
    KoDocumentResourceManager *documentResourceManager;
    QHash<QString, KoShape*> loadedShapes;
    QHash<QString, KoXmlElement> definitions;
    QHash<QString, const KoColorProfile*> profiles;
    SvgCssHelper cssStyles;
    SvgStyleParser *styleParser;
    FileFetcherFunc fileFetcher;
};

SvgLoadingContext::SvgLoadingContext(KoDocumentResourceManager *documentResourceManager)
    : d(new Private())
{
    d->documentResourceManager = documentResourceManager;
    d->styleParser = new SvgStyleParser(*this);
    Q_ASSERT(d->documentResourceManager);
}

SvgLoadingContext::~SvgLoadingContext()
{
    delete d;
}

SvgGraphicsContext *SvgLoadingContext::currentGC() const
{
    if (d->gcStack.isEmpty())
        return 0;

    return d->gcStack.top();
}

#include "parsers/SvgTransformParser.h"

SvgGraphicsContext *SvgLoadingContext::pushGraphicsContext(const KoXmlElement &element, bool inherit)
{
    SvgGraphicsContext *gc;
    // copy data from current context
    if (! d->gcStack.isEmpty() && inherit) {
        gc = new SvgGraphicsContext(*d->gcStack.top());
    } else {
        gc = new SvgGraphicsContext();
    }

    gc->textProperties.resetNonInheritableToDefault(); // some of the text properties are not inherited
    gc->filterId.clear(); // filters are not inherited
    gc->clipPathId.clear(); // clip paths are not inherited
    gc->clipMaskId.clear(); // clip masks are not inherited
    gc->display = true; // display is not inherited
    gc->opacity = 1.0; // opacity is not inherited

    if (!element.isNull()) {
        if (element.hasAttribute("transform")) {
            SvgTransformParser p(element.attribute("transform"));
            if (p.isValid()) {
                QTransform mat = p.transform();
                gc->matrix = mat * gc->matrix;
            }
        }
        if (element.hasAttribute("xml:base"))
            gc->xmlBaseDir = element.attribute("xml:base");
        if (element.hasAttribute("xml:space"))
            gc->preserveWhitespace = element.attribute("xml:space") == "preserve";
    }

    d->gcStack.push(gc);

    return gc;
}

void SvgLoadingContext::popGraphicsContext()
{
    delete(d->gcStack.pop());
}

void SvgLoadingContext::setInitialXmlBaseDir(const QString &baseDir)
{
    d->initialXmlBaseDir = baseDir;
}

QString SvgLoadingContext::xmlBaseDir() const
{
    SvgGraphicsContext *gc = currentGC();
    return (gc && !gc->xmlBaseDir.isEmpty()) ? gc->xmlBaseDir : d->initialXmlBaseDir;
}

QString SvgLoadingContext::absoluteFilePath(const QString &href)
{
    QFileInfo info(href);
    if (! info.isRelative())
        return href;

    SvgGraphicsContext *gc = currentGC();
    if (!gc)
        return d->initialXmlBaseDir;

    QString baseDir = d->initialXmlBaseDir;
    if (! gc->xmlBaseDir.isEmpty())
        baseDir = absoluteFilePath(gc->xmlBaseDir);

    QFileInfo pathInfo(QFileInfo(baseDir).filePath());

    QString relFile = href;
    while (relFile.startsWith(QLatin1String("../"))) {
        relFile.remove(0, 3);
        pathInfo.setFile(pathInfo.dir(), QString());
    }

    QString absFile = pathInfo.absolutePath() + '/' + relFile;

    return absFile;
}

QString SvgLoadingContext::relativeFilePath(const QString &href)
{
    const SvgGraphicsContext *gc = currentGC();
    if (!gc) return href;

    QString result = href;

    QFileInfo info(href);
    if (info.isRelative())
        return href;


    if (!gc->xmlBaseDir.isEmpty()) {
        result = QDir(gc->xmlBaseDir).relativeFilePath(href);
    } else if (!d->initialXmlBaseDir.isEmpty()) {
        result = QDir(d->initialXmlBaseDir).relativeFilePath(href);
    }

    return QDir::cleanPath(result);
}

int SvgLoadingContext::nextZIndex()
{
    return d->zIndex++;
}

KoImageCollection* SvgLoadingContext::imageCollection()
{
    return d->documentResourceManager->imageCollection();
}

void SvgLoadingContext::registerShape(const QString &id, KoShape *shape)
{
    if (!id.isEmpty())
        d->loadedShapes.insert(id, shape);
}

KoShape* SvgLoadingContext::shapeById(const QString &id)
{
    return d->loadedShapes.value(id);
}

void SvgLoadingContext::addDefinition(const KoXmlElement &element)
{
    const QString id = element.attribute("id");
    if (id.isEmpty() || d->definitions.contains(id))
        return;
    d->definitions.insert(id, element);
}

KoXmlElement SvgLoadingContext::definition(const QString &id) const
{
    return d->definitions.value(id);
}

bool SvgLoadingContext::hasDefinition(const QString &id) const
{
    return d->definitions.contains(id);
}

void SvgLoadingContext::addStyleSheet(const KoXmlElement &styleSheet)
{
    d->cssStyles.parseStylesheet(styleSheet);
}

QStringList SvgLoadingContext::matchingCssStyles(const KoXmlElement &element) const
{
    return d->cssStyles.matchStyles(element);
}

SvgStyleParser &SvgLoadingContext::styleParser()
{
    return *d->styleParser;
}

void SvgLoadingContext::parseProfile(const KoXmlElement &element)
{
    const QString href = element.attribute("xlink:href");
    const QByteArray uniqueId = QByteArray::fromHex(element.attribute("local").toLatin1());
    const QString name = element.attribute("name");

    if (element.attribute("rendering-intent", "auto") != "auto") {
        // WARNING: Krita does *not* treat rendering intents attributes of the profile!
        debugFlake << "WARNING: we do *not* treat rendering intents attributes of the profile!";
    }

    if (d->profiles.contains(name)) {
        debugFlake << "Profile already in the map!" << ppVar(name);
        return;
    }

    const KoColorProfile *profile =
            KoColorSpaceRegistry::instance()->profileByUniqueId(uniqueId);

    if (!profile && d->fileFetcher) {
        KoColorSpaceEngine *engine = KoColorSpaceEngineRegistry::instance()->get("icc");
        KIS_ASSERT(engine);
        if (engine) {
            const QString fileName = relativeFilePath(href);
            const QByteArray profileData = d->fileFetcher(fileName);
            if (!profileData.isEmpty()) {
                profile = engine->addProfile(profileData);

                if (profile->uniqueId() != uniqueId) {
                    debugFlake << "WARNING: ProfileID of the attached profile doesn't match the one mentioned in SVG element";
                    debugFlake << "       " << ppVar(profile->uniqueId().toHex());
                    debugFlake << "       " << ppVar(uniqueId.toHex());
                }
            } else {
                debugFlake << "WARNING: couldn't fetch the ICCprofile file!" << fileName;
            }
        }
    }

    if (profile) {
        d->profiles.insert(name, profile);
    } else {
        debugFlake << "WARNING: couldn't load SVG profile" << ppVar(name) << ppVar(href) << ppVar(uniqueId);
    }
}

QHash<QString, const KoColorProfile *> SvgLoadingContext::profiles()
{
    return d->profiles;
}

bool SvgLoadingContext::isRootContext() const
{
    KIS_ASSERT(!d->gcStack.isEmpty());
    return d->gcStack.size() == 1;
}

void SvgLoadingContext::setFileFetcher(SvgLoadingContext::FileFetcherFunc func)
{
    d->fileFetcher = func;
}

QByteArray SvgLoadingContext::fetchExternalFile(const QString &url)
{
    return d->fileFetcher ? d->fileFetcher(url) : QByteArray();
}
