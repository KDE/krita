/* This file is part of the KDE project
   Copyright (C) 2006 Thomas Schaap <thomas.schaap@kdemail.net>
   Copyright (C) 2010 C. Boemann <cbo@boemann.dk>

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
#ifdef QCA2

#include "KoEncryptedStore.h"
#include "KoEncryptionChecker.h"
#include "KoStore_p.h"
#include "KoXmlReader.h"
#include <KoXmlNS.h>

#include <QString>
#include <QByteArray>
#include <QIODevice>
#include <QWidget>
#include <QBuffer>
#include <kpassworddialog.h>
#include <knewpassworddialog.h>
#include <kwallet.h>
#include <klocale.h>
#include <kfilterdev.h>
#include <kmessage.h>
#include <kmessagebox.h>
#include <kzip.h>
#include <kio/netaccess.h>
#include <ktemporaryfile.h>
#include <kdebug.h>

struct KoEncryptedStore_EncryptionData {
    // Needed for Key Derivation
    QCA::SecureArray salt;
    unsigned int iterationCount;

    // Needed for enc/decryption
    QCA::SecureArray initVector;

    // Needed for (optional) password-checking
    QCA::SecureArray checksum;
    // checksumShort is set to true if the checksum-algorithm is SHA1/1K, which basically means we only use the first 1024 bytes of the unencrypted file to check against (see also http://www.openoffice.org/servlets/ReadMsg?list=dev&msgNo=17498)
    bool checksumShort;

    // The size of the uncompressed file
    qint64 filesize;
};

// TODO: Discuss naming of this filer in saving-dialogues
// TODO: Discuss possibility of allowing programs to remember the password after opening to enable them to supply it when saving
// TODO: Discuss autosaving and password/leakage-problem (currently: hardcoded no autosave)
namespace
{
const char* MANIFEST_FILE = "META-INF/manifest.xml";
const char* META_FILE = "meta.xml";
const char* THUMBNAIL_FILE = "Thumbnails/thumbnail.png";
}

KoEncryptedStore::KoEncryptedStore(const QString & filename, Mode mode, const QByteArray & appIdentification)
        : m_qcaInit(QCA::Initializer()), m_password(QCA::SecureArray()), m_filename(QString(filename)), m_manifestBuffer(QByteArray()), m_tempFile(NULL), m_bPasswordUsed(false), m_bPasswordDeclined(false), m_currentDir(NULL)
{
    Q_D(KoStore);

    m_pZip = new KZip(filename);
    d->good = true;
    d->localFileName = filename;

    init(mode, appIdentification);
}

KoEncryptedStore::KoEncryptedStore(QIODevice *dev, Mode mode, const QByteArray & appIdentification)
        : m_qcaInit(QCA::Initializer()), m_password(QCA::SecureArray()), m_filename(QString()), m_manifestBuffer(QByteArray()), m_tempFile(NULL), m_bPasswordUsed(false), m_bPasswordDeclined(false), m_currentDir(NULL)
{
    Q_D(KoStore);

    m_pZip = new KZip(dev);
    d->good = true;

    init(mode, appIdentification);
}

KoEncryptedStore::KoEncryptedStore(QWidget* window, const KUrl& url, const QString & filename, Mode mode, const QByteArray & appIdentification)
        : m_qcaInit(QCA::Initializer()), m_password(QCA::SecureArray()), m_filename(QString(url.url())), m_manifestBuffer(QByteArray()), m_tempFile(NULL), m_bPasswordUsed(false), m_bPasswordDeclined(false), m_currentDir(NULL)
{
    Q_D(KoStore);

    d->window = window;
    d->good = true;

    if (mode == Read) {
        d->fileMode = KoStorePrivate::RemoteRead;
        d->localFileName = filename;
        m_pZip = new KZip(d->localFileName);
    } else {
        d->fileMode = KoStorePrivate::RemoteWrite;
        m_tempFile = new KTemporaryFile();
        if (!m_tempFile->open()) {
            d->good = false;
        } else {
            d->localFileName = m_tempFile->fileName();
            m_pZip = new KZip(m_tempFile);
        }
    }
    d->url = url;

    init(mode, appIdentification);
}

bool KoEncryptedStore::init(Mode mode, const QByteArray & appIdentification)
{
    Q_D(KoStore);
    bool checksumErrorShown = false;
    bool unreadableErrorShown = false;
    if (!KoStore::init(mode) || !d->good) {
        // This Store is already bad
        d->good = false;
        return false;
    }
    d->mode = mode;
    if (mode == Write) {
        d->good = KoEncryptionChecker::isEncryptionSupported();
        if (d->good) {
            if (!m_pZip->open(QIODevice::WriteOnly)) {
                d->good = false;
                return false;
            }
            m_pZip->setCompression(KZip::NoCompression);
            m_pZip->setExtraField(KZip::NoExtraField);
            // Write identification
            (void)m_pZip->writeFile("mimetype", "", "", appIdentification.data(), appIdentification.length());
            m_pZip->setCompression(KZip::DeflateCompression);
            // We don't need the extra field in Calligra - so we leave it as "no extra field".
        }
    } else {
        d->good = m_pZip->open(QIODevice::ReadOnly);
        d->good &= m_pZip->directory() != 0;
        if (!d->good) {
            return false;
        }

        // Read the manifest-file, so we can get the data we'll need to decrypt the other files in the store
        const KArchiveEntry* manifestArchiveEntry = m_pZip->directory()->entry(MANIFEST_FILE);
        if (!manifestArchiveEntry || !manifestArchiveEntry->isFile()) {
            // No manifest file? OK, *I* won't complain
            return true;
        }
        QIODevice *dev = (static_cast< const KArchiveFile* >(manifestArchiveEntry))->createDevice();

        KoXmlDocument xmldoc;
        bool namespaceProcessing = true; // for the manifest ignore the namespace (bug #260515)
        if (!xmldoc.setContent(dev, namespaceProcessing) || xmldoc.documentElement().localName() != "manifest" || xmldoc.documentElement().namespaceURI() != KoXmlNS::manifest) {
            KMessage::message(KMessage::Warning, i18n("The manifest file seems to be corrupted. The document could not be opened."));
            dev->close();
            delete dev;
            m_pZip->close();
            d->good = false;
            return false;
        }
        KoXmlElement xmlroot = xmldoc.documentElement();
        if (xmlroot.hasChildNodes()) {
            QCA::Base64 base64decoder(QCA::Decode);
            KoXmlNode xmlnode = xmlroot.firstChild();
            while (!xmlnode.isNull()) {
                // Search for files
                if (!xmlnode.isElement() || xmlroot.namespaceURI() != KoXmlNS::manifest || xmlnode.toElement().localName() != "file-entry" || !xmlnode.toElement().hasAttribute("full-path") || !xmlnode.hasChildNodes()) {
                    xmlnode = xmlnode.nextSibling();
                    continue;
                }

                // Build a structure to hold the data and fill it with defaults
                KoEncryptedStore_EncryptionData encData;
                encData.filesize = 0;
                encData.checksum = QCA::SecureArray();
                encData.checksumShort = false;
                encData.salt = QCA::SecureArray();
                encData.iterationCount = 0;
                encData.initVector = QCA::SecureArray();

                // Get some info about the file
                QString fullpath = xmlnode.toElement().attribute("full-path");

                if (xmlnode.toElement().hasAttribute("size")) {
                    encData.filesize = xmlnode.toElement().attribute("size").toUInt();
                }

                // Find the embedded encryption-data block
                KoXmlNode xmlencnode = xmlnode.firstChild();
                while (!xmlencnode.isNull() && (!xmlencnode.isElement() || xmlencnode.toElement().localName() != "encryption-data" || !xmlencnode.hasChildNodes())) {
                    xmlencnode = xmlencnode.nextSibling();
                }
                if (xmlencnode.isNull()) {
                    xmlnode = xmlnode.nextSibling();
                    continue;
                }

                // Find some things about the checksum
                if (xmlencnode.toElement().hasAttribute("checksum")) {
                    base64decoder.clear();
                    encData.checksum = base64decoder.decode(QCA::SecureArray(xmlencnode.toElement().attribute("checksum").toAscii()));
                    if (xmlencnode.toElement().hasAttribute("checksum-type")) {
                        QString checksumType = xmlencnode.toElement().attribute("checksum-type");
                        if (checksumType == "SHA1") {
                            encData.checksumShort = false;
                        }
                        // For this particual hash-type: check KoEncryptedStore_encryptionData.checksumShort
                        else if (checksumType == "SHA1/1K") {
                            encData.checksumShort = true;
                        } else {
                            // Checksum type unknown
                            if (!checksumErrorShown) {
                                KMessage::message(KMessage::Warning, i18n("This document contains an unknown checksum. When you give a password it might not be verified."));
                                checksumErrorShown = true;
                            }
                            encData.checksum = QCA::SecureArray();
                        }
                    } else {
                        encData.checksumShort = false;
                    }
                }

                KoXmlNode xmlencattr = xmlencnode.firstChild();
                bool algorithmFound = false;
                bool keyDerivationFound = false;
                // Search all data about encrption
                while (!xmlencattr.isNull()) {
                    if (!xmlencattr.isElement()) {
                        xmlencattr = xmlencattr.nextSibling();
                        continue;
                    }

                    // Find some things about the encryption algorithm
                    if (xmlencattr.toElement().localName() == "algorithm" && xmlencattr.toElement().hasAttribute("initialisation-vector")) {
                        algorithmFound = true;
                        encData.initVector = base64decoder.decode(QCA::SecureArray(xmlencattr.toElement().attribute("initialisation-vector").toAscii()));
                        if (xmlencattr.toElement().hasAttribute("algorithm-name") && xmlencattr.toElement().attribute("algorithm-name") != "Blowfish CFB") {
                            if (!unreadableErrorShown) {
                                KMessage::message(KMessage::Warning, i18n("This document contains an unknown encryption method. Some parts may be unreadable."));
                                unreadableErrorShown = true;
                            }
                            encData.initVector = QCA::SecureArray();
                        }
                    }

                    // Find some things about the key derivation
                    if (xmlencattr.toElement().localName() == "key-derivation" && xmlencattr.toElement().hasAttribute("salt")) {
                        keyDerivationFound = true;
                        encData.salt = base64decoder.decode(QCA::SecureArray(xmlencattr.toElement().attribute("salt").toAscii()));
                        encData.iterationCount = 1024;
                        if (xmlencattr.toElement().hasAttribute("iteration-count")) {
                            encData.iterationCount = xmlencattr.toElement().attribute("iteration-count").toUInt();
                        }
                        if (xmlencattr.toElement().hasAttribute("key-derivation-name") && xmlencattr.toElement().attribute("key-derivation-name") != "PBKDF2") {
                            if (!unreadableErrorShown) {
                                KMessage::message(KMessage::Warning, i18n("This document contains an unknown encryption method. Some parts may be unreadable."));
                                unreadableErrorShown = true;
                            }
                            encData.salt = QCA::SecureArray();
                        }
                    }

                    xmlencattr = xmlencattr.nextSibling();
                }

                // Only use this encryption data if it makes sense to use it
                if (!(encData.salt.isEmpty() || encData.initVector.isEmpty())) {
                    m_encryptionData.insert(fullpath, encData);
                    if (!(algorithmFound && keyDerivationFound)) {
                        if (!unreadableErrorShown) {
                            KMessage::message(KMessage::Warning, i18n("This document contains incomplete encryption data. Some parts may be unreadable."));
                            unreadableErrorShown = true;
                        }
                    }
                }

                xmlnode = xmlnode.nextSibling();
            }
        }
        dev->close();
        delete dev;

        if (isEncrypted() && !(QCA::isSupported("sha1") && QCA::isSupported("pbkdf2(sha1)") && QCA::isSupported("blowfish-cfb"))) {
            d->good = false;
            KMessage::message(KMessage::Error, i18n("QCA has currently no support for SHA1 or PBKDF2 using SHA1. The document can not be opened."));
        }
    }
    return d->good;
}

bool KoEncryptedStore::doFinalize()
{
    Q_D(KoStore);
    if (d->good) {
        if (isOpen()) {
            close();
        }
        if (d->mode == Write) {
            // First change the manifest file and write it
            // We'll use the QDom classes here, since KoXmlReader and KoXmlWriter have no way of copying a complete xml-file
            // other than parsing it completely and rebuilding it.
            // Errorhandling here is done to prevent data from being lost whatever happens
            // TODO: Convert this to KoXML when KoXML is extended enough
            // Note: right now this is impossible due to lack of possibilities to copy an element as-is
            QDomDocument document;
            if (m_manifestBuffer.isEmpty()) {
                // No manifest? Better create one
                document = QDomDocument();
                QDomElement rootElement = document.createElement("manifest:manifest");
                rootElement.setAttribute("xmlns:manifest", "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0");
                document.appendChild(rootElement);
            }
            if (!m_manifestBuffer.isEmpty() && !document.setContent(m_manifestBuffer)) {
                // Oi! That's fresh XML we should have here!
                // This is the only case we can't fix
                KMessage::message(KMessage::Error, i18n("The manifest file seems to be corrupted. It cannot be modified and the document will remain unreadable. Please try and save the document again to prevent losing your work."));
                m_pZip->close();
                return false;
            }
            QDomElement documentElement = document.documentElement();
            QDomNodeList fileElements = documentElement.elementsByTagName("manifest:file-entry");
            // Search all files in the manifest
            QStringList foundFiles;
            for (int i = 0; i < fileElements.size(); i++) {
                QDomElement fileElement = fileElements.item(i).toElement();
                QString fullpath = fileElement.toElement().attribute("manifest:full-path");
                // See if it's encrypted
                if (fullpath.isEmpty() || !m_encryptionData.contains(fullpath)) {
                    continue;
                }
                foundFiles += fullpath;
                KoEncryptedStore_EncryptionData encData = m_encryptionData.value(fullpath);
                // Set the unencrypted size of the file
                fileElement.setAttribute("manifest:size", encData.filesize);
                // See if the user of this store has already provided (old) encryption data
                QDomNodeList childElements = fileElement.elementsByTagName("manifest:encryption-data");
                QDomElement encryptionElement;
                QDomElement algorithmElement;
                QDomElement keyDerivationElement;
                if (childElements.isEmpty()) {
                    encryptionElement = document.createElement("manifest:encryption-data");
                    fileElement.appendChild(encryptionElement);
                } else {
                    encryptionElement = childElements.item(0).toElement();
                }
                childElements = encryptionElement.elementsByTagName("manifest:algorithm");
                if (childElements.isEmpty()) {
                    algorithmElement = document.createElement("manifest:algorithm");
                    encryptionElement.appendChild(algorithmElement);
                } else {
                    algorithmElement = childElements.item(0).toElement();
                }
                childElements = encryptionElement.elementsByTagName("manifest:key-derivation");
                if (childElements.isEmpty()) {
                    keyDerivationElement = document.createElement("manifest:key-derivation");
                    encryptionElement.appendChild(keyDerivationElement);
                } else {
                    keyDerivationElement = childElements.item(0).toElement();
                }
                // Set the right encryption data
                QCA::Base64 encoder;
                QCA::SecureArray checksum = encoder.encode(encData.checksum);
                if (encData.checksumShort) {
                    encryptionElement.setAttribute("manifest:checksum-type", "SHA1/1K");
                } else {
                    encryptionElement.setAttribute("manifest:checksum-type", "SHA1");
                }
                encryptionElement.setAttribute("manifest:checksum", QString(checksum.toByteArray()));
                QCA::SecureArray initVector = encoder.encode(encData.initVector);
                algorithmElement.setAttribute("manifest:algorithm-name", "Blowfish CFB");
                algorithmElement.setAttribute("manifest:initialisation-vector", QString(initVector.toByteArray()));
                QCA::SecureArray salt = encoder.encode(encData.salt);
                keyDerivationElement.setAttribute("manifest:key-derivation-name", "PBKDF2");
                keyDerivationElement.setAttribute("manifest:iteration-count", QString::number(encData.iterationCount));
                keyDerivationElement.setAttribute("manifest:salt", QString(salt.toByteArray()));
            }
            if (foundFiles.size() < m_encryptionData.size()) {
                QList<QString> keys = m_encryptionData.keys();
                for (int i = 0; i < keys.size(); i++) {
                    if (!foundFiles.contains(keys.value(i))) {
                        KoEncryptedStore_EncryptionData encData = m_encryptionData.value(keys.value(i));
                        QDomElement fileElement = document.createElement("manifest:file-entry");
                        fileElement.setAttribute("manifest:full-path", keys.value(i));
                        fileElement.setAttribute("manifest:size", encData.filesize);
                        fileElement.setAttribute("manifest:media-type", "");
                        documentElement.appendChild(fileElement);
                        QDomElement encryptionElement = document.createElement("manifest:encryption-data");
                        QCA::Base64 encoder;
                        QCA::SecureArray checksum = encoder.encode(encData.checksum);
                        QCA::SecureArray initVector = encoder.encode(encData.initVector);
                        QCA::SecureArray salt = encoder.encode(encData.salt);
                        if (encData.checksumShort) {
                            encryptionElement.setAttribute("manifest:checksum-type", "SHA1/1K");
                        } else {
                            encryptionElement.setAttribute("manifest:checksum-type", "SHA1");
                        }
                        encryptionElement.setAttribute("manifest:checksum", QString(checksum.toByteArray()));
                        fileElement.appendChild(encryptionElement);
                        QDomElement algorithmElement = document.createElement("manifest:algorithm");
                        algorithmElement.setAttribute("manifest:algorithm-name", "Blowfish CFB");
                        algorithmElement.setAttribute("manifest:initialisation-vector", QString(initVector.toByteArray()));
                        encryptionElement.appendChild(algorithmElement);
                        QDomElement keyDerivationElement = document.createElement("manifest:key-derivation");
                        keyDerivationElement.setAttribute("manifest:key-derivation-name", "PBKDF2");
                        keyDerivationElement.setAttribute("manifest:iteration-count", QString::number(encData.iterationCount));
                        keyDerivationElement.setAttribute("manifest:salt", QString(salt.toByteArray()));
                        encryptionElement.appendChild(keyDerivationElement);
                    }
                }
            }
            m_manifestBuffer = document.toByteArray();
            m_pZip->setCompression(KZip::DeflateCompression);
            if (!m_pZip->writeFile(MANIFEST_FILE, "", "", m_manifestBuffer.data(), m_manifestBuffer.size())) {
                KMessage::message(KMessage::Error, i18n("The manifest file cannot be written. The document will remain unreadable. Please try and save the document again to prevent losing your work."));
                m_pZip->close();
                return false;
            }
        }
    }
    if (m_pZip)
        return m_pZip->close();
    else
        return true;
}

KoEncryptedStore::~KoEncryptedStore()
{
    Q_D(KoStore);
    /* Finalization of an encrypted store must happen earlier than deleting the zip. This rule normally is executed by KoStore, but too late to do any good.*/
    if (!d->finalized) {
        finalize();
    }

    delete m_pZip;

    if (d->fileMode == KoStorePrivate::RemoteWrite) {
        KIO::NetAccess::upload(d->localFileName, d->url, d->window);
        delete m_tempFile;
    } else if (d->fileMode == KoStorePrivate::RemoteRead) {
        KIO::NetAccess::removeTempFile(d->localFileName);
    }

    delete d->stream;
}

