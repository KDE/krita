/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisAndroidDonations.h"

#include "KisApplication.h"
#include "kis_splash_screen.h"

#include <QAndroidJniEnvironment>
#include <QImage>
#include <QTemporaryFile>
#include <QtAndroid>

// See DonationHelper.java for notes!

namespace
{
void cleanupProductIconData(void *info)
{
    uint32_t *data = static_cast<uint32_t *>(info);
    delete[] data;
}

QPixmap getProductIcon(const QAndroidJniObject &javaProduct)
{
    jint width = 256;
    jint height = 256;

    QAndroidJniObject pixelsArrayObject = javaProduct.callObjectMethod("getIconPixels", "(II)[I", width, height);
    if (!pixelsArrayObject.isValid()) {
        return QPixmap();
    }

    jintArray pixelsArray = static_cast<jintArray>(pixelsArrayObject.object());
    if (!pixelsArray) {
        return QPixmap();
    }

    QAndroidJniEnvironment env;
    jsize length = env->GetArrayLength(pixelsArray);
    if (length != jsize(width) * jsize(height)) {
        return QPixmap();
    }

    jint *pixels = env->GetIntArrayElements(pixelsArray, nullptr);
    if (!pixels) {
        return QPixmap();
    }

    uint32_t *data = new uint32_t[length];
    for (jsize i = 0; i < length; ++i) {
        data[i] = uint32_t(pixels[i]);
    }

    env->ReleaseIntArrayElements(pixelsArray, pixels, 0);

    QPixmap pixmap;
    if (!pixmap.convertFromImage(QImage(reinterpret_cast<uchar *>(data),
                                        int(width),
                                        int(height),
                                        qsizetype(width) * qsizetype(4),
                                        QImage::Format_ARGB32,
                                        cleanupProductIconData,
                                        data))) {
        return QPixmap();
    }

    return pixmap;
}
} // namespace

KisAndroidDonations *KisAndroidDonations::instance()
{
    KisApplication *app = qobject_cast<KisApplication *>(KisApplication::instance());
    if (app) {
        return app->androidDonations();
    } else {
        return nullptr;
    }
}

void KisAndroidDonations::setLoaded(bool loaded)
{
    QAndroidJniObject::callStaticMethod<void>("org/krita/android/MainActivity", "setLoaded", "(Z)V", jboolean(loaded));
}

void KisAndroidDonations::setLoadingText(const QString &text)
{
    QAndroidJniObject textObject = QAndroidJniObject::fromString(text);
    QAndroidJniObject::callStaticMethod<void>("org/krita/android/MainActivity",
                                              "setLoadingText",
                                              "(Ljava/lang/String;)V",
                                              textObject.object<jstring>());
}

void KisAndroidDonations::showDonationDialog(bool splash)
{
    QAndroidJniEnvironment env;
    jbyteArray splashBytes = nullptr;
    QString splashArtist;
    if (splash) {
        KisSplashScreen::Source source = KisSplashScreen::getImageSource();
        splashArtist = source.artistCredit;
        QFile f(source.resourcePath);
        if (f.open(QIODevice::ReadOnly)) {
            QByteArray buf = f.readAll();
            f.close();

            jsize length = buf.size();
            if (length > 0) {
                splashBytes = env->NewByteArray(length);
                env->SetByteArrayRegion(splashBytes, 0, length, reinterpret_cast<const jbyte *>(buf.constData()));
            }
        } else {
            qWarning("Failed to read splash screen image from '%s': %s",
                     qUtf8Printable(source.resourcePath),
                     qUtf8Printable(f.errorString()));
        }
    }
    QAndroidJniObject splashArtistObject = QAndroidJniObject::fromString(splashArtist);
    QAndroidJniObject splashVersionObject = QAndroidJniObject::fromString(qApp->applicationVersion());
    QAndroidJniObject::callStaticMethod<void>("org/krita/android/MainActivity",
                                              "showDonationDialog",
                                              "(Z[BLjava/lang/String;Ljava/lang/String;)V",
                                              jboolean(splash),
                                              splashBytes,
                                              splashArtistObject.object(),
                                              splashVersionObject.object());
    if (splashBytes) {
        env->DeleteLocalRef(splashBytes);
    }
}

bool KisAndroidDonations::shouldShowSupporterBadge() const
{
    return m_state == State::Supporter && isProductOwned(QStringLiteral("thankyoukiki"));
}

void KisAndroidDonations::setShowDonationManagementDialogPending(bool pending)
{
    if (pending && !m_showDonationManagementDialogPending) {
        m_showDonationManagementDialogPending = true;
        Q_EMIT sigShowDonationManagementDialogRequested();
    } else if (!pending && m_showDonationManagementDialogPending) {
        m_showDonationManagementDialogPending = false;
    }
}

