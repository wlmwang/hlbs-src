<?php

/**
 * Copyright (C) Anny Wang.
 * Copyright (C) Hupu, Inc.
 */

class Hlbs {
	private static $_getSvrStatus = [
		 0	=> "获取Svr成功",
		-1	=> "系统错误",
		-2	=> "连接AgentSvr失败",
		-3	=> "发送查询Svr请求失败",
		-4	=> "接受查询Svr返回失败（过载）",
		-5	=> "返回数据格式错误",
		-99	=> "hlbs.so扩展错误",
	];
	private static $_notifySvrStatus = [
		 0	=> "上报Svr成功",
		-1	=> "系统错误",
		-2	=> "连接AgentSvr失败",
		-3	=> "发送上报Svr请求失败",
		-4	=> "接受上报Svr返回失败",
		-5	=> "返回数据格式错误",
		-99	=> "hlbs.so扩展错误",
	];

	private $svrInfo = [
		'gid' => 0,
		'xid' => 0,
		'host'=> '',
		'port'=> '',
	];
	private $timeout = 30;
	private $beginSec = 0;
	private $endSec = 0;
	private $ret = 0;

	public function __construct($gid, $xid, $def_host, $def_port, $timeout = 30) {
		$this->svrInfo = ['gid'=> $gid, 'xid'=> $xid, 'host'=> $svr_host, 'port'=> $svr_port];
		$this->timeout = $timeout;
	}

	/**
	 * 获取一个Svr
	 */
	public function GetSvr() {
		if (empty($this->svrInfo['gid']) || empty($this->svrInfo['xid'])) {
			return $this->ret = -1;
		}
		if (extension_loaded('hlbs')) {
			$this->ret = hlbs_query_svr($this->svrInfo, $this->timeout);
			$this->Start();
		} else {
			$this->ret = -99;
		}
		return ['ret' => $this->ret, 'msg' => self::$_getSvrStatus[$this->ret]];
	}

	/**
	 * 上报结果
	 */
	public function NotifySvr() {
		if (empty($this->svrInfo['gid']) || empty($this->svrInfo['xid']) || empty($this->svrInfo['host']) || empty($this->svrInfo['port'])) {
			return $this->ret = -1;
		}
		if (extension_loaded('hlbs')) {
			$this->ret = hlbs_notify_res($this->svrInfo, $this->ret, ($this->End() - $this->beginSec) * 1000000);
		} else {
			$this->ret = -99;
		}
		return ['ret' => $this->ret, 'msg' => self::$_notifySvrStatus[$this->ret]];
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