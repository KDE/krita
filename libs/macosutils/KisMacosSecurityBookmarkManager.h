/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2023 Ivan Santa María <ghevan@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KisMacosSecurityScopeBookmarkManager_h
#define KisMacosSecurityScopeBookmarkManager_h

#include <QObject>

class QString;
class QUrl;

class KisMacosSecurityBookmarkManager : public QObject
{
    Q_OBJECT
    
public:
    enum SecurityBookmarkType {
        File = 0,
        Directory
    };
    Q_ENUM(SecurityBookmarkType)

    static KisMacosSecurityBookmarkManager *instance();
    
    explicit KisMacosSecurityBookmarkManager();
    
    ~KisMacosSecurityBookmarkManager();
    /**
     * @return Return true if file is contained within a directory with permissions
     * previously granted by NSOpenPanel
     *
     */
    bool parentDirHasPermissions(const QString &path);
    
    /**
     * Creates a bookmark security scope for diven path
     * @param path File path returned from NSOpenPanel
     * @param refpath If non empty creates the security key relative to the
     *          reference path. This is useful for documents referring to other documents
     */
    void createBookmarkFromPath(const QString &path, const QString &refpath,SecurityBookmarkType type = SecurityBookmarkType::File);
    
    /**
     * starts access to all registered security bookmarks
     */
    void startAccessingSecurityScopedResources();

    /**
     * stops access to all started security bookmarks
     */
    void stopAccessingSecurityScopedResources();

    /**
     * Shows NSOpenPanel and saves the selected directory for future access
     *
     * @return TRUE if selected path is same as input path. users can still select
     *         any directory they want, but we specifically requested they give us
     *         permission to a specific directory
     *
     */
    bool requestAccessToDir(const QString &path);

    /**
     * Loads security scoped files to internal dictionary
     *
     */
    void loadSecurityScopedResources();
    
    bool isSandboxed();


public Q_SLOTS:
    void addBookmarkAndCheckParentDir(const QUrl &url);

    void slotCreateBookmark(const QString &path);
    
private:
    Q_DISABLE_COPY(KisMacosSecurityBookmarkManager)

    void loadKeysFromArray(SecurityBookmarkType);
    
    QString securityBookmarkTypeToString(const SecurityBookmarkType);
    
    QUrl decodeBookmarkToURL(QString encodedPath);
    
    class Private;
    const QScopedPointer<Private> m_d;
};

#endif /* KisMacosSecurityScopeBookmarkManager_h */
