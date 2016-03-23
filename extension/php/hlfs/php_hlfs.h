#ifndef PHP_HLFS_H
#define PHP_HLFS_H

extern zend_module_entry hlfs_module_entry;
#define phpext_hlfs_ptr &hlfs_module_entry

#define PHP_HLFS_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_HLFS_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_HLFS_API __attribute__ ((visibility("default")))
#else
#	define PHP_HLFS_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

//modify
/*
PHP_MINIT_FUNCTION(hlfs);
PHP_MSHUTDOWN_FUNCTION(hlfs);
PHP_RINIT_FUNCTION(hlfs);
PHP_RSHUTDOWN_FUNCTION(hlfs);
*/

PHP_MINFO_FUNCTION(hlfs);

//modify
PHP_FUNCTION(hlfs_query_svr);
PHP_FUNCTION(hlfs_notify_res);
PHP_FUNCTION(hlfs_notify_num);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     
*/
/*
ZEND_BEGIN_MODULE_GLOBALS(hlfs)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(hlfs)
*/

/* In every utility function you add that needs to use variables 
   in php_hlfs_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as HLFS_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#endif