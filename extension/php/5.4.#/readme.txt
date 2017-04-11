
###############################
# Copyright (C) Anny Wang.
# Copyright (C) Hupu, Inc.
###############################


# ./lib/目录下所有文件均来自其他目录，并应以其他目录为准。建议在编译扩展时实时覆盖该目录下的所有文件
# cp -r /usr/local/webserver/hlbs/extension/c++/common/* .
# cp -r /usr/local/webserver/hlbs/command/* .


###############################
hlbs.so扩展命令集合
###############################

# cd /usr/local/src/php-5.4.#/ext      #请自行更新PHP源码目录
# ./ext_skel --extname=hlbs
# cd hlbs
# cp -rf /usr/local/webserver/hlbs/extension/php/5.4.#/* .
# /usr/local/php/bin/phpize
# ./configure --with-php-config=/usr/local/php/bin/php-config
# make && make test
# make install
# echo extension=hlbs.so >> /usr/local/php/etc/php.ini
# /usr/local/php/bin/php –m |grep hlbs