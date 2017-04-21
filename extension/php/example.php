<?php
include "Hlbs.class.php";

$hlbs = new Hlbs(1, 1, "127.0.0.1", 8800, 30);

$res = $hlbs->GetSvr();    // 获取一个可用的通知服务器地址
if ($res['ret'] >= 0) {
	$svr = $hlbs->SvrInfo();     // 可用服务器地址信息

	// 直连访问服务 start
	print_r($svr);
	usleep(1000);
	// 直连访问服务 end

	print_r($hlbs->NotifySvr(0));		// 上报服务使用情况 >=0 成功 < 0 为失败
} else {
	print_r($res);
}

?>