bool KoEncryptedStore::isEncrypted()
{
    Q_D(KoStore);
    if (d->mode == Read) {
        return !m_encryptionData.isEmpty();
    }
    return true;
}

bool KoEncryptedStore::isToBeEncrypted(const QString& name)
{
    return !(name == META_FILE || name == MANIFEST_FILE || name == THUMBNAIL_FILE);
}

bool KoEncryptedStore::openRead(const QString& name)
{
    Q_D(KoStore);
    if (bad())
        return false;

    const KArchiveEntry* fileArchiveEntry = m_pZip->directory()->entry(name);
    if (!fileArchiveEntry) {
        return false;
    }
    if (fileArchiveEntry->isDirectory()) {
        kWarning(30002) << name << " is a directory!";
        return false;
    }
    const KZipFileEntry* fileZipEntry = static_cast<const KZipFileEntry*>(fileArchiveEntry);

    delete d->stream;
    d->stream = fileZipEntry->createDevice();
    d->size = fileZipEntry->size();
    if (m_encryptionData.contains(name)) {
        // This file is encrypted, do some decryption first
        if (m_bPasswordDeclined) {
            // The user has already declined to give a password
            // Open the file as empty
            d->stream->close();
            delete d->stream;
            d->stream = new QBuffer();
            d->stream->open(QIODevice::ReadOnly);
            d->size = 0;
            return true;
        }
        QCA::SecureArray encryptedFile(d->stream->readAll());
        if (encryptedFile.size() != d->size) {
            // Read error detected
            d->stream->close();
            delete d->stream;
            d->stream = NULL;
            kWarning(30002) << "read error";
            return false;
        }
        d->stream->close();
        delete d->stream;
        d->stream = NULL;
        KoEncryptedStore_EncryptionData encData = m_encryptionData.value(name);
        QCA::SecureArray decrypted;

        // If we don't have a password yet, try and find one
        if (m_password.isEmpty()) {
            findPasswordInKWallet();
        }

        while (true) {
            QByteArray pass;
            QCA::SecureArray password;
            bool keepPass = false;
            // I already have a password! Let's test it. If it's not good, we can dump it, anyway.
            if (!m_password.isEmpty()) {
                password = m_password;
                m_password = QCA::SecureArray();
            } else {
                if (!m_filename.isNull())
                    keepPass = false;
                KPasswordDialog dlg(d->window , keepPass ? KPasswordDialog::ShowKeepPassword : static_cast<KPasswordDialog::KPasswordDialogFlags>(0));
                dlg.setPrompt(i18n("Please enter the password to open this file."));
                if (! dlg.exec()) {
                    m_bPasswordDeclined = true;
                    d->stream = new QBuffer();
                    d->stream->open(QIODevice::ReadOnly);
                    d->size = 0;
                    return true;
                }
                password = QCA::SecureArray(dlg.password().toUtf8());
                if (keepPass)
                    keepPass = dlg.keepPassword();
                if (password.isEmpty()) {
                    continue;
                }
            }

            decrypted = decryptFile(encryptedFile, encData, password);
            if (decrypted.isEmpty()) {
                kError(30002) << "empty decrypted file" << endl;
                return false;
            }

            if (!encData.checksum.isEmpty()) {
                QCA::SecureArray checksum;
                if (encData.checksumShort && decrypted.size() > 1024) {
                    // TODO: Eww!!!! I don't want to convert via insecure arrays to get the first 1K characters of a secure array <- fix QCA?
                    checksum = QCA::Hash("sha1").hash(QCA::SecureArray(decrypted.toByteArray().left(1024)));
                } else {
                    checksum = QCA::Hash("sha1").hash(decrypted);
                }
                if (checksum != encData.checksum) {
                    continue;
                }
            }

            // The password passed all possible tests, so let's accept it
            m_password = password;
            m_bPasswordUsed = true;

            if (keepPass) {
                savePasswordInKWallet();
            }

            break;
        }

        QByteArray *resultArray = new QByteArray(decrypted.toByteArray());
        QIODevice *resultDevice = KFilterDev::device(new QBuffer(resultArray, NULL), "application/x-gzip");
        if (!resultDevice) {
            delete resultArray;
            return false;
        }
        static_cast<KFilterDev*>(resultDevice)->setSkipHeaders();
        d->stream = resultDevice;
        d->size = encData.filesize;
    }
    if (!d->stream->isOpen()) {
        d->stream->open(QIODevice::ReadOnly);
    }
    return true;
}

