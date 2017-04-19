<?php
include "Hlbs.class.php";

$example = new Hlbs(1, 1, "127.0.0.1", 8800, 30);

// 获取
print_r($example->GetSvr());
print_r($example->SvrInfo());

usleep(1000);

// 上报
print_r($example->NotifySvr(0));

?>