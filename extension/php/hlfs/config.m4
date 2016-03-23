dnl $Id$
dnl config.m4 for extension hlfs

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(hlfs, for hlfs support,
dnl Make sure that the comment is aligned:
dnl [  --with-hlfs             Include hlfs support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(hlfs, whether to enable hlfs support,
Make sure that the comment is aligned:
[  --enable-hlfs           Enable hlfs support])

if test "$PHP_HLFS" != "no"; then
  dnl Write more examples of tests here...
  
  dnl # --with-hlfs -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/hlfs.h"  # you most likely want to change this
  dnl if test -r $PHP_HLFS/$SEARCH_FOR; then # path given as parameter
  dnl   HLFS_DIR=$PHP_HLFS
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for hlfs files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       HLFS_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$HLFS_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the hlfs distribution])
  dnl fi

  dnl # --with-hlfs -> add include path
  dnl PHP_ADD_INCLUDE($HLFS_DIR/include)

  dnl # --with-hlfs -> check for lib and symbol presence
  dnl LIBNAME=hlfs # you may want to change this
  dnl LIBSYMBOL=hlfs # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $HLFS_DIR/lib, HLFS_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_HLFSLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong hlfs lib version or lib not found])
  dnl ],[
  dnl   -L$HLFS_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(HLFS_SHARED_LIBADD)

  PHP_REQUIRE_CXX()
  PHP_SUBST(HLFS_SHARED_LIBADD)
  PHP_ADD_LIBRARY(stdc++, 1, HLFS_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(agent_api, /home/hlfs/disvr/extension/c++, HLFS_SHARED_LIBADD) 
  PHP_NEW_EXTENSION(hlfs, hlfs.cpp, $ext_shared)

  PHP_ADD_INCLUDE(/home/hlfs/disvr/extension/c++/base)
  PHP_ADD_INCLUDE(/home/hlfs/disvr/extension/c++)
  PHP_ADD_INCLUDE(/usr/local/lib)
  
fi
