#include <KoPoint.h>
#include <kdebug.h>
#include <kglobal.h>
#include <stdlib.h>
#include <math.h>

bool check( QString txt, bool res, bool expected )
{
    if ( res == expected ) {
        kDebug() << txt << " : checking '" << res << "' against expected value '" << expected << "'... " << "ok" << endl;
    } else {
        kDebug() << txt << " : checking '" << res << "' against expected value '" << expected << "'... " << "KO !" << endl;
        exit( 1 );
    }
    return true;
}

bool check( QString txt, double res, double expected )
{
    if ( qAbs(res - expected) < 0.000001 ) {
        kDebug() << txt << " : checking '" << res << "' against expected value '" << expected << "'... " << "ok" << endl;
    } else {
        kDebug() << txt << " : checking '" << res << "' against expected value '" << expected << "'... " << "KO !" << endl;
        exit( 1 );
    }
    return true;
}

int main()
{
    KoPoint p0;
    check( "KoPoint() is null", p0.isNull(), true );
    KoPoint p1( 10.1, 20.2 );
    check( "KoPoint(...) is not null", p1.isNull(), false );
    check( "KoPoint::x()", p1.x(), 10.1 );
    check( "KoPoint::y()", p1.y(), 20.2 );
    p1.setX( 2.1 );
    p1.setY( 3.2 );
    check( "KoPoint::setX()", p1.x(), 2.1 );
    check( "KoPoint::setY()", p1.y(), 3.2 );

    KoPoint p2( QPoint( -30, -40 ) );
    check( "KoPoint(const QPoint&)", p2.x(), -30.0 );
    check( "KoPoint(const QPoint&)", p2.y(), -40.0 );
    p2.rx() = 1.5;
    p2.ry() = 2.5;
    check( "KoPoint::rx()", p2.x(), 1.5 );
    check( "KoPoint::ry()", p2.y(), 2.5 );
    p2.setCoords( -1.6, -1.7 );
    check( "KoPoint::setCoords(double, double)", p2.x(), -1.6 );
    check( "KoPoint::setCoords(double, double)", p2.y(), -1.7 );

    check( "KoPoint::operator==(const KoPoint&)", p1 == p1, true );
    check( "KoPoint::operator!=(const KoPoint&)", p1 != p2, true );

    KoPoint p4( 10.2, 20.2 );
    KoPoint p5( 30.4, 40.4 );
    p4 += p5;
    check( "KoPoint::operator+=(const KoPoint&)", p4.x(), 40.6 );
    check( "KoPoint::operator+=(const KoPoint&)", p4.y(), 60.6 );
    p4 -= p5;
    check( "KoPoint::operator-=(const KoPoint&)", p4.x(), 10.2 );
    check( "KoPoint::operator-=(const KoPoint&)", p4.y(), 20.2 );
    p4 *= 2.0;
    check( "KoPoint::operator*=(double)", p4.x(), 20.4 );
    check( "KoPoint::operator*=(double)", p4.y(), 40.4 );
    p4 = p5;
    check( "KoPoint::operator=(const KoPoint&)", p4.x(), 30.4 );
    check( "KoPoint::operator=(const KoPoint&)", p4.y(), 40.4 );

    // ### global operator+, operator- and operator*
    // ### transform

    KoPoint p6( 1.0, 2.0 );
    check( "KoPoint::isNear()", p6.isNear( p6, 0 ), true );
    check( "KoPoint::isNear()", p6.isNear( KoPoint( 2, 10 ), 1 ), false );
    check( "KoPoint::isNear()", p6.isNear( KoPoint( 2, 1 ), 1 ), true );

    KoPoint p7( 10, 10 );
    check( "KoPoint::getAngle()",
           fmod( KoPoint::getAngle( p7, p7 * -1 ) + 10*360.0, 360.0 ), 45.0 );
    // ### ???   check( "KoPoint::getAngle()",
    //         fmod( KoPoint::getAngle( -1 * p7, p7 ) + 10*360.0, 360.0 ), 45.0 );
    // can we define a behaviour ?
    // ### check( "KoPoint::getAngle()", KoPoint::getAngle( p7, p7 ), ??? );

    kDebug() << endl << "Test OK !" << endl;
}

