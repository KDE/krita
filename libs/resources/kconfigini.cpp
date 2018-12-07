/*
   This file is part of the KDE libraries
   Copyright (c) 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (C) 1997-1999 Matthias Kalle Dalheimer (kalle@kde.org)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kconfigini_p.h"

#include "kconfig.h"
#include "kconfigbackend_p.h"
#include "bufferfragment_p.h"
#include "kconfigdata.h"

#include <qsavefile.h>
#include <qlockfile.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdebug.h>
#include <qplatformdefs.h>

#ifndef Q_OS_WIN
#include <unistd.h> // getuid, close
#endif
#include <sys/types.h> // uid_t
#include <fcntl.h> // open

bool kde_kiosk_exception = false; // flag to disable kiosk restrictions

static QByteArray lookup(const KConfigIniBackend::BufferFragment &fragment, QHash<KConfigIniBackend::BufferFragment, QByteArray> *cache)
{
    auto it = cache->constFind(fragment);
    if (it != cache->constEnd()) {
        return it.value();
    }

    return cache->insert(fragment, fragment.toByteArray()).value();
}

QString KConfigIniBackend::warningProlog(const QIODevice &file, int line)
{
    const QFile *f = dynamic_cast<const QFile*>(&file);
    if (f) {
        return QStringLiteral("KConfigIni: In file %2, line %1: ").arg(line).arg(f->fileName());
    }
    else {
        return QStringLiteral("KConfigIni: line %1: ").arg(line);
    }
}

KConfigIniBackend::KConfigIniBackend()
    : KConfigBackend(), lockFile(nullptr)
{
}

KConfigIniBackend::~KConfigIniBackend()
{
}

KConfigBackend::ParseInfo KConfigIniBackend::parseConfig(const QByteArray &currentLocale, KEntryMap &entryMap,
                               ParseOptions options)
{
    return parseConfig(currentLocale, entryMap, options, false);
}

// merging==true is the merging that happens at the beginning of writeConfig:
// merge changes in the on-disk file with the changes in the KConfig object.
KConfigBackend::ParseInfo KConfigIniBackend::parseConfig(const QByteArray &currentLocale, KEntryMap &entryMap, ParseOptions options, bool merging)
{
    if (filePath().isEmpty() || !QFile::exists(filePath())) {
        return ParseOk;
    }

    QFile file(filePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return ParseOpenError;
    }

    return parseConfigIO(file, currentLocale, entryMap, options, merging);
}


// merging==true is the merging that happens at the beginning of writeConfig:
// merge changes in the on-disk file with the changes in the KConfig object.
KConfigBackend::ParseInfo KConfigIniBackend::parseConfigIO(QIODevice &file, const QByteArray &currentLocale, KEntryMap &entryMap, ParseOptions options, bool merging)
{
    const QByteArray currentLanguage = currentLocale.split('_').first();

    bool bDefault = options & ParseDefaults;
    bool allowExecutableValues = options & ParseExpansions;

    QByteArray currentGroup("<default>");

    QList<QByteArray> immutableGroups;

    bool fileOptionImmutable = false;
    bool groupOptionImmutable = false;
    bool groupSkip = false;

    int lineNo = 0;
    // on systems using \r\n as end of line, \r will be taken care of by
    // trim() below
    QByteArray buffer = file.readAll();
    BufferFragment contents(buffer.data(), buffer.size());
    unsigned int len = contents.length();
    unsigned int startOfLine = 0;

    // Reduce memory overhead by making use of implicit sharing
    // This assumes that config files contain only a small amount of
    // different fragments which are repeated often.
    // This is often the case, especially sub groups will all have
    // the same list of keys and similar values as well.
    QHash<BufferFragment, QByteArray> cache;
    cache.reserve(4096);

    while (startOfLine < len) {
        BufferFragment line = contents.split('\n', &startOfLine);
        line.trim();
        lineNo++;

        // skip empty lines and lines beginning with '#'
        if (line.isEmpty() || line.at(0) == '#') {
            continue;
        }

        if (line.at(0) == '[') { // found a group
            groupOptionImmutable = fileOptionImmutable;

            QByteArray newGroup;
            int start = 1, end;
            do {
                end = start;
                for (;;) {
                    if (end == line.length()) {
                        qWarning() << warningProlog(file, lineNo) << "Invalid group header.";
                        // XXX maybe reset the current group here?
                        goto next_line;
                    }
                    if (line.at(end) == ']') {
                        break;
                    }
                    end++;
                }
                if (end + 1 == line.length() && start + 2 == end &&
                        line.at(start) == '$' && line.at(start + 1) == 'i') {
                    if (newGroup.isEmpty()) {
                        fileOptionImmutable = !kde_kiosk_exception;
                    } else {
                        groupOptionImmutable = !kde_kiosk_exception;
                    }
                } else {
                    if (!newGroup.isEmpty()) {
                        newGroup += '\x1d';
                    }
                    BufferFragment namePart = line.mid(start, end - start);
                    printableToString(&namePart, file, lineNo);
                    newGroup += namePart.toByteArray();
                }
            } while ((start = end + 2) <= line.length() && line.at(end + 1) == '[');
            currentGroup = newGroup;

            groupSkip = entryMap.getEntryOption(currentGroup, nullptr, nullptr, KEntryMap::EntryImmutable);

            if (groupSkip && !bDefault) {
                continue;
            }

            if (groupOptionImmutable)
                // Do not make the groups immutable until the entries from
                // this file have been added.
            {
                immutableGroups.append(currentGroup);
            }
        } else {
            if (groupSkip && !bDefault) {
                continue;    // skip entry
            }

            BufferFragment aKey;
            int eqpos = line.indexOf('=');
            if (eqpos < 0) {
                aKey = line;
                line.clear();
            } else {
                BufferFragment temp = line.left(eqpos);
                temp.trim();
                aKey = temp;
                line.truncateLeft(eqpos + 1);
                line.trim();
            }
            if (aKey.isEmpty()) {
                qWarning() << warningProlog(file, lineNo) << "Invalid entry (empty key)";
                continue;
            }

            KEntryMap::EntryOptions entryOptions = nullptr;
            if (groupOptionImmutable) {
                entryOptions |= KEntryMap::EntryImmutable;
            }

            BufferFragment locale;
            int start;
            while ((start = aKey.lastIndexOf('[')) >= 0) {
                int end = aKey.indexOf(']', start);
                if (end < 0) {
                    qWarning() << warningProlog(file, lineNo)
                               << "Invalid entry (missing ']')";
                    goto next_line;
                } else if (end > start + 1 && aKey.at(start + 1) == '$') { // found option(s)
                    int i = start + 2;
                    while (i < end) {
                        switch (aKey.at(i)) {
                        case 'i':
                            if (!kde_kiosk_exception) {
                                entryOptions |= KEntryMap::EntryImmutable;
                            }
                            break;
                        case 'e':
                            if (allowExecutableValues) {
                                entryOptions |= KEntryMap::EntryExpansion;
                            }
                            break;
                        case 'd':
                            entryOptions |= KEntryMap::EntryDeleted;
                            aKey = aKey.left(start);
                            printableToString(&aKey, file, lineNo);
                            entryMap.setEntry(currentGroup, aKey.toByteArray(), QByteArray(), entryOptions);
                            goto next_line;
                        default:
                            break;
                        }
                        i++;
                    }
                } else { // found a locale
                    if (!locale.isNull()) {
                        qWarning() << warningProlog(file, lineNo)
                                   << "Invalid entry (second locale!?)";
                        goto next_line;
                    }

                    locale = aKey.mid(start + 1, end - start - 1);
                }
                aKey.truncate(start);
            }
            if (eqpos < 0) { // Do this here after [$d] was checked
                qWarning() << warningProlog(file, lineNo) << "Invalid entry (missing '=')";
                continue;
            }
            printableToString(&aKey, file, lineNo);
            if (!locale.isEmpty()) {
                if (locale != currentLocale && locale != currentLanguage) {
                    // backward compatibility. C == en_US
                    if (locale.at(0) != 'C' || currentLocale != "en_US") {
                        if (merging) {
                            entryOptions |= KEntryMap::EntryRawKey;
                        } else {
                            goto next_line;    // skip this entry if we're not merging
                        }
                    }
                }
            }

            if (!(entryOptions & KEntryMap::EntryRawKey)) {
                printableToString(&aKey, file, lineNo);
            }

            if (options & ParseGlobal) {
                entryOptions |= KEntryMap::EntryGlobal;
            }
            if (bDefault) {
                entryOptions |= KEntryMap::EntryDefault;
            }
            if (!locale.isNull()) {
                entryOptions |= KEntryMap::EntryLocalized;
                if (locale.indexOf('_') != -1) {
                    entryOptions |= KEntryMap::EntryLocalizedCountry;
                }
            }
            printableToString(&line, file, lineNo);
            if (entryOptions & KEntryMap::EntryRawKey) {
                QByteArray rawKey;
                rawKey.reserve(aKey.length() + locale.length() + 2);
                rawKey.append(aKey.toVolatileByteArray());
                rawKey.append('[').append(locale.toVolatileByteArray()).append(']');
                entryMap.setEntry(currentGroup, rawKey, lookup(line, &cache), entryOptions);
            } else {
                entryMap.setEntry(currentGroup, lookup(aKey, &cache), lookup(line, &cache), entryOptions);
            }
        }
    next_line:
        continue;
    }

    // now make sure immutable groups are marked immutable
    Q_FOREACH (const QByteArray &group, immutableGroups) {
        entryMap.setEntry(group, QByteArray(), QByteArray(), KEntryMap::EntryImmutable);
    }

    return fileOptionImmutable ? ParseImmutable : ParseOk;
}

void KConfigIniBackend::writeEntries(const QByteArray &locale, QIODevice &file,
                                     const KEntryMap &map, bool defaultGroup, bool &firstEntry)
{
    QByteArray currentGroup;
    bool groupIsImmutable = false;
    const KEntryMapConstIterator end = map.constEnd();
    for (KEntryMapConstIterator it = map.constBegin(); it != end; ++it) {
        const KEntryKey &key = it.key();

        // Either process the default group or all others
        if ((key.mGroup != "<default>") == defaultGroup) {
            continue;    // skip
        }

        // the only thing we care about groups is, is it immutable?
        if (key.mKey.isNull()) {
            groupIsImmutable = it->bImmutable;
            continue; // skip
        }

        const KEntry &currentEntry = *it;
        if (!defaultGroup && currentGroup != key.mGroup) {
            if (!firstEntry) {
                file.putChar('\n');
            }
            currentGroup = key.mGroup;
            for (int start = 0, end;; start = end + 1) {
                file.putChar('[');
                end = currentGroup.indexOf('\x1d', start);
                if (end < 0) {
                    int cgl = currentGroup.length();
                    if (currentGroup.at(start) == '$' && cgl - start <= 10) {
                        for (int i = start + 1; i < cgl; i++) {
                            char c = currentGroup.at(i);
                            if (c < 'a' || c > 'z') {
                                goto nope;
                            }
                        }
                        file.write("\\x24");
                        start++;
                    }
                nope:
                    file.write(stringToPrintable(currentGroup.mid(start), GroupString));
                    file.putChar(']');
                    if (groupIsImmutable) {
                        file.write("[$i]", 4);
                    }
                    file.putChar('\n');
                    break;
                } else {
                    file.write(stringToPrintable(currentGroup.mid(start, end - start), GroupString));
                    file.putChar(']');
                }
            }
        }

        firstEntry = false;
        // it is data for a group

        if (key.bRaw) { // unprocessed key with attached locale from merge
            file.write(key.mKey);
        } else {
            file.write(stringToPrintable(key.mKey, KeyString)); // Key
            if (key.bLocal && locale != "C") { // 'C' locale == untranslated
                file.putChar('[');
                file.write(locale); // locale tag
                file.putChar(']');
            }
        }
        if (currentEntry.bDeleted) {
            if (currentEntry.bImmutable) {
                file.write("[$di]", 5);    // Deleted + immutable
            } else {
                file.write("[$d]", 4);    // Deleted
            }
        } else {
            if (currentEntry.bImmutable || currentEntry.bExpand) {
                file.write("[$", 2);
                if (currentEntry.bImmutable) {
                    file.putChar('i');
                }
                if (currentEntry.bExpand) {
                    file.putChar('e');
                }
                file.putChar(']');
            }
            file.putChar('=');
            file.write(stringToPrintable(currentEntry.mValue, ValueString));
        }
        file.putChar('\n');
    }
}

void KConfigIniBackend::writeEntries(const QByteArray &locale, QIODevice &file, const KEntryMap &map)
{
    bool firstEntry = true;

    // write default group
    writeEntries(locale, file, map, true, firstEntry);

    // write all other groups
    writeEntries(locale, file, map, false, firstEntry);
}

bool KConfigIniBackend::writeConfig(const QByteArray &locale, KEntryMap &entryMap,
                                    WriteOptions options)
{
    Q_ASSERT(!filePath().isEmpty());

    KEntryMap writeMap;
    const bool bGlobal = options & WriteGlobal;

    // First, reparse the file on disk, to merge our changes with the ones done by other apps
    // Store the result into writeMap.
    {
        ParseOptions opts = ParseExpansions;
        if (bGlobal) {
            opts |= ParseGlobal;
        }
        ParseInfo info = parseConfig(locale, writeMap, opts, true);
        if (info != ParseOk) { // either there was an error or the file became immutable
            return false;
        }
    }

    const KEntryMapIterator end = entryMap.end();
    for (KEntryMapIterator it = entryMap.begin(); it != end; ++it) {
        if (!it.key().mKey.isEmpty() && !it->bDirty) { // not dirty, doesn't overwrite entry in writeMap. skips default entries, too.
            continue;
        }

        const KEntryKey &key = it.key();

        // only write entries that have the same "globality" as the file
        if (it->bGlobal == bGlobal) {
            if (it->bReverted) {
                writeMap.remove(key);
            } else if (!it->bDeleted) {
                writeMap[key] = *it;
            } else {
                KEntryKey defaultKey = key;
                defaultKey.bDefault = true;
                if (!entryMap.contains(defaultKey)) {
                    writeMap.remove(key); // remove the deleted entry if there is no default
                    //qDebug() << "Detected as deleted=>removed:" << key.mGroup << key.mKey << "global=" << bGlobal;
                } else {
                    writeMap[key] = *it; // otherwise write an explicitly deleted entry
                    //qDebug() << "Detected as deleted=>[$d]:" << key.mGroup << key.mKey << "global=" << bGlobal;
                }
            }
            it->bDirty = false;
        }
    }

    // now writeMap should contain only entries to be written
    // so write it out to disk

    // check if file exists
    QFile::Permissions fileMode = QFile::ReadUser | QFile::WriteUser;
    bool createNew = true;

    QFileInfo fi(filePath());
    if (fi.exists()) {
#ifdef Q_OS_WIN
        //TODO: getuid does not exist on windows, use GetSecurityInfo and GetTokenInformation instead
        createNew = false;
#else
        if (fi.ownerId() == ::getuid()) {
            // Preserve file mode if file exists and is owned by user.
            fileMode = fi.permissions();
        } else {
            // File is not owned by user:
            // Don't create new file but write to existing file instead.
            createNew = false;
        }
#endif
    }

    if (createNew) {
        QSaveFile file(filePath());
        if (!file.open(QIODevice::WriteOnly)) {
            return false;
        }

        file.setTextModeEnabled(true); // to get eol translation
        writeEntries(locale, file, writeMap);

        if (!file.size() && (fileMode == (QFile::ReadUser | QFile::WriteUser))) {
            // File is empty and doesn't have special permissions: delete it.
            file.cancelWriting();

            if (fi.exists()) {
                // also remove the old file in case it existed. this can happen
                // when we delete all the entries in an existing config file.
                // if we don't do this, then deletions and revertToDefault's
                // will mysteriously fail
                QFile::remove(filePath());
            }
        } else {
            // Normal case: Close the file
            if (file.commit()) {
                QFile::setPermissions(filePath(), fileMode);
                return true;
            }
            // Couldn't write. Disk full?
            qWarning() << "Couldn't write" << filePath() << ". Disk full?";
            return false;
        }
    } else {
        // Open existing file. *DON'T* create it if it suddenly does not exist!
#ifdef Q_OS_UNIX
        int fd = QT_OPEN(QFile::encodeName(filePath()).constData(), O_WRONLY | O_TRUNC);
        if (fd < 0) {
            return false;
        }
        FILE *fp = ::fdopen(fd, "w");
        if (!fp) {
            QT_CLOSE(fd);
            return false;
        }
        QFile f;
        if (!f.open(fp, QIODevice::WriteOnly)) {
            fclose(fp);
            return false;
        }
        writeEntries(locale, f, writeMap);
        f.close();
        fclose(fp);
#else
        QFile f(filePath());
        // XXX This is broken - it DOES create the file if it is suddenly gone.
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            return false;
        }
        f.setTextModeEnabled(true);
        writeEntries(locale, f, writeMap);
#endif
    }
    return true;
}

bool KConfigIniBackend::isWritable() const
{
    const QString filePath = this->filePath();
    if (!filePath.isEmpty()) {
        QFileInfo file(filePath);
        if (!file.exists()) {
            // If the file does not exist, check if the deepest
            // existing dir is writable.
            QFileInfo dir(file.absolutePath());
            while (!dir.exists()) {
                QString parent = dir.absolutePath(); // Go up. Can't use cdUp() on non-existing dirs.
                if (parent == dir.filePath()) {
                    // no parent
                    return false;
                }
                dir.setFile(parent);
            }
            return dir.isDir() && dir.isWritable();
        } else {
            return file.isWritable();
        }
    }

    return false;
}

QString KConfigIniBackend::nonWritableErrorMessage() const
{
    return tr("Configuration file \"%1\" not writable.\n").arg(filePath());
}

void KConfigIniBackend::createEnclosing()
{
    const QString file = filePath();
    if (file.isEmpty()) {
        return;    // nothing to do
    }

    // Create the containing dir, maybe it wasn't there
    QDir dir;
    dir.mkpath(QFileInfo(file).absolutePath());
}

void KConfigIniBackend::setFilePath(const QString &file)
{
    if (file.isEmpty()) {
        return;
    }

    Q_ASSERT(QDir::isAbsolutePath(file));

    const QFileInfo info(file);
    if (info.exists()) {
        setLocalFilePath(info.canonicalFilePath());
    } else {
        setLocalFilePath(file);
    }
}

KConfigBase::AccessMode KConfigIniBackend::accessMode() const
{
    if (filePath().isEmpty()) {
        return KConfigBase::NoAccess;
    }

    if (isWritable()) {
        return KConfigBase::ReadWrite;
    }

    return KConfigBase::ReadOnly;
}

bool KConfigIniBackend::lock()
{
    Q_ASSERT(!filePath().isEmpty());

    if (!lockFile) {
        lockFile = new QLockFile(filePath() + QLatin1String(".lock"));
    }

    lockFile->lock();
    return lockFile->isLocked();
}

void KConfigIniBackend::unlock()
{
    lockFile->unlock();
    delete lockFile;
    lockFile = nullptr;
}

bool KConfigIniBackend::isLocked() const
{
    return lockFile && lockFile->isLocked();
}

QByteArray KConfigIniBackend::stringToPrintable(const QByteArray &aString, StringType type)
{
    static const char nibbleLookup[] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    if (aString.isEmpty()) {
        return aString;
    }
    const int l = aString.length();

    QByteArray result; // Guesstimated that it's good to avoid data() initialization for a length of l*4
    result.resize(l * 4); // Maximum 4x as long as source string due to \x<ab> escape sequences
    const char *s = aString.constData();
    int i = 0;
    char *data = result.data();
    char *start = data;

    // Protect leading space
    if (s[0] == ' ' && type != GroupString) {
        *data++ = '\\';
        *data++ = 's';
        i++;
    }

    for (; i < l; ++i/*, r++*/) {
        switch (s[i]) {
        default:
            // The \n, \t, \r cases (all < 32) are handled below; we can ignore them here
            if (((unsigned char)s[i]) < 32) {
                goto doEscape;
            }
            *data++ = s[i];
            break;
        case '\n':
            *data++ = '\\';
            *data++ = 'n';
            break;
        case '\t':
            *data++ = '\\';
            *data++ = 't';
            break;
        case '\r':
            *data++ = '\\';
            *data++ = 'r';
            break;
        case '\\':
            *data++ = '\\';
            *data++ = '\\';
            break;
        case '=':
            if (type != KeyString) {
                *data++ = s[i];
                break;
            }
            goto doEscape;
        case '[':
        case ']':
            // Above chars are OK to put in *value* strings as plaintext
            if (type == ValueString) {
                *data++ = s[i];
                break;
            }
        doEscape:
            *data++ = '\\';
            *data++ = 'x';
            *data++ = nibbleLookup[((unsigned char)s[i]) >> 4];
            *data++ = nibbleLookup[((unsigned char)s[i]) & 0x0f];
            break;
        }
    }
    *data = 0;
    result.resize(data - start);

    // Protect trailing space
    if (result.endsWith(' ') && type != GroupString) {
        result.replace(result.length() - 1, 1, "\\s");
    }

    return result;
}

