<?php

/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

class Hlbs {
	private static $_version = "3.0.0";

	private static $_getSvrStatus = [
		 0	=> "获取Svr成功",
		-1	=> "系统错误",
		-2	=> "连接AgentSvr失败",
		-3	=> "发送查询Svr请求失败",
		-4	=> "接受查询Svr返回失败（过载）",
		-5	=> "返回数据格式错误",
		99	=> "hlbs.so扩展错误",
	];
	
	private static $_notifySvrStatus = [
		 0	=> "上报Svr成功",
		-1	=> "系统错误",
		-2	=> "连接AgentSvr失败",
		-3	=> "发送上报Svr请求失败",
		-4	=> "接受上报Svr返回失败",
		-5	=> "返回数据格式错误",
		99	=> "hlbs.so扩展错误",
	];

	private $svrInfo = [
		'gid' => 0,
		'xid' => 0,
		'host'=> '',
		'port'=> '',
	];

	private $beginSec = 0;
	private $endSec = 0;
	private $timeout = 30;

	public function __construct($gid, $xid, $def_host, $def_port, $timeout = 30) {
		$this->timeout = $timeout;
		$this->svrInfo = ['gid'=> $gid, 'xid'=> $xid, 'host'=> $def_host, 'port'=> $def_port];
	}

	/**
	 * 获取一个Svr
	 */
	public function GetSvr() {
		if (extension_loaded('hlbs')) {
			$ret = hlbs_query_svr($this->svrInfo, $this->timeout);
		} else {
			$ret = 99;
		}
		$this->Start();
		return ['ret' => $ret, 'msg' => self::$_getSvrStatus[$ret]];
	}

	/**
	 * 上报svr访问结果
	 */
	public function NotifySvr($ret = 0) {
		if (extension_loaded('hlbs')) {
			$ret = hlbs_notify_res($this->svrInfo, $ret, ($this->End() - $this->beginSec) * 1000000);
		} else {
			$ret = 99;
		}
		return ['ret' => $ret, 'msg' => self::$_notifySvrStatus[$ret]];
	}

	public function SvrInfo() {
		return $this->svrInfo;
	}

	private function Start() {
		return $this->beginSec = microtime(true);
	}

	private function End() {
		return $this->endSec = microtime(true);
	}
}

?>