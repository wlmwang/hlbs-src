
#以下假设hlbs被安装在 /usr/local/webserver/hlbs 目录中

# ./lib/目录下所有文件均来自其他目录
# cp -rf /usr/local/webserver/hlbs/command/* lib
# cp -rf /usr/local/webserver/hlbs/extension/c++/common/* lib


--------------------------------------
php5.#.# 安装示例
(php7.#.# 安装仅需替换为对应版本)
--------------------------------------

# cd /usr/local/src/php-5.4.41/ext      #请自行替换为实际的PHP源码目录
# ./ext_skel --extname=hlbs
# cd hlbs
# cp -rf /usr/local/webserver/hlbs/extension/php/lib .
# cp -rf /usr/local/lib/libhnet.a lib/libhnet-php.a
# cp -rf /usr/local/webserver/hlbs/extension/php/5.#.#/* .
# /usr/local/php/bin/phpize 	#请自行替换为实际的PHP安装目录
# ./configure --with-php-config=/usr/local/php/bin/php-config 	#请自行替换为实际的PHP安装目录
# make && make test
# make install
# echo extension=hlbs.so >> /usr/local/php/etc/php.ini
# /usr/local/php/bin/php –m | grep hlbs 	#查看是否安装成功
# php /usr/local/webserver/hlbs/extension/php/example.php
