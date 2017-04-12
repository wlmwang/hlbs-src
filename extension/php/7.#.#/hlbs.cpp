/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2017 The PHP Group                                |
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

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>
#ifndef INT64_MAX
#define INT64_MAX	INT64_C( 9223372036854775807)
#endif
#ifndef INT64_MIN
#define INT64_MIN	(-INT64_C( 9223372036854775807)-1)
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_hlbs.h"

#ifdef __cplusplus
}
#include "agent_api.h"
#endif

/* If you declare any globals in php_hlbs.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(hlbs)
*/

/* True global resources - no need for thread safety here */
static int le_hlbs;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("hlbs.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_hlbs_globals, hlbs_globals)
    STD_PHP_INI_ENTRY("hlbs.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_hlbs_globals, hlbs_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_hlbs_compiled(string arg)
   Return a string to confirm that the module is compiled in */
/*
PHP_FUNCTION(confirm_hlbs_compiled)
{
	char *arg = NULL;
	size_t arg_len, len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "hlbs", arg);

	RETURN_STR(strg);
}
*/
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_hlbs_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_hlbs_init_globals(zend_hlbs_globals *hlbs_globals)
{
	hlbs_globals->global_value = 0;
	hlbs_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(hlbs)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(hlbs)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(hlbs)
{
#if defined(COMPILE_DL_HLBS) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(hlbs)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(hlbs) {
	php_info_print_table_start();
	php_info_print_table_header(2, "hlbs support", "enabled");
	php_info_print_table_row(2, "version",     PHP_HLBS_VERSION);
	php_info_print_table_row(2, "anthor",      "Anny Wang, Hupu Inc.");
	php_info_print_table_row(2, "copyright",   "Copyright (c) 2016 Hupu Inc. All Rights Reserved.");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ hlbs_functions[]
 *
 * Every user visible function must have an entry in hlbs_functions[].
 */
const zend_function_entry hlbs_functions[] = {
	//PHP_FE(confirm_hlbs_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE(hlbs_query_svr,	NULL)
	PHP_FE(hlbs_notify_res,	NULL)
	PHP_FE_END	/* Must be the last line in hlbs_functions[] */
};
/* }}} */

/* {{{ hlbs_module_entry
 */
zend_module_entry hlbs_module_entry = {
	STANDARD_MODULE_HEADER,
	"hlbs",
	hlbs_functions,
	PHP_MINIT(hlbs),
	PHP_MSHUTDOWN(hlbs),
	PHP_RINIT(hlbs),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(hlbs),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(hlbs),
	PHP_HLBS_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef __cplusplus
BEGIN_EXTERN_C()
#endif
#ifdef COMPILE_DL_HLBS
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(hlbs)
#endif
#ifdef __cplusplus
END_EXTERN_C()
#endif

PHP_FUNCTION(hlbs_query_svr) {
    zval *svrParam = NULL;  //路由zval
    double timeout = 0;     //超时
    if (ZEND_NUM_ARGS() == 2) {
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ad", &svrParam, &timeout) == FAILURE) {
            RETURN_LONG(-1);
        }
    } else {
        WRONG_PARAM_COUNT;
        RETURN_LONG(-1);
    }
    HashTable* htSvrReq = Z_ARRVAL_P(svrParam);

    zend_string* str = zend_string_init("gid", strlen("gid"), 0);
    zval *zvalGid = zend_hash_find(htSvrReq, str);  //gid zval
    if (zvalGid == NULL) {
    	zend_string_release(str);
        RETURN_LONG(-1);
	}
	zend_string_release(str);
	
	str = zend_string_init("xid", strlen("xid"), 0);
	zval *zvalXid = zend_hash_find(htSvrReq, str);  //xid zval
    if (zvalXid == NULL) {
    	zend_string_release(str);
		RETURN_LONG(-1);
    } else {
    	zend_string_release(str);

		std::string err;
		struct SvrNet_t svr;
		svr.mGid = Z_LVAL_P(zvalGid);
		svr.mXid = Z_LVAL_P(zvalXid);
		int ret = QueryNode(svr, timeout, err);
		if (ret >= 0) {
			add_assoc_stringl(svrParam, "host", svr.mHost, strlen(svr.mHost));
			add_assoc_long(svrParam, "port", svr.mPort);
		}
		RETURN_LONG(ret);
	}
}

PHP_FUNCTION(hlbs_notify_res) {
    zval *svrParam   = NULL;    //路由zval
    long interfRet   = 0;       //结果
    long usetimeUsec = 0;       //延时
    if (ZEND_NUM_ARGS() == 3) {
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "all", &svrParam, &interfRet, &usetimeUsec) == FAILURE) {
            RETURN_LONG(-1);
        }
    } else {
        WRONG_PARAM_COUNT;
        RETURN_LONG(-1);
    }
    HashTable* htSvrReq = Z_ARRVAL_P(svrParam);

    zend_string* str = zend_string_init("gid", strlen("gid"), 0);
    zval *zvalGid = zend_hash_find(htSvrReq, str);  //gid zval
    if (zvalGid == NULL) {
    	zend_string_release(str);
        RETURN_LONG(-1);
	}
	zend_string_release(str);

	str = zend_string_init("host", strlen("host"), 0);
    zval *zvalHost = zend_hash_find(htSvrReq, str);  //host zval
    if (zvalHost == NULL) {
    	zend_string_release(str);
        RETURN_LONG(-1);
	}
	zend_string_release(str);
    if (Z_TYPE_P(zvalHost) != IS_STRING) {
        RETURN_LONG(-1);
    }

	str = zend_string_init("port", strlen("port"), 0);
    zval *zvalPort = zend_hash_find(htSvrReq, str);  //port zval
    if (zvalPort == NULL) {
    	zend_string_release(str);
        RETURN_LONG(-1);
	}
	zend_string_release(str);

    str = zend_string_init("xid", strlen("xid"), 0);
    zval *zvalXid = zend_hash_find(htSvrReq, str);  //xid zval
    if (zvalXid == NULL) {
    	zend_string_release(str);
        RETURN_LONG(-1);
    } else {
    	zend_string_release(str);

        std::string err;
        struct SvrNet_t svr;
        svr.mGid = Z_LVAL_P(zvalGid);
        svr.mXid = Z_LVAL_P(zvalXid);
        svr.mPort = Z_LVAL_P(zvalPort);

        zend_string* host = Z_STR_P(zvalHost);
        memset(svr.mHost, 0, sizeof(svr.mHost));
        memcpy(svr.mHost, ZSTR_VAL(host), ZSTR_LEN(host));
        RETURN_LONG(NotifyCallerRes(svr, interfRet, usetimeUsec, err));
    }
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
