//KoHyphenator test, Lukas Tinkl <lukas.tinkl@suse.cz>

#include <stdio.h>
#include <stdlib.h>

#include <QStringList>

#include <kapplication.h>

#include "kohyphen.h"
#include <kdebug.h>

static bool check(QString a, QString b)
{
  if (a.isEmpty())
     a = QString::null;
  if (b.isEmpty())
     b = QString::null;
  if (a == b) {
    kDebug() << "checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
  }
  else {
    kDebug() << "checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}

KoHyphenator * hypher = 0L;

void check_hyphenation( const QStringList& tests, const QStringList& results, const char* lang )
{
    QStringList::ConstIterator it, itres;
    for ( it = tests.begin(), itres = results.begin(); it != tests.end() ; ++it, ++itres ) {
        QString result = hypher->hyphenate((*it), lang);
        kDebug() << (*it) << " hyphenates like this: " << result << endl;
        check( result.replace(QChar(0xad),'-'), *itres );
    }
}

int main (int argc, char ** argv)
{
    QApplication app(argc, argv);

    try {
        hypher = KoHyphenator::self();
    }
    catch (KoHyphenatorException &e)
    {
        kDebug() << e.message() << endl;
        return 1;
    }

    QStringList::ConstIterator it, itres;

    //testing Czech language, this text is in UTF-8!
    QStringList cs_tests = QStringList() << "Žluťoučký" << "kůň" << "úpěl" <<
                        "ďábelské" << "ódy";

    for ( it = cs_tests.begin(); it != cs_tests.end() ; ++it )
        kDebug() << (*it) << " hyphenates like this: " << hypher->hyphenate((*it), "cs") << endl;

    //testing English
    QStringList en_tests = QStringList() << "Follow" << "white" << "rabbit";
    QStringList en_results = QStringList() << "Fol-low" << "white" << "rab-bit";
    check_hyphenation( en_tests, en_results, "en" );

    QStringList fr_tests = QStringList() << "constitution" ;
    QStringList fr_results = QStringList() << "consti-tu-tion" ;
    check_hyphenation( fr_tests, fr_results, "fr" );

    return 0;
}