QVector<KisSupporterProduct> KisAndroidDonations::getCurrentProducts() const
{
    QVector<KisSupporterProduct> products;
    QAndroidJniObject currentProducts = QAndroidJniObject::callStaticObjectMethod("org/krita/android/DonationHelper",
                                                                                  "getCurrentProducts",
                                                                                  "()Ljava/util/List;");
    jint size = currentProducts.callMethod<jint>("size", "()I");
    products.reserve(size);
    for (jint i = 0; i < size; ++i) {
        QAndroidJniObject javaProduct = currentProducts.callObjectMethod("get", "(I)Ljava/lang/Object;", i);

        products.append(KisSupporterProduct{
            KisSupporterProductType(javaProduct.callMethod<jint>("getType", "()I")),
            KisSupporterProductAvailability(javaProduct.callMethod<jint>("getAvailability", "()I")),
            javaProduct.callObjectMethod("getId", "()Ljava/lang/String;").toString(),
            javaProduct.callObjectMethod("getTitle", "()Ljava/lang/String;").toString(),
            javaProduct.callObjectMethod("getDescription", "()Ljava/lang/String;").toString(),
            getProductIcon(javaProduct),
            javaProduct.callObjectMethod("getButtonText", "()Ljava/lang/String;").toString(),
            javaProduct.callObjectMethod("getOfferToken", "()Ljava/lang/String;").toString(),
        });
    }
    return products;
}

void KisAndroidDonations::startBillingFlowFor(const QString &productId, const QString &offerToken)
{
    QAndroidJniObject productIdObject = QAndroidJniObject::fromString(productId);
    QAndroidJniObject offerTokenObject = QAndroidJniObject::fromString(offerToken);
    QAndroidJniObject::callStaticMethod<void>("org/krita/android/DonationHelper",
                                              "startBillingFlowForProductId",
                                              "(Ljava/lang/String;Ljava/lang/String;)V",
                                              productIdObject.object<jstring>(),
                                              offerTokenObject.object<jstring>());
}

void KisAndroidDonations::slotStartDonationFlow()
{
    showDonationDialog(false);
}

void KisAndroidDonations::slotManageSubscriptions()
{
    QAndroidJniObject::callStaticMethod<void>("org/krita/android/MainActivity", "manageSubscriptions", "()V");
}

void KisAndroidDonations::slotManageSubscription(const QString &productId)
{
    QAndroidJniObject productIdObject = QAndroidJniObject::fromString(productId);
    QAndroidJniObject::callStaticMethod<void>("org/krita/android/MainActivity",
                                              "manageSubscription",
                                              "(Ljava/lang/String;)V",
                                              productIdObject.object<jstring>());
}

void KisAndroidDonations::slotUpdateState(int state, long long ownedProductFlags)
{
    if (int(m_state) != state || m_ownedProductFlags != ownedProductFlags) {
        m_state = State(state);
        m_ownedProductFlags = ownedProductFlags;
        Q_EMIT sigStateChanged();
    }
}

KisAndroidDonations::KisAndroidDonations(QObject *parent)
    : QObject(parent)
{
    // The state update can come from a different thread, so we'll use a queued connection.
    connect(this,
            &KisAndroidDonations::sigStateUpdateReceived,
            this,
            &KisAndroidDonations::slotUpdateState,
            Qt::QueuedConnection);
}

bool KisAndroidDonations::isProductOwned(const QString &productId) const
{
    QAndroidJniObject productIdObject = QAndroidJniObject::fromString(productId);
    return QAndroidJniObject::callStaticMethod<jboolean>("org/krita/android/DonationHelper",
                                                         "isProductOwned",
                                                         "(Ljava/lang/String;)B");
}

void KisAndroidDonations::syncState()
{
    QAndroidJniObject::callStaticMethod<void>("org/krita/android/DonationHelper", "syncState", "()V");
}

extern "C" JNIEXPORT void JNICALL Java_org_krita_android_JNIWrappers_donationStateUpdated(JNIEnv * /*env*/,
                                                                                          jobject /*obj*/,
                                                                                          jint state,
                                                                                          jlong ownedProductFlags)
{
    KisApplication *app = qobject_cast<KisApplication *>(KisApplication::instance());
    if (app) {
        KisAndroidDonations *androidDonations = app->androidDonations();
        if (androidDonations) {
            Q_EMIT androidDonations->sigStateUpdateReceived(state, ownedProductFlags);
        }
    }
}

extern "C" JNIEXPORT void JNICALL Java_org_krita_android_JNIWrappers_showDonationManagementDialog(JNIEnv * /*env*/,
                                                                                                  jobject /*obj*/)
{
    KisApplication *app = qobject_cast<KisApplication *>(KisApplication::instance());
    if (app) {
        KisAndroidDonations *androidDonations = app->androidDonations();
        if (androidDonations) {
            androidDonations->setShowDonationManagementDialogPending(true);
        }
    }
}

extern "C" JNIEXPORT void JNICALL Java_org_krita_android_JNIWrappers_onSplashDialogDismissed(JNIEnv * /*env*/,
                                                                                             jobject /*obj*/)
{
    KisApplication *app = qobject_cast<KisApplication *>(KisApplication::instance());
    if (app) {
        KisAndroidDonations *androidDonations = app->androidDonations();
        if (androidDonations) {
            Q_EMIT androidDonations->sigSplashDialogDismissed();
        }
    }
}
