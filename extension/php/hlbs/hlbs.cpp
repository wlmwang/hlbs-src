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

//modify
#include "agent_api.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//modify
#ifdef __cplusplus
extern "C" {
#endif
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#ifdef __cplusplus
}
#endif

#include "php_hlbs.h"

//ZEND_DECLARE_MODULE_GLOBALS(hlbs)

/* True global resources - no need for thread safety here */
//static int le_hlbs;

//modify
const zend_function_entry hlbs_functions[] = {
	PHP_FE(hlbs_query_svr,	NULL)
	PHP_FE(hlbs_notify_res,	NULL)
	PHP_FE(hlbs_notify_num,	NULL)
	PHP_FE_END
};

//modify
zend_module_entry hlbs_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"hlbs",
	hlbs_functions,
	NULL,
	NULL,
	NULL,
	NULL,
	PHP_MINFO(hlbs),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_HLBS_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};

//modify
#ifdef __cplusplus
BEGIN_EXTERN_C()
#endif
ZEND_GET_MODULE(hlbs)
#ifdef __cplusplus
END_EXTERN_C()
#endif

/*
//modify
PHP_MINIT_FUNCTION(hlbs)
{
	ConnectAgent();	//连接agent
}

//modify
PHP_MSHUTDOWN_FUNCTION(hlbs)
{
	CloseAgent();	//连接agent
}
*/

//modify
PHP_MINFO_FUNCTION(hlbs)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "hlbs support", "enabled");
	php_info_print_table_end();

    //php_info_print_table_start();
    //php_info_print_table_header(2, "HLBS System", "HLBS System - php extension");
    //php_info_print_table_row(2, "Version",     PHP_HLBS_VERSION );
    //php_info_print_table_row(2, "Anthor",      "PF, Hupu Inc." );
    //php_info_print_table_row(2, "Copyright",   "Copyright (c) 2016 Hupu Inc. All Rights Reserved." );
    //php_info_print_table_end();
	
	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}

PHP_FUNCTION(hlbs_query_svr)
{
    double iTimeOut   = 0;

	zval *pSvrParam;		//路由zval（参数）
    zval **zvalGid;			//gid zval
    zval **zvalXid;			//xid zval
	//zval **zval_key;
	
    HashTable	*pHTSvrReq;	//路由hash

    int iArgc = ZEND_NUM_ARGS();
    if (iArgc == 2)
	{
        if (zend_parse_parameters(iArgc TSRMLS_CC, "ad", &pSvrParam, &iTimeOut) == FAILURE )
		{
            RETURN_LONG(-1);
		}
    }
	else 
	{
        WRONG_PARAM_COUNT;
        RETURN_LONG(-1);
    }

    pHTSvrReq = Z_ARRVAL_P(pSvrParam);
    if (zend_hash_find(pHTSvrReq, "gid", sizeof("gid"), (void **) &zvalGid) == FAILURE)
	{
        RETURN_LONG(-1);
	}
		
	convert_to_long(*zvalGid);
	
    if (zend_hash_find(pHTSvrReq, "xid", sizeof("xid"), (void **) &zvalXid) == FAILURE)
	{
		RETURN_LONG(-1);
    }
	else
	{
		std::string sErrMsg;
		struct SvrNet_t stSvr;

		convert_to_long(*zvalXid);
		
		stSvr.mGid = Z_LVAL_PP(zvalGid);
		stSvr.mXid = Z_LVAL_PP(zvalXid);

		int iRet = QueryNode(stSvr, (float)iTimeOut, sErrMsg);
		if(iRet >= 0)
		{
			add_assoc_stringl(pSvrParam, "host", (char *)stSvr.mHost, strlen(stSvr.mHost), 1);
			add_assoc_long(pSvrParam, "port", stSvr.mPort);
		}
		RETURN_LONG(iRet);
	}
}

/**
 * <0 上报失败
 * =0 上报成功
 */
PHP_FUNCTION(hlbs_notify_res)
{
    long iInterfRet   = 0;	//结果
    long iUsetimeUsec = 0;	//延时

    zval *pSvrParam;		//路由zval（参数）
    zval **zvalGid;			//gid zval
    zval **zvalXid;			//xid zval
    zval **zvalHost;		//host zval
    zval **zvalPort;		//port zval
	//zval **zval_key;		//key zval（xid、key都可索引 特定类别服务）
		
    HashTable	*pHTSvrReq;	//路由hash

	int iArgc = ZEND_NUM_ARGS();
    if (iArgc == 3) 
	{
        if (zend_parse_parameters(iArgc TSRMLS_CC, "all", &pSvrParam, &iInterfRet, &iUsetimeUsec) == FAILURE)
		{
            RETURN_LONG(-1);
		}
    }
	else
	{
        WRONG_PARAM_COUNT;
        RETURN_LONG(-1);
    }

    pHTSvrReq = Z_ARRVAL_P(pSvrParam);
	
    if (zend_hash_find(pHTSvrReq, "gid", sizeof("gid"), (void **) &zvalGid) == FAILURE)
	{
        RETURN_LONG(-1);
	}
		
	convert_to_long(*zvalGid);
	
	if (zend_hash_find(pHTSvrReq, "host", sizeof("host"), (void **) &zvalHost) == FAILURE)
	{
        RETURN_LONG(-1);
	}
	if (Z_TYPE_PP(zvalHost) != IS_STRING )
	{
		RETURN_LONG(-1);
	}
   
    if (zend_hash_find(pHTSvrReq, "port", sizeof("port"), (void **)&zvalPort)  == FAILURE)
	{
		RETURN_LONG(-1);
	}
	convert_to_long(*zvalPort);
	
	if (zend_hash_find(pHTSvrReq, "xid", sizeof("xid"), (void **) &zvalXid) == FAILURE)
	{
		RETURN_LONG(-1);
    }
	else
	{
		struct SvrNet_t stSvr;
		std::string sErrMsg;

		convert_to_long(*zvalXid);

		stSvr.mGid = Z_LVAL_PP(zvalGid);
		stSvr.mXid = Z_LVAL_PP(zvalXid);
		stSvr.mPort = Z_LVAL_PP(zvalPort);
		memcpy(stSvr.mHost, Z_STRVAL_PP(zvalHost), sizeof(stSvr.mHost));

		int iRet = NotifyCallerRes(stSvr, iInterfRet, iUsetimeUsec, sErrMsg);
		RETURN_LONG(iRet);
	}
}

PHP_FUNCTION(hlbs_notify_num)
{
	int argc = ZEND_NUM_ARGS();
	long gid;
	long xid;

	if (zend_parse_parameters(argc TSRMLS_CC, "ll", &gid, &xid) == FAILURE) 
		return;

	php_error(E_WARNING, "hlbs_notify_num: not yet implemented");
}