bool KoEncryptedStore::closeRead()
{
    Q_D(KoStore);
    delete d->stream;
    d->stream = NULL;
    return true;
}

void KoEncryptedStore::findPasswordInKWallet()
{
    Q_D(KoStore);
    /* About KWallet access
     *
     * The choice has been made to postfix every entry in a kwallet concerning passwords for opendocument files with /opendocument
     * This choice has been made since, at the time of this writing, the author could not find any reference to standardized
     * naming schemes for entries in the wallet. Since collision of passwords in entries should be avoided and is at least possible,
     * considering remote files might be both protected by a secured web-area (konqueror makes an entry) and a password (we make an
     * entry), it seems a good thing to make sure it won't happen.
     */
    if (!m_filename.isNull() && !KWallet::Wallet::folderDoesNotExist(KWallet::Wallet::LocalWallet(), KWallet::Wallet::PasswordFolder()) && !KWallet::Wallet::keyDoesNotExist(KWallet::Wallet::LocalWallet(), KWallet::Wallet::PasswordFolder(), m_filename + "/opendocument")) {
        KWallet::Wallet *wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), d->window ? d->window->winId() : 0);
        if (wallet) {
            if (wallet->setFolder(KWallet::Wallet::PasswordFolder())) {
                QString pass;
                wallet->readPassword(m_filename + "/opendocument", pass);
                m_password = QCA::SecureArray(pass.toUtf8());
            }
            delete wallet;
        }
    }
}

