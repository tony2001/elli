dnl $Id$

PHP_ARG_WITH(elli, for elli support,
[  --with-elli=<DIR>             Include elli support])

if test "$PHP_ELLI" != "no"; then
  SEARCH_PATH="/usr/local /usr"
  SEARCH_FOR="/include/elli.h"
  if test -r $PHP_ELLI/$SEARCH_FOR; then # path given as parameter
    ELLI_DIR=$PHP_ELLI
  else # search default path list
    AC_MSG_CHECKING([for elli files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        ELLI_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$ELLI_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please specify prefix to installed libelli])
  fi

  PHP_ADD_INCLUDE($ELLI_DIR/include)

  LIBNAME=elli
  LIBSYMBOL=elli_ctx_create

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $ELLI_DIR/$PHP_LIBDIR, ELLI_SHARED_LIBADD)
    AC_DEFINE(HAVE_ELLILIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong libelli version or lib not found])
  ],[
    -L$ELLI_DIR/$PHP_LIBDIR -lm
  ])

  PHP_SUBST(ELLI_SHARED_LIBADD)
  AC_DEFINE(HAVE_ELLI, 1, [ Have elli support ])
  PHP_NEW_EXTENSION(elli, elli.c, $ext_shared)
fi
