# php扩展安装

```
$ cd /usr/local/src/php-5.4.45/ext      #请自行更新PHP源码目录
$ ./ext_skel --extname=hlbs
$ cd hlbs
$ cp -r /usr/local/webserver/hlbs/extension/php/hlbs/* .
$ cp -r /usr/local/webserver/hlbs/extension/c++/common/* .
$ cp -r /usr/local/webserver/hlbs/command/* .
$ /usr/local/php/bin/phpize
$ ./configure --with-php-config=/usr/local/php/bin/php-config
$ make
$ make test
$ make install
$ vim /usr/local/php/etc/php.ini    #添加 extension=hlbs.so
$ /usr/local/php/bin/php –m |grep hlbs    #查看是否安装成功
hlbs #安装成功
```
