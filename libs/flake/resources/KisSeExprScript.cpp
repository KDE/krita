/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <FlakeDebug.h>
#include <KoStore.h>
#include <KoStoreDevice.h>
#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QTextDecoder>
#include <kis_assert.h>

#include "KisSeExprScript.h"

struct KisSeExprScript::Private {
    QString script;
    QByteArray data;
    bool dirtyPreset = false;
};

KisSeExprScript::KisSeExprScript(const QString &filename)
    : KoResource(filename)
    , d(new Private)
{
}

KisSeExprScript::KisSeExprScript(const QImage &image, const QString &script, const QString &name, const QString &folderName)
    : KoResource(QString())
    , d(new Private)
{
    setScript(script);
    setImage(image);
    setName(name);

    QFileInfo fileInfo(folderName + QDir::separator() + name + defaultFileExtension());

    int i = 1;
    while (fileInfo.exists()) {
        fileInfo.setFile(folderName + QDir::separator() + name + QString::number(i) + defaultFileExtension());
        i++;
    }

    setFilename(fileInfo.filePath());
}

KisSeExprScript::KisSeExprScript(KisSeExprScript *rhs)
    : KisSeExprScript(*rhs)
{
}

KisSeExprScript::KisSeExprScript(const KisSeExprScript &rhs)
    : KoResource(rhs)
    , d(new Private)
{
    setFilename(rhs.filename());
    setScript(rhs.script());
    setImage(rhs.image());
    setName(rhs.name());
    setValid(rhs.valid());
    setDirty(rhs.isDirty());
}

KisSeExprScript::~KisSeExprScript()
{
    delete d;
}

bool KisSeExprScript::loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(resourcesInterface);

    if (!dev->isOpen())
        dev->open(QIODevice::ReadOnly);

    d->data = dev->readAll();

    // TODO: test
    KIS_ASSERT_RECOVER_RETURN_VALUE(d->data.size() != 0, false);

    if (filename().isNull()) {
        warnFlake << "Cannot load SeExpr script" << name() << ", there is no filename set";
        return false;
    }

    if (d->data.isNull()) {
        QFile file(filename());
        if (file.size() == 0) {
            warnFlake << "Cannot load SeExpr script" << name() << "there is no data available";
            return false;
        }

        file.open(QIODevice::ReadOnly);
        d->data = file.readAll();
        file.close();
    }

    QBuffer buf(&d->data);
    buf.open(QBuffer::ReadOnly);

    QScopedPointer<KoStore> store(KoStore::createStore(&buf, KoStore::Read, "application/x-krita-seexpr-script", KoStore::Zip));
    if (!store || store->bad())
        return false;

    bool storeOpened = store->open("script.se");
    if (!storeOpened) {
        return false;
    }

    d->script = QString(store->read(store->size()));
    store->close();

    if (store->open("preview.png")) {
        KoStoreDevice previewDev(store.data());
        previewDev.open(QIODevice::ReadOnly);

        QImage preview = QImage();
        preview.load(&previewDev, "PNG");
        setImage(preview);

        (void)store->close();
    }

    buf.close();

    QFileInfo fileinfo(filename());
    // The name of a SeExpr script is its basename
    // KoResourceServer uses its filename -- amyspark
    setName(fileinfo.baseName());
    setValid(true);
    setDirty(false);

    return true;
}

bool KisSeExprScript::saveToDevice(QIODevice *dev) const
{
    KoStore *store(KoStore::createStore(dev, KoStore::Write, "application/x-krita-seexpr-script", KoStore::Zip));
    if (!store || store->bad())
        return false;

    if (!store->open("script.se")) {
        return false;
    }

    KoStoreDevice storeDev(store);
    storeDev.open(QIODevice::WriteOnly);

    storeDev.write(d->script.toUtf8());

    if (!store->close()) {
        return false;
    }

    if (!store->open("preview.png")) {
        return false;
    }

    KoStoreDevice previewDev(store);
    previewDev.open(QIODevice::WriteOnly);

    image().save(&previewDev, "PNG");
    if (!store->close()) {
        return false;
    }

    return store->finalize();
}

QPair<QString, QString> KisSeExprScript::resourceType() const
{
    return QPair<QString, QString>(ResourceType::SeExprScripts, "");
}

QString KisSeExprScript::defaultFileExtension() const
{
    return QString(".kse");
}

QString KisSeExprScript::script() const
{
    return d->script;
}

void KisSeExprScript::setScript(const QString &script)
{
    d->script = script;
}

KoResourceSP KisSeExprScript::clone() const
{
    return KoResourceSP(new KisSeExprScript(*this));
}

void KisSeExprScript::setDirty(bool value)
{
    d->dirtyPreset = value;
}

bool KisSeExprScript::isDirty() const
{
    return d->dirtyPreset;
}
