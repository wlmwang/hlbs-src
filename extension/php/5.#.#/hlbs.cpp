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

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
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

/* {{{ hlbs_functions[]
 *                                                                                                                                                                                                                                           
 * Every user visible function must have an entry in hlbs_functions[].
 */
const zend_function_entry hlbs_functions[] = {
	//PHP_FE(confirm_hlbs_compiled,   NULL)       /* For testing, remove later. */ 
	PHP_FE(hlbs_query_svr,	NULL)
	PHP_FE(hlbs_notify_res,	NULL)
	PHP_FE_END
};
/* }}} */

/* {{{ hlbs_module_entry
 */
zend_module_entry hlbs_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"hlbs",
	hlbs_functions,
	PHP_MINIT(hlbs),
	PHP_MSHUTDOWN(hlbs),
	PHP_RINIT(hlbs),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(hlbs),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(hlbs),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_HLBS_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef __cplusplus
BEGIN_EXTERN_C()
#endif
#ifdef COMPILE_DL_HLBS
ZEND_GET_MODULE(hlbs)
#endif
#ifdef __cplusplus
END_EXTERN_C()
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini                                                                                                                                                                           
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("hlbs.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_hlbs_globals, hlbs_globals)
    STD_PHP_INI_ENTRY("hlbs.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_hlbs_globals, hlbs_globals)
PHP_INI_END()
*/
/* }}} */

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
    int arg_len, len;
    char *strg;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
        return;
    }

    len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "hlbs", arg);
    RETURN_STRINGL(strg, len, 0);
}
*/
/* }}} */

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
    
    zval **zvalGid = NULL;  //gid zval
    if (zend_hash_find(htSvrReq, "gid", sizeof("gid"), (void **)&zvalGid) == FAILURE) {
        RETURN_LONG(-1);
    }
	
    zval **zvalXid = NULL;  //xid zval
    if (zend_hash_find(htSvrReq, "xid", sizeof("xid"), (void **)&zvalXid) == FAILURE) {
        RETURN_LONG(-1);
    } else {
        std::string err;
        struct SvrNet_t svr;
        svr.mGid = Z_LVAL_PP(zvalGid);
        svr.mXid = Z_LVAL_PP(zvalXid);
        int ret = QueryNode(svr, timeout, err);
        if (ret >= 0) {
            add_assoc_stringl(svrParam, "host", svr.mHost, strlen(svr.mHost), 1);
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
	
    zval **zvalGid = NULL; //gid zval
    if (zend_hash_find(htSvrReq, "gid", sizeof("gid"), (void **)&zvalGid) == FAILURE) {
        RETURN_LONG(-1);
    }
    convert_to_long(*zvalGid);
	
    zval **zvalHost = NULL; //host zval
    if (zend_hash_find(htSvrReq, "host", sizeof("host"), (void **)&zvalHost) == FAILURE) {
        RETURN_LONG(-1);
    }
    if (Z_TYPE_PP(zvalHost) != IS_STRING) {
        RETURN_LONG(-1);
    }

    zval **zvalPort = NULL; //port zval
    if (zend_hash_find(htSvrReq, "port", sizeof("port"), (void **)&zvalPort)  == FAILURE) {
        RETURN_LONG(-1);
    }
    convert_to_long(*zvalPort);
	
    zval **zvalXid = NULL;  //xid zval
    if (zend_hash_find(htSvrReq, "xid", sizeof("xid"), (void **) &zvalXid) == FAILURE) {
        RETURN_LONG(-1);
    } else {
        convert_to_long(*zvalXid);

        std::string err;
        struct SvrNet_t svr;
        svr.mGid = Z_LVAL_PP(zvalGid);
        svr.mXid = Z_LVAL_PP(zvalXid);
        svr.mPort = Z_LVAL_PP(zvalPort);

        memset(svr.mHost, 0, sizeof(svr.mHost));
        memcpy(svr.mHost, Z_STRVAL_PP(zvalHost), Z_STRLEN_PP(zvalHost));
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
