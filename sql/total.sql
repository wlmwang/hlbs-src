DELIMITER ;

--
-- 推送信息
--
DROP table if exists `push_info`;
CREATE TABLE `push_info` (
  `push_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `action_type` int(11) unsigned NOT NULL default '0',
  `start_time` datetime NOT NULL,
  `end_time` datetime NOT NULL,
  `create_time` datetime NOT NULL,
  `author` varchar(45) NOT NULL,
  `md5sum` varchar(34) NOT NULL,
  `packet_name` varchar(100) NOT NULL,
  `apk_name` varchar(100) NOT NULL,
  `total_count` int(11) unsigned NOT NULL default '0',
  `current_count` int(11) unsigned NOT NULL default '0',
  `push_ever_install` bigint(20) unsigned NOT NULL default '1',
  `if_startup` tinyint(3) unsigned NOT NULL default '0',
  `conditions` blob NOT NULL,
  `action` blob NOT NULL,
  `time_check` tinyint(3) unsigned NOT NULL default '1',
  `success_count` int(11) unsigned NOT NULL default '0',
  `switch` tinyint(3) unsigned NOT NULL default '1',
  `push_installed` tinyint(3) unsigned NOT NULL default '1',
  `if_system` tinyint(3) unsigned NOT NULL default '0',
  `if_interval` tinyint(3) unsigned NOT NULL default '1',
  `addon_installed` tinyint(3) unsigned NOT NULL default '0',
  `addon_packet_name` varchar(100) NOT NULL default "",
  `if_screen` tinyint(3) unsigned NOT NULL default '0',
  `screen_x` int(11) unsigned NOT NULL default '0',
  `screen_y` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY (`push_id`),
  KEY `index_start_time` (`start_time`),
  KEY `index_end_time` (`end_time`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

--
-- 手机内部错误信息
--
DROP table if exists `upgraded_error_info`;
CREATE TABLE `upgraded_error_info` (
  `info_id` int(11) NOT NULL AUTO_INCREMENT,
  `product_platform` varchar(45) NOT NULL,
  `product_flavor` varchar(45) NOT NULL,
  `blue_tooth_mac` varchar(45) NOT NULL,
  `imei1` varchar(45) NOT NULL,
  `imei2` varchar(45) NOT NULL,
  `build_version` bigint(20) unsigned NOT NULL DEFAULT '0',
  `create_time` datetime NOT NULL,
  PRIMARY KEY (`info_id`),
  KEY `index_blue_tooth_mac` (`blue_tooth_mac`),
  KEY `index_product_flavor` (`product_flavor`),
  KEY `index_imei1` (`imei1`),
  KEY `index_imei2` (`imei2`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

--
-- 手机错误信息
--
DROP table if exists `error_info`;
CREATE TABLE `error_info` (
  `info_id` int(11) NOT NULL AUTO_INCREMENT,
  `product_platform` varchar(45) NOT NULL,
  `product_flavor` varchar(45) NOT NULL,
  `channel_number` varchar(45) NOT NULL,
  `blue_tooth_mac` varchar(45) NOT NULL,
  `wifi_mac` varchar(45) NOT NULL,
  `phone_number` varchar(45) NOT NULL,
  `imei1` varchar(45) NOT NULL,
  `imei2` varchar(45) NOT NULL,
  `imsi` varchar(45) NOT NULL,
  `current_zone` varchar(45) NOT NULL,
  `kernel_version` varchar(45) NOT NULL,
  `android_version` varchar(45) NOT NULL,
  `build_version` bigint(20) unsigned NOT NULL DEFAULT '0',
  `brand` varchar(45) NOT NULL,
  `phone_type` varchar(45) NOT NULL,
  `product_name` varchar(45) NOT NULL,
  `cpu_type` varchar(45) NOT NULL,
  `screen_x_pixel` int(11) NOT NULL DEFAULT '0',
  `screen_y_pixel` int(11) NOT NULL DEFAULT '0',
  `client_version` int(11) NOT NULL,
  `gps_latitude` varchar(45) NOT NULL,
  `gps_longitude` varchar(45) NOT NULL,
  `gps_accuracy` varchar(45) NOT NULL,
  `gps_province` varchar(45) NOT NULL,
  `gps_city` varchar(45) NOT NULL,
  `gps_district` varchar(45) NOT NULL,
  `gps_street` varchar(45) NOT NULL,
  `gps_street_detail` varchar(45) NOT NULL,
  `create_time` datetime NOT NULL,
  PRIMARY KEY (`info_id`),
  KEY `index_product_platform` (`product_platform`),
  KEY `index_blue_tooth_mac` (`blue_tooth_mac`),
  KEY `index_imei1` (`imei1`),
  KEY `index_imei2` (`imei2`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

--
-- 一些配置信息以及通用配置
--
DROP table if exists `setting_info`;
CREATE TABLE `setting_info` (
	days_per_push double unsigned NOT NULL DEFAULT '0',
	days_per_sms double unsigned NOT NULL DEFAULT '0',
	days_per_notify double unsigned NOT NULL DEFAULT '0',
	days_per_shortcut double unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO setting_info (days_per_push, days_per_sms, days_per_notify, days_per_shortcut) VALUES (7.0, 0.5, 0.5, 0.5);

DELIMITER ;;

drop PROCEDURE if exists `create_info`;
create PROCEDURE `create_info` (
	platform varchar(45)
)
BEGIN
	set @sql = concat("create table if not exists `info_", platform, "` (",
		"`info_id` int(11) unsigned NOT NULL AUTO_INCREMENT,",
		"`product_flavor` varchar(45) NOT NULL,",
		"`channel_number` varchar(45) NOT NULL,",
		"`blue_tooth_mac` varchar(45) NOT NULL,",
		"`wifi_mac` varchar(45) NOT NULL,",
		"`phone_number` varchar(45) NOT NULL,",
		"`imei1` varchar(45) NOT NULL,",
		"`imei2` varchar(45) NOT NULL,",
		"`imsi` varchar(45) NOT NULL,",
		"`current_zone` varchar(45) NOT NULL,",
		"`kernel_version` varchar(45) NOT NULL,",
		"`android_version` varchar(45) NOT NULL,",
		"`build_version` bigint(20) unsigned NOT NULL DEFAULT '0',",
		"`brand` varchar(45) NOT NULL,",
		"`phone_type` varchar(45) NOT NULL,",
		"`product_name` varchar(45) NOT NULL,",
		"`cpu_type` varchar(45) NOT NULL,",
		"`screen_x_pixel` int(11) unsigned NOT NULL DEFAULT '0',",
		"`screen_y_pixel` int(11) unsigned NOT NULL DEFAULT '0',",
		"`client_version` int(11) unsigned NOT NULL DEFAULT '0',",
		"`gps_latitude` varchar(45) NOT NULL,",
		"`gps_longitude` varchar(45) NOT NULL,",
		"`gps_accuracy` varchar(45) NOT NULL,",
		"`gps_province` varchar(45) NOT NULL,",
		"`gps_city` varchar(45) NOT NULL,",
		"`gps_district` varchar(45) NOT NULL,",
		"`gps_street` varchar(45) NOT NULL,",
		"`gps_street_detail` varchar(45) NOT NULL,",
		"`create_time` datetime NOT NULL,",
		"`last_update_time` datetime NOT NULL,",
		"`update_times` int(11) unsigned NOT NULL DEFAULT '0',",
		"`last_push_time` datetime NOT NULL DEFAULT 0,",
		"`last_sms_time` datetime NOT NULL DEFAULT 0,",
		"`last_notify_time` datetime NOT NULL DEFAULT 0,",
		"`last_shortcut_time` datetime NOT NULL DEFAULT 0,",
		"`new_flag` tinyint(3) unsigned NOT NULL DEFAULT '0',",
		"`push_flag` tinyint(3) unsigned NOT NULL default '0',",
		"PRIMARY KEY (`info_id`),",
		"KEY `index_blue_tooth_mac` (`blue_tooth_mac`),",
		"KEY `index_imei1` (`imei1`),",
		"KEY `index_imei2` (`imei2`),",
		"KEY `index_last_update_time` (`last_update_time`),",
		"KEY `index_last_sms_time` (`last_sms_time`),",
		"KEY `index_phone_type` (`phone_type`),",
		"KEY `index_new_flag` (`new_flag`),",
		"KEY `index_create_time` (`create_time`)",
		") ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8");
	prepare create_tb from @sql;
	execute create_tb;
END ;;

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

drop PROCEDURE if exists `create_usage_log`;
create PROCEDURE `create_usage_log` (
	platform varchar(45)
)
BEGIN
	set @sql = concat("create table if not exists `usage_log_", platform, "` (",
		"`info_id` int(11) unsigned NOT NULL default '0',",
		"`packet_name` varchar(100) NOT NULL,",
		"`usage_time` int(11) unsigned NOT NULL,",
		"`duration` int(11) unsigned NOT NULL,",
		"`datetime` varchar(45) NOT NULL,",
		"`create_time` datetime NOT NULL,",
		"KEY `index_info_id` (`info_id`)",
		") ENGINE=InnoDB DEFAULT CHARSET=utf8");
	prepare create_tb from @sql;
	execute create_tb;
END ;;

drop PROCEDURE if exists `create_push_log`;
create PROCEDURE `create_push_log` (
	platform varchar(45)
)
BEGIN
	set @sql = concat("create table if not exists `push_log_", platform, "` (",
		"`info_id` int(11) unsigned NOT NULL default '0',",
		"`push_id` int(11) unsigned NOT NULL default '0',",
		"`flag` tinyint(3) unsigned NOT NULL default '0',",
		"`create_time` datetime NOT NULL,",
		"`count` int(11) unsigned NOT NULL default '0',",
		"KEY `index_info_id` (`info_id`)",
		") ENGINE=InnoDB DEFAULT CHARSET=utf8");
	prepare create_tb from @sql;
	execute create_tb;
END ;;

drop PROCEDURE if exists `create_active_log`;
create PROCEDURE `create_active_log` (
	platform varchar(45)
)
BEGIN
	set @sql = concat("create table if not exists `active_log_", platform, "` (",
		"`info_id` int(11) unsigned NOT NULL default '0',",
		"`date` datetime NOT NULL DEFAULT 0,",
		"`times` int(11) unsigned NOT NULL default '0',",
		"KEY `index_info_id` (`info_id`),",
		"KEY `index_date` (`date`)",
		") ENGINE=InnoDB DEFAULT CHARSET=utf8");
	prepare create_tb from @sql;
	execute create_tb;
END ;;

drop PROCEDURE if exists `create_platform_table`;
create PROCEDURE `create_platform_table` (
	platform varchar(45)
)
BEGIN
	call create_info(platform);
	call create_installed_packet(platform);
	call create_oem_packet(platform);
	call create_install_log(platform);
	call create_uninstall_log(platform);
	call create_usage_log(platform);
	call create_push_log(platform);
	call create_active_log(platform);
END ;;

DELIMITER ;
