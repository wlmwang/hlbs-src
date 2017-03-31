<?php
namespace prestige_sdk;
class Hlbs
{
	public $aSvrReq = array();
	public $uBeginSec = 0;
	public $uEndSec = 0;

	public $aGetSvrRet = array(
			 0 => "获取Svr成功",
			-1 => "连接AgentSvrd失败",
			-2 => "发送查询Svr请求失败",
			-3 => "接受查询Svr返回失败（过载）",
		);

	public $aNotifySvrRet = array(
			 0 => "上报Svr成功",
			-1 => "连接AgentSvrd失败",
			-2 => "发送上报Svr请求失败",
			-3 => "接受上报Svr返回失败",
		);

	public function __construct($iGid, $iXid)
	{
		$this->aSvrReq = ['gid'=> $iGid, 'xid'=> $iXid];
	}

	/**
	 * 获取一个Svr
	 * @param  float  $iTimeout 超时时间
	 * @return [int]          =0 正确，<0 获取失败
	 */
	public function getSvr($iTimeout = 30)
	{
		if (empty($this->aSvrReq['gid']) || empty($this->aSvrReq['xid'])) 
		{
			return -1;
		}

		$iRet = hlbs_query_svr($this->aSvrReq, $iTimeout);
		if ($iRet < 0) 
		{
			return $iRet;
		}
		$this->uBeginSec = microtime(true);
		return 0;
	}

	/**
	 * 上报结果
	 * @param integer $iRet     <0失败 >=0成功
	 * @param integer $uTimeSuc 微妙用时
	 */
	public function NotifySvr($iRet = 0, $uTimeSuc = 0)
	{
		if (empty($this->aSvrReq['gid']) || empty($this->aSvrReq['xid']) || empty($this->aSvrReq['host']) || empty($this->aSvrReq['port'])) 
		{
			return -1;
		}

		if ($uTimeSuc == 0) 
		{
			$this->uEndSec = microtime(true);
			$uTimeSuc = $this->uEndSec - $this->uBeginSec;
		}
		$iRet = hlbs_notify_res($this->aSvrReq, $iRet, $uTimeSuc * 1000000);
		return $iRet;
	}

	public function Start()
	{
		$this->uBeginSec = microtime(true);
	}

	public function End()
	{
		$this->uEndSec = microtime(true);
	}
}

?>