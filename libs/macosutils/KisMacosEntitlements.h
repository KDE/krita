/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2023 Ivan Santa María <ghevan@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KisMacosEntitlements_h
#define KisMacosEntitlements_h

#include <QObject>

class KisMacosEntitlements : public QObject
{
    Q_OBJECT
    
public:

    enum Entitlements {
        Sandbox = 0, // com.apple.security.app-sandbox
        BookmarkScopeApp, // com.apple.security.files.bookmarks.app-scope
        BookmarkScopeDocument, // com.apple.security.files.bookmarks.document-scope
        Other = 99
    };

//    static KisMacosEntitlements *instance();
    
    KisMacosEntitlements();
    
    ~KisMacosEntitlements();
    
    
    void loadAvailableEntitlements(void);

    bool hasEntitlement(Entitlements);

    bool sandbox();

    
private:
//    Q_DISABLE_COPY(KisMacosEntitlements)
    
    class Private;
    const QScopedPointer<Private> m_d;
};


#endif /* KisMacosEntitlements_h */
