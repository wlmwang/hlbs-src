<?php
class Hlbs {

	private static $_getSvrStatus = [
		 0 => "获取Svr成功",
		-1 => "连接AgentSvrd失败",
		-2 => "发送查询Svr请求失败",
		-3 => "接受查询Svr返回失败（过载）",
	];
	private static $_notifySvrStatus = [
		 0 => "上报Svr成功",
		-1 => "连接AgentSvrd失败",
		-2 => "发送上报Svr请求失败",
		-3 => "接受上报Svr返回失败",
	];

	private $timeout = 30;
	private $beginSec = 0;
	private $endSec = 0;
	private $ret = 0;
	private $svrInfo = [];

	public function __construct($gid, $xid, $timeout = 30) {
		$this->svrInfo = ['gid'=> $gid, 'xid'=> $xid];
		$this->timeout = $timeout;
	}

	/**
	 * 获取一个Svr
	 */
	public function GetSvr() {
		if (empty($this->svrInfo['gid']) || empty($this->svrInfo['xid'])) {
			return $this->ret = -1;
		}

		$this->ret = hlbs_query_svr($this->svrInfo, $this->timeout);
		$this->Start();
		return ['ret' => $this->ret, 'msg' => self::$_getSvrStatus[$this->ret]];
	}

	/**
	 * 上报结果
	 */
	public function NotifySvr() {
		if (empty($this->svrInfo['gid']) || empty($this->svrInfo['xid']) || empty($this->svrInfo['host']) || empty($this->svrInfo['port'])) {
			return $this->ret = -1;
		}

		$this->ret = hlbs_notify_res($this->svrInfo, $this->ret, ($this->End() - $this->beginSec) * 1000000);
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