/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2014 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_hlfs.h"

ZEND_DECLARE_MODULE_GLOBALS(hlfs)

/* True global resources - no need for thread safety here */
static int le_hlfs;

/* {{{ hlfs_functions[]
 *
 * Every user visible function must have an entry in hlfs_functions[].
 */
const zend_function_entry hlfs_functions[] = {
	PHP_FE(confirm_hlfs_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE(hlfs_query_svr,	NULL)
	PHP_FE(hlfs_notify_res,	NULL)
	PHP_FE(hlfs_notify_num,	NULL)
	PHP_FE_END	/* Must be the last line in hlfs_functions[] */
};
/* }}} */

/* {{{ hlfs_module_entry
 */
zend_module_entry hlfs_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"hlfs",
	hlfs_functions,
	PHP_MINIT(hlfs),
	PHP_MSHUTDOWN(hlfs),
	PHP_RINIT(hlfs),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(hlfs),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(hlfs),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_HLFS_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_HLFS
ZEND_GET_MODULE(hlfs)
#endif

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    //STD_PHP_INI_ENTRY("hlfs.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_hlfs_globals, hlfs_globals)
    //STD_PHP_INI_ENTRY("hlfs.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_hlfs_globals, hlfs_globals)
	//STD_PHP_INI_ENTRY("hlfs.path", "", PHP_INI_SYSTEM, OnUpdateString, path, zend_hlfs_globals, hlfs_globals)
PHP_INI_END()

/* }}} */

/* {{{ php_hlfs_init_globals
 */
static void php_hlfs_init_globals(zend_hlfs_globals *hlfs_globals)
{
	//hlfs_globals->path = NULL;
	//hlfs_globals->slist = NULL;
}
/* }}} */

/**
 *  启动初始化
 *  附着共享内存||连接agent服务器
 */
PHP_MINIT_FUNCTION(hlfs)
{
	/*
	if(HlfsStart(&g_handle) < 0)
	{
		return FAILURE;
	}
	*/
	
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(hlfs)
{
	HlfsFinal(&g_handle);
	
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(hlfs)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(hlfs)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(hlfs)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "hlfs support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_hlfs_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_hlfs_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "hlfs", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/

/* {{{ proto int hlfs_query_svr(int gid, int xid)
   ; */
PHP_FUNCTION(hlfs_query_svr)
{
	int argc = ZEND_NUM_ARGS();
	long gid;
	long xid;

	if (zend_parse_parameters(argc TSRMLS_CC, "ll", &gid, &xid) == FAILURE) 
		return;

	php_error(E_WARNING, "hlfs_query_svr: not yet implemented");
}
/* }}} */

/* {{{ proto int hlfs_notify_res(int gid, int xid)
   char* host, int port, int reqres, int reqcount); */
PHP_FUNCTION(hlfs_notify_res)
{
	int argc = ZEND_NUM_ARGS();
	long gid;
	long xid;
	char *host;
	int host_len = 0;
	long ret = 0;
	long retcount = 1;
	
	if (zend_parse_parameters(argc TSRMLS_CC, "llsll", &gid, &xid,&host,&host_len,&ret,&retcount) == FAILURE) 
		return;
	
	NotifyCallerRes(gid,xid,host,ret,retcount);
	
	//php_error(E_WARNING, "hlfs_notify_res: not yet implemented");
}
/* }}} */

/* {{{ proto int hlfs_notify_num(int gid, int xid)
   char* host, int port, int reqcount); */
PHP_FUNCTION(hlfs_notify_num)
{
	int argc = ZEND_NUM_ARGS();
	long gid;
	long xid;

	if (zend_parse_parameters(argc TSRMLS_CC, "ll", &gid, &xid) == FAILURE) 
		return;

	php_error(E_WARNING, "hlfs_notify_num: not yet implemented");
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