void KoEncryptedStore::savePasswordInKWallet()
{
    Q_D(KoStore);
    KWallet::Wallet *wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), d->window ? d->window->winId() : 0);
    if (wallet) {
        if (!wallet->hasFolder(KWallet::Wallet::PasswordFolder())) {
            wallet->createFolder(KWallet::Wallet::PasswordFolder());
        }
        if (wallet->setFolder(KWallet::Wallet::PasswordFolder())) {
            if (wallet->hasEntry(m_filename + "/opendocument")) {
                wallet->removeEntry(m_filename + "/opendocument");
            }
            wallet->writePassword(m_filename + "/opendocument", m_password.toByteArray().data());
        }
        delete wallet;
    }
}

QCA::SecureArray KoEncryptedStore::decryptFile(QCA::SecureArray & encryptedFile, KoEncryptedStore_EncryptionData & encData, QCA::SecureArray & password)
{
    QCA::SecureArray keyhash = QCA::Hash("sha1").hash(password);
    QCA::SymmetricKey key = QCA::PBKDF2("sha1").makeKey(keyhash, QCA::InitializationVector(encData.salt), 16, encData.iterationCount);
    QCA::Cipher decrypter("blowfish", QCA::Cipher::CFB, QCA::Cipher::DefaultPadding, QCA::Decode, key, QCA::InitializationVector(encData.initVector));
    QCA::SecureArray result = decrypter.update(encryptedFile);
    result += decrypter.final();
    return result;
}