char KConfigIniBackend::charFromHex(const char *str, const QIODevice &file, int line)
{
    unsigned char ret = 0;
    for (int i = 0; i < 2; i++) {
        ret <<= 4;
        quint8 c = quint8(str[i]);

        if (c >= '0' && c <= '9') {
            ret |= c - '0';
        } else if (c >= 'a' && c <= 'f') {
            ret |= c - 'a' + 0x0a;
        } else if (c >= 'A' && c <= 'F') {
            ret |= c - 'A' + 0x0a;
        } else {
            QByteArray e(str, 2);
            e.prepend("\\x");
            qWarning() << warningProlog(file, line) << "Invalid hex character " << c
                       << " in \\x<nn>-type escape sequence \"" << e.constData() << "\".";
            return 'x';
        }
    }
    return char(ret);
}

void KConfigIniBackend::printableToString(BufferFragment *aString, const QIODevice &file, int line)
{
    if (aString->isEmpty() || aString->indexOf('\\') == -1) {
        return;
    }
    aString->trim();
    int l = aString->length();
    char *r = aString->data();
    char *str = r;

    for (int i = 0; i < l; i++, r++) {
        if (str[i] != '\\') {
            *r = str[i];
        } else {
            // Probable escape sequence
            i++;
            if (i >= l) { // Line ends after backslash - stop.
                *r = '\\';
                break;
            }

            switch (str[i]) {
            case 's':
                *r = ' ';
                break;
            case 't':
                *r = '\t';
                break;
            case 'n':
                *r = '\n';
                break;
            case 'r':
                *r = '\r';
                break;
            case '\\':
                *r = '\\';
                break;
            case ';':
                // not really an escape sequence, but allowed in .desktop files, don't strip '\;' from the string
                *r = '\\';
                r++;
                *r = ';';
                break;
            case 'x':
                if (i + 2 < l) {
                    *r = charFromHex(str + i + 1, file, line);
                    i += 2;
                } else {
                    *r = 'x';
                    i = l - 1;
                }
                break;
            default:
                *r = '\\';
                qWarning() << warningProlog(file, line)
                           << QStringLiteral("Invalid escape sequence \"\\%1\".").arg(str[i]);
            }
        }
    }
    aString->truncate(r - aString->constData());
}
