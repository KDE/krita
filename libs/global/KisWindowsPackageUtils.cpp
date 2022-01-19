/*
 * SPDX-FileCopyrightText: 2022 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisWindowsPackageUtils.h"

#include <windows.h>

#include <QDebug>
#include <QString>


constexpr int appmodel_PACKAGE_FULL_NAME_MAX_LENGTH = 127;

constexpr LONG winerror_APPMODEL_ERROR_NO_PACKAGE = 15700;

// ---
// GetCurrentPackageFamilyName
// appmodel.h / Kernel32.dll / Windows 8
// ---
typedef LONG (WINAPI *pGetCurrentPackageFamilyName_t)(
    UINT32 *packageFamilyNameLength,
    PWSTR packageFamilyName
);

// ---
// GetCurrentPackageFullName
// appmodel.h / Kernel32.dll / Windows 8
// ---
typedef LONG (WINAPI *pGetCurrentPackageFullName_t)(
    UINT32 *packageFullNameLength,
    PWSTR packageFullName
);

struct AppmodelFunctions {
    pGetCurrentPackageFamilyName_t pGetCurrentPackageFamilyName;
    pGetCurrentPackageFullName_t pGetCurrentPackageFullName;

    AppmodelFunctions() {
        HMODULE dllKernel32 = LoadLibraryW(L"kernel32.dll");
        pGetCurrentPackageFamilyName = reinterpret_cast<pGetCurrentPackageFamilyName_t>(
            GetProcAddress(dllKernel32, "GetCurrentPackageFamilyName"));
        pGetCurrentPackageFullName = reinterpret_cast<pGetCurrentPackageFullName_t>(
            GetProcAddress(dllKernel32, "GetCurrentPackageFullName"));
    }
};

static const AppmodelFunctions &f()
{
    static AppmodelFunctions s_functions;
    return s_functions;
} 

namespace KisWindowsPackageUtils
{

bool isRunningInPackage()
{
    return tryGetCurrentPackageFamilyName(nullptr);
}

bool tryGetCurrentPackageFamilyName(QString *outName)
{
    if (!f().pGetCurrentPackageFamilyName) {
        // We are probably on Windows 7 or earlier.
        return false;
    }

    WCHAR name[appmodel_PACKAGE_FULL_NAME_MAX_LENGTH + 1]; // includes null terminator
    UINT32 nameLength = sizeof(name) / sizeof(name[0]);
    LONG result = f().pGetCurrentPackageFamilyName(&nameLength, name);
    if (result == winerror_APPMODEL_ERROR_NO_PACKAGE) {
        // Process not running from a package.
        return false;
    }
    if (result == ERROR_INSUFFICIENT_BUFFER) {
        // This shouldn't happen!
        qWarning() << "GetCurrentPackageFamilyName returned ERROR_INSUFFICIENT_BUFFER, required length is" << nameLength;
        if (outName) {
            *outName = QString();
        }
        return true;
    }
    if (result != ERROR_SUCCESS) {
        qWarning() << "GetCurrentPackageFamilyName returned unexpected error code:" << result;
        return false;
    }

    if (outName) {
        // Sanity check
        if (nameLength > sizeof(name) / sizeof(name[0])) {
            qWarning() << "GetCurrentPackageFamilyName returned a length exceeding the buffer size:" << nameLength;
            nameLength = sizeof(name) / sizeof(name[0]);
        }
        // Exclude null terminator
        if (nameLength > 0 && name[nameLength - 1] == L'\0') {
            nameLength -= 1;
        }
        *outName = QString::fromWCharArray(name, nameLength);
    }
    return true;
}

bool tryGetCurrentPackageFullName(QString *outName)
{
    if (!f().pGetCurrentPackageFullName) {
        // We are probably on Windows 7 or earlier.
        return false;
    }

    WCHAR name[appmodel_PACKAGE_FULL_NAME_MAX_LENGTH + 1]; // includes null terminator
    UINT32 nameLength = sizeof(name) / sizeof(name[0]);
    LONG result = f().pGetCurrentPackageFullName(&nameLength, name);
    if (result == winerror_APPMODEL_ERROR_NO_PACKAGE) {
        // Process not running from a package.
        return false;
    }
    if (result == ERROR_INSUFFICIENT_BUFFER) {
        // This shouldn't happen!
        qWarning() << "GetCurrentPackageFullName returned ERROR_INSUFFICIENT_BUFFER, required length is" << nameLength;
        if (outName) {
            *outName = QString();
        }
        return true;
    }
    if (result != ERROR_SUCCESS) {
        qWarning() << "GetCurrentPackageFullName returned unexpected error code:" << result;
        return false;
    }

    if (outName) {
        // Sanity check
        if (nameLength > sizeof(name) / sizeof(name[0])) {
            qWarning() << "GetCurrentPackageFullName returned a length exceeding the buffer size:" << nameLength;
            nameLength = sizeof(name) / sizeof(name[0]);
        }
        // Exclude null terminator
        if (nameLength > 0 && name[nameLength - 1] == L'\0') {
            nameLength -= 1;
        }
        *outName = QString::fromWCharArray(name, nameLength);
    }
    return true;
}

} /* namespace KisWindowsPackageUtils */
