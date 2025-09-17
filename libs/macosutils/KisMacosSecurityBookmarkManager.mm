/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2023 Ivan Santa María <ghevan@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#import "KisMacosSecurityBookmarkManager.h"

#import <Foundation/Foundation.h>
#import <Security/SecCode.h>

#ifdef KIS_STANDALONE
#import <AppKit/AppKit.h>
#endif

#include <QStandardPaths>
#include <QSettings>
#include <QHash>
#include <QVariant>
#include <QString>
#include <QUrl>
#include <QMetaEnum>

#include <QMessageBox>
#include <QFileDialog>

#include <QDebug>

#include <klocalizedstring.h>

#include "KisMacosEntitlements.h"


#if ! __has_feature(objc_arc)
#error "Enable ARC to compile this file"
#endif

Q_GLOBAL_STATIC(KisMacosSecurityBookmarkManager, s_instance)

class KisMacosSecurityBookmarkManager::Private
{
public:
    Private()
        : securedFiles(QHash<QString,QString>())
        , entitlements(KisMacosEntitlements())
    {
    }

    ~Private()
    {
    }

    KisMacosEntitlements entitlements;
    QHash<QString, QString> securedFiles;
};

KisMacosSecurityBookmarkManager* KisMacosSecurityBookmarkManager::instance()
{
    return s_instance;
}

KisMacosSecurityBookmarkManager::KisMacosSecurityBookmarkManager()
    : m_d(new Private())
{
    loadSecurityScopedResources();
    startAccessingSecurityScopedResources();
}

KisMacosSecurityBookmarkManager::~KisMacosSecurityBookmarkManager()
{
    stopAccessingSecurityScopedResources();
}

bool KisMacosSecurityBookmarkManager::parentDirHasPermissions(const QString &path)
{
    bool contained = false;
    Q_FOREACH(QString key, m_d->securedFiles.keys()) {
        if(path.contains(key)) {
            contained = true;
            break;
        }
    }
    return contained;
}

void KisMacosSecurityBookmarkManager::createBookmarkFromPath(const QString &path, const QString &refpath, SecurityBookmarkType type)
{
    if(path.isEmpty()) {
        return;
    }
    NSURLBookmarkCreationOptions
        options = m_d->entitlements.hasEntitlement(KisMacosEntitlements::Entitlements::BookmarkScopeApp) ? NSURLBookmarkCreationWithSecurityScope : 0;
    NSError *err = nil;

    NSString *pathEscaped = [path.toNSString() stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet URLFragmentAllowedCharacterSet]];
    NSURL *url = [NSURL URLWithString:pathEscaped];

    NSURL *refurl = nil;
    if (!refpath.isEmpty()) {
        NSString *refpathEscaped = [refpath.toNSString() stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet URLFragmentAllowedCharacterSet]];
        refurl = [NSURL URLWithString:refpathEscaped];
    }

    // working with paths as strings
    // this assumes user used NSOpenPanel on the path itself

    // Create bookmark
    NSData *bookmark = [url bookmarkDataWithOptions: options includingResourceValuesForKeys: nil relativeToURL: refurl error:&err];
    NSString *bookmarkString = [bookmark base64EncodedStringWithOptions: NSDataBase64Encoding64CharacterLineLength];
    NSLog(@" path: %@\n err: %@", pathEscaped, err);

    if (!err) {
        [url startAccessingSecurityScopedResource];

        QString base64Data = QString::fromNSString(bookmarkString);
        // write to file
        const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
        QSettings kritarc(configPath + QStringLiteral("/securitybookmarkrc"), QSettings::NativeFormat);

        // get array current size
        int size = kritarc.beginReadArray(securityBookmarkTypeToString(type));
        kritarc.endArray();

        kritarc.beginWriteArray(securityBookmarkTypeToString(type));
        kritarc.setArrayIndex(size);
        kritarc.setValue("path",path);
        kritarc.setValue("base64", base64Data);
        kritarc.endArray();

        // Finally add to hashmap
        m_d->securedFiles[path] = base64Data;
    }

    return;
}

void KisMacosSecurityBookmarkManager::loadKeysFromArray(SecurityBookmarkType arrayKey)
{
    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QSettings kritarc(configPath + QStringLiteral("/securitybookmarkrc"), QSettings::NativeFormat);

    int size = kritarc.beginReadArray(securityBookmarkTypeToString(arrayKey));
    for (int i = 0; i < size; i++) {
        kritarc.setArrayIndex(i);
        QString key = kritarc.value("path").toString();
        m_d->securedFiles[key] = kritarc.value("base64").toString();
    }
    kritarc.endArray();
}

