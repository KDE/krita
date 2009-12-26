#ifdef HAVE_ICONV_H
#include <iconv.h>
#endif
#ifdef HAVE_SYS_ICONV_H
#include <sys/iconv.h>
#endif
#include <stdlib.h>

int check( const char* from, const char* to )
{
    iconv_t myConverter = iconv_open( to, from );

    if ( myConverter != (iconv_t)-1 ) {
	iconv_close( myConverter );
	return 0;
    }
    else
	return 1;
}

int main(int argc, char** argv)
{
    const char* from[] = { "CP874", "CP932", "CP936", "CP949",
			   "CP950", "CP1250", "CP1251", "CP1252",
			   "CP1253", "CP1254", "CP1255", "CP1256",
			   "CP1257", "koi8-r", 0 };
    const char* to[] = { "UNICODELITTLE", "UNICODEBIG", 0 };
    int fromIndex = 0;
    int toIndex = 0;

    while ( to[ toIndex ] != 0 ) {
	while( from[ fromIndex ] != 0 ) {
	    if ( check( from[ fromIndex ], to[ toIndex ] ) != 0 )
		exit( 1 );
	    fromIndex = fromIndex + 1;
	}
	toIndex = toIndex + 1;
	fromIndex = 0;
    }
    exit( 0 );
}
