/*
 * SPDX-FileCopyrightText: 2022 Alvin Wong <alvin@alvinhc.com>
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

// Get Windows Vista API
#if defined(WINVER) && WINVER < 0x0600
#undef WINVER
#endif
#if defined(_WIN32_WINNT) && _WIN32_WINNT < 0x0600
#undef _WIN32_WINNT
#endif
#ifndef WINVER
#define WINVER 0x0600
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include "KisWindowsPackageUtils.h"

#include <array>

#include <Shlobj.h>
#include <windows.h>

#include <QDebug>
#include <QString>

constexpr int appmodel_PACKAGE_FULL_NAME_MAX_LENGTH = 127;

constexpr LONG winerror_APPMODEL_ERROR_NO_PACKAGE = 15700;

// ---
// GetCurrentPackageFamilyName
// appmodel.h / Kernel32.dll / Windows 8
// ---
using pGetCurrentPackageFamilyName_t =
    LONG(WINAPI *)(UINT32 *packageFamilyNameLength, PWSTR packageFamilyName);

// ---
// GetCurrentPackageFullName
// appmodel.h / Kernel32.dll / Windows 8
// ---
using pGetCurrentPackageFullName_t =
    LONG(WINAPI *)(UINT32 *packageFullNameLength, PWSTR packageFullName);

// Flag for `KNOWN_FOLDER_FLAG`, introduced in Win 10 ver 1703, which when
// used within a Desktop Bridge process, will cause the API to return the
// redirected target of the locations.
//
// ---
// KF_FLAG_RETURN_FILTER_REDIRECTION_TARGET
// shlobj_core.h / Windows 10 v1703
// ---
constexpr int shlobj_KF_FLAG_RETURN_FILTER_REDIRECTION_TARGET = 0x00040000;

struct AppmodelFunctions {
    pGetCurrentPackageFamilyName_t getCurrentPackageFamilyName{};
    pGetCurrentPackageFullName_t getCurrentPackageFullName{};
    HMODULE dllKernel32;

    template<typename T, typename U>
    inline T cast_to_function(U v) noexcept
    {
        return reinterpret_cast<T>(reinterpret_cast<void *>(v));
    }

    static const AppmodelFunctions &instance()
    {
        static const AppmodelFunctions s{};
        return s;
    }

    AppmodelFunctions()
        : dllKernel32(LoadLibrary(TEXT("kernel32.dll")))
    {
        getCurrentPackageFamilyName =
            cast_to_function<pGetCurrentPackageFamilyName_t>(
                GetProcAddress(dllKernel32, "GetCurrentPackageFamilyName"));
        getCurrentPackageFullName =
            cast_to_function<pGetCurrentPackageFullName_t>(
                GetProcAddress(dllKernel32, "GetCurrentPackageFullName"));
    }

    ~AppmodelFunctions()
    {
        FreeLibrary(dllKernel32);
    }
};

namespace KisWindowsPackageUtils
{

bool isRunningInPackage()
{
    return tryGetCurrentPackageFamilyName(nullptr);
}

bool tryGetCurrentPackageFamilyName(QString *outName)
{
    if (!AppmodelFunctions::instance().getCurrentPackageFamilyName) {
        // We are probably on Windows 7 or earlier.
        return false;
    }

    std::array<WCHAR, appmodel_PACKAGE_FULL_NAME_MAX_LENGTH + 1>
        name{}; // includes null terminator
    UINT32 nameLength = name.size();
    LONG result =
        AppmodelFunctions::instance().getCurrentPackageFamilyName(&nameLength,
                                                                  name.data());
    if (result == winerror_APPMODEL_ERROR_NO_PACKAGE) {
        // Process not running from a package.
        return false;
    }
    if (result == ERROR_INSUFFICIENT_BUFFER) {
        // This shouldn't happen!
        qWarning() << "GetCurrentPackageFamilyName returned "
                      "ERROR_INSUFFICIENT_BUFFER, required length is"
                   << nameLength;
        if (outName) {
            *outName = QString();
        }
        return true;
    }
    if (result != ERROR_SUCCESS) {
        qWarning()
            << "GetCurrentPackageFamilyName returned unexpected error code:"
            << result;
        return false;
    }

    if (outName) {
        // Sanity check
        if (nameLength > name.size()) {
            qWarning() << "GetCurrentPackageFamilyName returned a length "
                          "exceeding the buffer size:"
                       << nameLength;
            nameLength = name.size();
        }
        // Exclude null terminator
        if (nameLength > 0 && name.at(nameLength - 1) == L'\0') {
            nameLength -= 1;
        }
        *outName =
            QString::fromWCharArray(name.data(), static_cast<int>(nameLength));
    }
    return true;
}

bool tryGetCurrentPackageFullName(QString *outName)
{
    if (!AppmodelFunctions::instance().getCurrentPackageFullName) {
        // We are probably on Windows 7 or earlier.
        return false;
    }

    std::array<WCHAR, appmodel_PACKAGE_FULL_NAME_MAX_LENGTH + 1>
        name{}; // includes null terminator
    UINT32 nameLength = name.size();
    const LONG result =
        AppmodelFunctions::instance().getCurrentPackageFullName(&nameLength,
                                                                name.data());
    if (result == winerror_APPMODEL_ERROR_NO_PACKAGE) {
        // Process not running from a package.
        return false;
    }
    if (result == ERROR_INSUFFICIENT_BUFFER) {
        // This shouldn't happen!
        qWarning() << "GetCurrentPackageFullName returned "
                      "ERROR_INSUFFICIENT_BUFFER, required length is"
                   << nameLength;
        if (outName) {
            *outName = QString();
        }
        return true;
    }
    if (result != ERROR_SUCCESS) {
        qWarning()
            << "GetCurrentPackageFullName returned unexpected error code:"
            << result;
        return false;
    }

    if (outName) {
        // Sanity check
        if (nameLength > name.size()) {
            qWarning() << "GetCurrentPackageFullName returned a length "
                          "exceeding the buffer size:"
                       << nameLength;
            nameLength = name.size();
        }
        // Exclude null terminator
        if (nameLength > 0 && name.at(nameLength - 1) == L'\0') {
            nameLength -= 1;
        }
        *outName =
            QString::fromWCharArray(name.data(), static_cast<int>(nameLength));
    }
    return true;
}

QString getPackageRoamingAppDataLocation()
{
    PWSTR path = nullptr;
    HRESULT result =
        SHGetKnownFolderPath(FOLDERID_RoamingAppData,
                             shlobj_KF_FLAG_RETURN_FILTER_REDIRECTION_TARGET,
                             nullptr,
                             &path);
    if (result != S_OK) {
        qWarning() << "SHGetKnownFolderPath returned error HRESULT:" << result;
        return {};
    }
    if (!path) {
        qWarning() << "SHGetKnownFolderPath did not return a path";
        return {};
    }
    QString appData = QString::fromWCharArray(path);
    CoTaskMemFree(path);
    return appData;
}

} /* namespace KisWindowsPackageUtils */
