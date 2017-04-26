dnl $Id$
dnl config.m4 for extension hlbs

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(hlbs, for hlbs support,
Make sure that the comment is aligned:
[  --with-hlbs             Include hlbs support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(hlbs, whether to enable hlbs support,
dnl Make sure that the comment is aligned:
dnl [  --enable-hlbs           Enable hlbs support])

if test "$PHP_HLBS" != "no"; then
  dnl Write more examples of tests here...
  
  dnl # --with-hlbs -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/hlbs.h"  # you most likely want to change this
  dnl if test -r $PHP_HLBS/$SEARCH_FOR; then # path given as parameter
  dnl   HLBS_DIR=$PHP_HLBS
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for hlbs files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       HLBS_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$HLBS_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the hlbs distribution])
  dnl fi

  dnl # --with-hlbs -> add include path
  dnl PHP_ADD_INCLUDE($HLBS_DIR/include)

  dnl # --with-hlbs -> check for lib and symbol presence
  dnl LIBNAME=hlbs # you may want to change this
  dnl LIBSYMBOL=hlbs # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $HLBS_DIR/lib, HLBS_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_HLBSLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong hlbs lib version or lib not found])
  dnl ],[
  dnl   -L$HLBS_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(HLBS_SHARED_LIBADD)

  CXXFLAGS="-std=c++0x"
  PHP_REQUIRE_CXX()
  PHP_ADD_INCLUDE(./lib)
  PHP_ADD_INCLUDE(/usr/local/include/hnet)
  PHP_ADD_LIBRARY(stdc++, 1, HLBS_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(hnet, /usr/local/lib, HLBS_SHARED_LIBADD)
  PHP_SUBST(HLBS_SHARED_LIBADD)

  PHP_NEW_EXTENSION(hlbs, hlbs.cpp ./lib/agent_api.cpp, $ext_shared)
fi
