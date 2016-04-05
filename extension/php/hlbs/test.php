<?php

$svr_req = array(
	'gid' => 1,	
	'xid' => 2
);
$time_out = 0.2;
$ret = hlbs_query_svr($svr_req, $time_out);

print_r($svr_req);
echo "[hlbs_query_svr] ret:".$ret."<br/>";

if (!empty($svr_req['host'])) 
{
	$svr_req = array(
	 	'gid' => '1',	
	 	'xid' => '2',
	    'host' => $svr_req['host'],
	    'port' => $svr_req['port'],
	 );

	$ret = hlbs_notify_res($svr_req, 0, 3000000);
	echo "[hlbs_notify_res] ret:".$ret."\n";
}

?>
