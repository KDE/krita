In xcftools.h, the following changes need to be done for MSVC:

@@ -37,6 +37,11 @@
 #if HAVE_INTTYPES_H
 # define __STDC_FORMAT_MACROS
 # include <inttypes.h>
+#elif defined(_MSC_VER)
+# include <stdint.h> // KDEWin
+# define PRIX32 "I32X"
+# define PRIu32 "I32u"
+# define PRIXPTR "IX"