void KisMacosSecurityBookmarkManager::loadSecurityScopedResources()
{
    loadKeysFromArray(SecurityBookmarkType::File);
    loadKeysFromArray(SecurityBookmarkType::Directory);
}

QUrl KisMacosSecurityBookmarkManager::decodeBookmarkToURL(QString encodedPath) {
    NSString *bookmarkString = encodedPath.toNSString();
    NSData *decodedBookmark = [[NSData alloc] initWithBase64EncodedString: bookmarkString options:NSDataBase64DecodingIgnoreUnknownCharacters];

    NSError *err = nil;

    // Resolve the decoded bookmark data into a security-scoped URL.
    NSURL *url =[NSURL URLByResolvingBookmarkData: decodedBookmark options: NSURLBookmarkResolutionWithSecurityScope relativeToURL: nil bookmarkDataIsStale: nil error:&err];
    return QUrl::fromNSURL(url);
}

void KisMacosSecurityBookmarkManager::startAccessingSecurityScopedResources()
{
    Q_FOREACH(QString encodedPath, m_d->securedFiles.values()) {
        NSURL *url = decodeBookmarkToURL(encodedPath).toNSURL();
        [url startAccessingSecurityScopedResource];
    }
}

void KisMacosSecurityBookmarkManager::stopAccessingSecurityScopedResources()
{
    Q_FOREACH(QString encodedPath, m_d->securedFiles.values()) {
        NSURL *url = decodeBookmarkToURL(encodedPath).toNSURL();
        [url stopAccessingSecurityScopedResource];
    }
}

bool KisMacosSecurityBookmarkManager::requestAccessToDir(const QString &path)
{
    bool fileSelected = false;
    QUrl fileURL = QUrl(path);

    NSString *nsStdPath = path.toNSString();
#ifdef KIS_STANDALONE
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setAlertStyle:NSAlertStyleInformational];
    [alert setMessageText:[NSString stringWithFormat:@"The file %@ is located in a directory where the application has no permission, please give the permission to the container folder or a higher one to allow krita to save temporary backups next to your file", [nsStdPath lastPathComponent]]];
    [alert runModal];


    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.canChooseFiles = false;
    panel.canChooseDirectories = true;
    NSURL *startLoc = [[NSURL alloc] initFileURLWithPath:nsStdPath ];
    panel.directoryURL = startLoc;

    if ([panel runModal] == NSModalResponseOK) {
        static NSURL *fileUrl = [panel URL];
        NSString *pathString = [fileUrl absoluteString];

        QString filepath = QString::fromNSString(pathString);
        createBookmarkFromPath(filepath, QString(), SecurityBookmarkType::Directory);
        fileSelected = true;
    }

#else
    QMessageBox msgBox;
    msgBox.setText(i18n("The file %1 is located in a directory where the application has no permissions, please give krita permission to this directory or a higher one to allow krita to save temporary backups next to your file", fileURL.fileName()));
    msgBox.setInformativeText(i18n("The directory you select will grant krita permissions to all files and directories contained in it"));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    int ret = msgBox.exec();

    QUrl dirUrl = QFileDialog::getExistingDirectoryUrl(0, QString(), QUrl(path));

    if (!dirUrl.isEmpty()) {
        QString filepath = dirUrl.toDisplayString(QUrl::None);
        createBookmarkFromPath(filepath, QString(), SecurityBookmarkType::Directory);
        fileSelected = true;
    }
#endif
    return fileSelected;
}

QString KisMacosSecurityBookmarkManager::securityBookmarkTypeToString(const SecurityBookmarkType type)
{
    return QMetaEnum::fromType<SecurityBookmarkType>().valueToKey(type);
}

void KisMacosSecurityBookmarkManager::addBookmarkAndCheckParentDir(const QUrl &url)
{
    const QString path = url.toDisplayString(QUrl::None);
    qDebug() << "1 inserting to sandbox" << url << path;
    if (!parentDirHasPermissions(path)) {
        // we can't force the user to select a directory in particular
        // we add the bookmark even if the file root directory was selected
        createBookmarkFromPath(path, QString());

        requestAccessToDir(path);
    }
}

void KisMacosSecurityBookmarkManager::slotCreateBookmark(const QString &path)
{
    // necessary convert to get proper location
    QUrl url = QUrl::fromLocalFile(path);
    QString pathString = url.toDisplayString();
    if (!m_d->securedFiles.contains(pathString)) {
        createBookmarkFromPath(pathString, QString());
    }
}

bool KisMacosSecurityBookmarkManager::isSandboxed()
{
    return m_d->entitlements.sandbox();
}