bool KoEncryptedStore::setPassword(const QString& password)
{
    if (m_bPasswordUsed || password.isEmpty()) {
        return false;
    }
    m_password = QCA::SecureArray(password.toUtf8());
    return true;
}

QString KoEncryptedStore::password()
{
    if (m_password.isEmpty()) {
        return QString();
    }
    return QString(m_password.toByteArray());
}

bool KoEncryptedStore::openWrite(const QString& name)
{
    Q_D(KoStore);
    if (bad())
        return false;
    if (isToBeEncrypted(name)) {
        // Encrypted files will be compressed by this class and should be stored in the zip as not compressed
        m_pZip->setCompression(KZip::NoCompression);
    } else {
        m_pZip->setCompression(KZip::DeflateCompression);
    }
    d->stream = new QBuffer();
    (static_cast< QBuffer* >(d->stream))->open(QIODevice::WriteOnly);
    if (name == MANIFEST_FILE)
        return true;
    return m_pZip->prepareWriting(name, "", "", 0);
}

bool KoEncryptedStore::closeWrite()
{
    Q_D(KoStore);
    bool passWasAsked = false;
    if (d->fileName == MANIFEST_FILE) {
        m_manifestBuffer = static_cast<QBuffer*>(d->stream)->buffer();
        return true;
    }

    // Find a password
    // Do not accept empty passwords for compatibility with OOo
    if (m_password.isEmpty()) {
        findPasswordInKWallet();
    }
    while (m_password.isEmpty()) {
        KNewPasswordDialog dlg(d->window);
        dlg.setPrompt(i18n("Please enter the password to encrypt the document with."));
        if (! dlg.exec()) {
            // Without the first password, prevent asking again by deadsimply refusing to continue functioning
            // TODO: This feels rather hackish. There should be a better way to do this.
            delete m_pZip;
            m_pZip = 0;
            d->good = false;
            return false;
        }
        m_password = QCA::SecureArray(dlg.password().toUtf8());
        passWasAsked = true;
    }

    // Ask the user to save the password
    if (passWasAsked && KMessageBox::questionYesNo(d->window, i18n("Do you want to save the password?")) == KMessageBox::Yes) {
        savePasswordInKWallet();
    }

    QByteArray resultData;
    if (d->fileName == THUMBNAIL_FILE) {
        // TODO: Replace with a generic 'encrypted'-thumbnail
        resultData = static_cast<QBuffer*>(d->stream)->buffer();
    } else if (!isToBeEncrypted(d->fileName)) {
        resultData = static_cast<QBuffer*>(d->stream)->buffer();
    } else {
        m_bPasswordUsed = true;
        // Build all cryptographic data
        QCA::SecureArray passwordHash = QCA::Hash("sha1").hash(m_password);
        QCA::Random random;
        KoEncryptedStore_EncryptionData encData;
        encData.initVector = random.randomArray(8);
        encData.salt = random.randomArray(16);
        encData.iterationCount = 1024;
        QCA::SymmetricKey key = QCA::PBKDF2("sha1").makeKey(passwordHash, QCA::InitializationVector(encData.salt), 16, encData.iterationCount);
        QCA::Cipher encrypter("blowfish", QCA::Cipher::CFB, QCA::Cipher::DefaultPadding, QCA::Encode, key, QCA::InitializationVector(encData.initVector));

        // Get the written data
        QByteArray data = static_cast<QBuffer*>(d->stream)->buffer();
        encData.filesize = data.size();

        // Compress the data
        QBuffer compressedData;
        QIODevice *compressDevice = KFilterDev::device(&compressedData, "application/x-gzip", false);
        if (!compressDevice) {
            return false;
        }
        static_cast<KFilterDev*>(compressDevice)->setSkipHeaders();
        if (!compressDevice->open(QIODevice::WriteOnly)) {
            delete compressDevice;
            return false;
        }
        if (compressDevice->write(data) != data.size()) {
            delete compressDevice;
            return false;
        }
        compressDevice->close();
        delete compressDevice;

        encData.checksum = QCA::Hash("sha1").hash(QCA::SecureArray(compressedData.buffer()));
        encData.checksumShort = false;

        // Encrypt the data
        QCA::SecureArray result = encrypter.update(QCA::SecureArray(compressedData.buffer()));
        result += encrypter.final();
        resultData = result.toByteArray();

        m_encryptionData.insert(d->fileName, encData);
    }

    if (!m_pZip->writeData(resultData.data(), resultData.size())) {
        m_pZip->finishWriting(resultData.size());
        return false;
    }

    return m_pZip->finishWriting(resultData.size());
}

bool KoEncryptedStore::enterRelativeDirectory(const QString& dirName)
{
    Q_D(KoStore);
    if (d->mode == Read) {
        if (!m_currentDir) {
            m_currentDir = m_pZip->directory(); // initialize
        }
        const KArchiveEntry *entry = m_currentDir->entry(dirName);
        if (entry && entry->isDirectory()) {
            m_currentDir = dynamic_cast<const KArchiveDirectory*>(entry);
            return m_currentDir != 0;
        }
        return false;
    } else { // Write, no checking here
        return true;
    }
}

bool KoEncryptedStore::enterAbsoluteDirectory(const QString& path)
{
    if (path.isEmpty()) {
        m_currentDir = 0;
        return true;
    }
    m_currentDir = dynamic_cast<const KArchiveDirectory*>(m_pZip->directory()->entry(path));
    return m_currentDir != 0;
}

bool KoEncryptedStore::fileExists(const QString& absPath) const
{
    const KArchiveEntry *entry = m_pZip->directory()->entry(absPath);
    return (entry && entry->isFile()) || (absPath == MANIFEST_FILE && !m_manifestBuffer.isNull());
}

#endif // QCA2
