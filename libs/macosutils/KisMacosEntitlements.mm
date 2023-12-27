/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2023 Ivan Santa María <ghevan@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisMacosEntitlements.h"

#import <Foundation/Foundation.h>

#include <QHash>
#include <QVariant>
#include <QString>

#if ! __has_feature(objc_arc)
#error "Enable ARC to compile this file"
#endif

//Q_GLOBAL_STATIC(KisMacosEntitlements, s_instance)

class KisMacosEntitlements::Private
{
public:
    Private()
        : entitlements(QHash<Entitlements,QVariant>())
    {
    }

    ~Private()
    {
    }

    QHash<Entitlements, QVariant> entitlements;
};

//KisMacosEntitlements* KisMacosEntitlements::instance()
//{
//    return s_instance;
//}


KisMacosEntitlements::KisMacosEntitlements()
    : m_d(new Private())
{
        loadAvailableEntitlements();
}

KisMacosEntitlements::~KisMacosEntitlements()
{
}


void KisMacosEntitlements::loadAvailableEntitlements(void)
{
    OSStatus status;
    SecCodeRef kritaRef = NULL;
    CFDictionaryRef dynamicInfo = NULL;

    status = SecCodeCopySelf(kSecCSDefaultFlags, &kritaRef);
//    KIS_ASSERT_RECOVER_RETURN_VALUE(status == errSecSuccess, isSandboxed);

    SecCodeCopySigningInformation(kritaRef, (SecCSFlags) kSecCSDynamicInformation, &dynamicInfo);
//    KIS_ASSERT_RECOVER_RETURN_VALUE(status == errSecSuccess, isSandboxed);

    CFDictionaryRef rawEntitlements = (CFDictionaryRef)CFDictionaryGetValue(dynamicInfo, kSecCodeInfoEntitlementsDict);

    if (rawEntitlements) {
        NSDictionary *entitlementsDir = (__bridge NSDictionary*)rawEntitlements;

        for (NSString *key in entitlementsDir) {
            id value = entitlementsDir[key];
            if ([key isEqualToString:@"com.apple.security.app-sandbox"]) {
                m_d->entitlements[Entitlements::Sandbox] = [value boolValue];
            }
            else if ([key isEqualToString:@"com.apple.security.files.bookmarks.app-scope"]) {
                m_d->entitlements[Entitlements::BookmarkScopeApp] = [value boolValue];
            }
            else if ([key isEqualToString:@"com.apple.security.files.bookmarks.document-scope"]) {
                m_d->entitlements[Entitlements::BookmarkScopeDocument] = [value boolValue];
            }
        }
    }

    if (dynamicInfo != NULL) {
        CFRelease(dynamicInfo);
    }
    if (kritaRef != NULL) {
        CFRelease(kritaRef);
    }

    return;
}

bool KisMacosEntitlements::hasEntitlement(Entitlements key)
{
    // we assume if given entitlement exists then its true, but this is not always the case
    return m_d->entitlements.contains(key);
}

bool KisMacosEntitlements::sandbox()
{
    return m_d->entitlements.contains(Entitlements::Sandbox);
}
