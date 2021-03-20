/*
   This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
   SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>

   Portions copyright:
   SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGINI_P_H
#define KCONFIGINI_P_H

#include <kconfigbackend_p.h>

class QLockFile;
class QIODevice;

class KConfigIniBackend : public KConfigBackend
{
    Q_OBJECT
private:
    QLockFile *lockFile;

public:
    class BufferFragment;

    KConfigIniBackend();
    ~KConfigIniBackend() override;

    ParseInfo parseConfig(const QByteArray &locale,
                          KEntryMap &entryMap,
                          ParseOptions options) override;

    ParseInfo parseConfig(const QByteArray &locale,
                          KEntryMap &entryMap,
                          ParseOptions options,
                          bool merging);
    ParseInfo parseConfigIO(QIODevice &file,
                            const QByteArray &locale,
                            KEntryMap &entryMap,
                            ParseOptions options,
                            bool merging);
    bool writeConfig(const QByteArray &locale, KEntryMap &entryMap,
                     WriteOptions options) override;

    bool isWritable() const override;
    QString nonWritableErrorMessage() const override;
    KConfigBase::AccessMode accessMode() const override;
    void createEnclosing() override;
    void setFilePath(const QString &path) override;
    bool lock() override;
    void unlock() override;
    bool isLocked() const override;

    void writeEntries(const QByteArray &locale, QIODevice &file, const KEntryMap &map);
    void writeEntries(const QByteArray &locale, QIODevice &file, const KEntryMap &map,
                      bool defaultGroup, bool &firstEntry);


protected:

    enum StringType {
        GroupString = 0,
        KeyString = 1,
        ValueString = 2
    };
    // Warning: this modifies data in-place. Other BufferFragment objects referencing the same buffer
    // fragment will get their data modified too.
    static void printableToString(BufferFragment *aString, const QIODevice &file, int line);
    static QByteArray stringToPrintable(const QByteArray &aString, StringType type);
    static char charFromHex(const char *str, const QIODevice &file, int line);
    static QString warningProlog(const QIODevice &file, int line);

};

#endif // KCONFIGINI_P_H
