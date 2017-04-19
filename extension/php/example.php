<?php
include "Hlbs.class.php";

$example = new Hlbs(1, 2, "127.0.0.1", 10001, 30);

// 获取
$example->GetSvr();
print_r($example->SvrInfo());

usleep(1000);

// 上报
print_r($example->NotifySvr(0));

?>
