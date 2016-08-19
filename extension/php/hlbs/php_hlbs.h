#ifndef PHP_HLBS_H
#define PHP_HLBS_H

extern zend_module_entry hlbs_module_entry;
#define phpext_hlbs_ptr &hlbs_module_entry

#define PHP_HLBS_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_HLBS_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_HLBS_API __attribute__ ((visibility("default")))
#else
#	define PHP_HLBS_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

//modify
/*
PHP_MINIT_FUNCTION(hlbs);
PHP_MSHUTDOWN_FUNCTION(hlbs);
PHP_RINIT_FUNCTION(hlbs);
PHP_RSHUTDOWN_FUNCTION(hlbs);
*/

PHP_MINFO_FUNCTION(hlbs);

//modify
PHP_FUNCTION(hlbs_query_svr);
PHP_FUNCTION(hlbs_notify_res);
PHP_FUNCTION(hlbs_notify_num);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     
*/
/*
ZEND_BEGIN_MODULE_GLOBALS(hlbs)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(hlbs)
*/

/* In every utility function you add that needs to use variables 
   in php_hlbs_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as HLBS_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#endif