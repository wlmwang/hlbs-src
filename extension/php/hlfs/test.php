<?php

$svr_req = array(
	'gid' => 1,	
	'xid' => 1	
);
$time_out = 0.2;
$ret = hlfs_query_svr($svr_req, $time_out);

print_r($svr_req);
echo "ret:".$ret."\n";

$svr_req = array(
 	'gid' => '1',	
 	'xid' => '1',
    'host' => '192.168.8.13',
    'port' => '3306',
 );

$ret = hlfs_notify_res($svr_req, 0, 3000000);
echo "ret:".$ret."\n";

?>
