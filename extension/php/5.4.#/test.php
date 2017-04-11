<?php
include "Hlbs.class.php";

$hlbs = new Hlbs(1, 2, 30);

var_dump("GetSvr", $hlbs->GetSvr(), $hlbs->SvrInfo());

usleep(1000);

var_dump("NotifySvr", $hlbs->NotifySvr());

?>
