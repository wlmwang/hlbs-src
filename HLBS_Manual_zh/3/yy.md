# 安装PHP扩展

```
$ cd /usr/local/webserver/hlbs/extension/c++
$ make clean
$ make
$ make install
$ ldconfig

$ cd /usr/local/src/php-5.5.16/ext 	#php源码目录
$ ./ext_skel --extname=hlbs
$ cd hlbs
$ cp –r /usr/local/webserver/hlbs/extension/php/hlbs/* .
$ /usr/local/php/bin/phpize		#php安装目录
$./configure --with-php-config=/usr/local/php/bin/php-config
$ make
$ make test
$ make install
$ vim /usr/local/php/etc/php.ini	#添加 extension=hlbs.so
$ /usr/local/php/bin/php –m |grep hlbs	#查看是否安装成功
  hlbs #安装成功
```

# 测试

```
$ cp /usr/local/webserver/hlbs/extension/php/test.php /data/www #修改文件test.php，测试结果
```
