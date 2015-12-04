DELIMITER ;

--
-- 手机信息
--
DROP table if exists `phone_info`;
CREATE TABLE `phone_info` (
  `info_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `product_flavor` varchar(45) NOT NULL,
  `android_version` varchar(45) NOT NULL,
  `brand` varchar(45) NOT NULL,
  `screen_x_pixel` int(11) unsigned NOT NULL default '0',
  `screen_y_pixel` int(11) unsigned NOT NULL default '0',
  `gps_province` varchar(45) NOT NULL,
  `gps_city` varchar(45) NOT NULL,
  `gps_district` varchar(45) NOT NULL,
  `gps_street` varchar(45) NOT NULL,
  `gps_street_detail` varchar(45) NOT NULL,
  `create_time` datetime NOT NULL,
  `last_update_time` datetime NOT NULL,
  `phone_type` varchar(45) NOT NULL,
  `product_name` varchar(45) NOT NULL,
  `cpu_type` varchar(45) NOT NULL,
  `client_version` int(11) unsigned NOT NULL default '0',
  `gps_latitude` varchar(45) NOT NULL,
  `gps_longitude` varchar(45) NOT NULL,
  `gps_accuracy` varchar(45) NOT NULL,
  `update_times` int(11) unsigned NOT NULL default '0',
  `last_push_time` datetime NOT NULL,
  `last_sms_time` datetime NOT NULL,
  `last_notify_time` datetime NOT NULL,
  `last_shortcut_time` datetime NOT NULL,
  `new_flag` tinyint(3) unsigned NOT NULL default '0',
  `push_flag` tinyint(3) unsigned NOT NULL default '0',
  `channel_number` varchar(45) NOT NULL,
  `blue_tooth_mac` varchar(45) NOT NULL,
  `phone_number` varchar(45) NOT NULL,
  `wifi_mac` varchar(45) NOT NULL,
  `imei1` varchar(45) NOT NULL,
  `imei2` varchar(45) NOT NULL,
  `imsi` varchar(45) NOT NULL,
  `current_zone` varchar(45) NOT NULL,
  `build_version` bigint(20) unsigned NOT NULL default '0',
  `kernel_version` varchar(45) NOT NULL,
  PRIMARY KEY (`info_id`),
  KEY `index_blue_tooth_mac` (`blue_tooth_mac`),
  KEY `index_imei1` (`imei1`),
  KEY `index_imei2` (`imei2`),
  KEY `index_last_update_time` (`last_update_time`),
  KEY `index_last_sms_time` (`last_sms_time`),
  KEY `index_phone_type` (`phone_type`),
  KEY `index_new_flag` (`new_flag`),
  KEY `index_create_time` (`create_time`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

DELIMITER ;;


drop PROCEDURE if exists `create_installed_packet`;
create PROCEDURE `create_installed_packet` (
	platform varchar(45)
)
BEGIN
	set @sql = concat("create table if not exists `installed_packet_", platform, "` (",
		"`info_id` int(11) unsigned NOT NULL default '0',",
		"`packet_name` varchar(45) NOT NULL,",
		"`create_time` varchar(45) NOT NULL,",
		"KEY `index_info_id` (`info_id`),",
		"KEY `index_packet_name` (packet_name(8))",
		") ENGINE=InnoDB DEFAULT CHARSET=utf8");
	prepare create_tb from @sql;
	execute create_tb;
END ;;

drop PROCEDURE if exists `create_oem_packet`;
create PROCEDURE `create_oem_packet` (
	platform varchar(45)
)
BEGIN
	set @sql = concat("create table if not exists `oem_packet_", platform, "` (",
		"`info_id` int(11) unsigned NOT NULL default '0',",
		"`packet_name` varchar(100) NOT NULL,",
		"`apk_name` varchar(100) NOT NULL,",
		"`create_time` datetime NOT NULL,",
		"KEY `index_info_id` (`info_id`)",
		") ENGINE=InnoDB DEFAULT CHARSET=utf8");
	prepare create_tb from @sql;
	execute create_tb;
END ;;

drop PROCEDURE if exists `create_install_log`;
create PROCEDURE `create_install_log` (
	platform varchar(45)
)
BEGIN
	set @sql = concat("create table if not exists `install_log_", platform, "` (",
		"`info_id` int(11) unsigned NOT NULL default '0',",
		"`packet_name` varchar(100) NOT NULL,",
		"`install_time` varchar(45) NOT NULL,",
		"`create_time` datetime NOT NULL,",
		"KEY `index_info_id` (`info_id`),",
		"KEY `index_packet_name` (packet_name(8))",
		") ENGINE=InnoDB DEFAULT CHARSET=utf8");
	prepare create_tb from @sql;
	execute create_tb;
END ;;

drop PROCEDURE if exists `create_uninstall_log`;
create PROCEDURE `create_uninstall_log` (
	platform varchar(45)
)
BEGIN
	set @sql = concat("create table if not exists `uninstall_log_", platform, "` (",
		"`info_id` int(11) unsigned NOT NULL default '0',",
		"`packet_name` varchar(100) NOT NULL,",
		"`uninstall_time` varchar(45) NOT NULL,",
		"`create_time` datetime NOT NULL,",
		"KEY `index_info_id` (`info_id`)",
		") ENGINE=InnoDB DEFAULT CHARSET=utf8");
	prepare create_tb from @sql;
	execute create_tb;
END ;;


drop PROCEDURE if exists `create_platform_table`;
create PROCEDURE `create_platform_table` (
	platform varchar(45)
)
BEGIN
	call create_installed_packet(platform);
	call create_oem_packet(platform);
	call create_install_log(platform);
	call create_uninstall_log(platform);
END ;;

DELIMITER ;
