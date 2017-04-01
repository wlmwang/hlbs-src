# php扩展安装

```
# cd /usr/local/src/php-5.4.#/ext      #请自行更新PHP源码目录
# ./ext_skel --extname=hlbs
# cd hlbs
# cp -rf /usr/local/webserver/hlbs/extension/php/5.4.#/* .
# cp -rf /usr/local/lib/libtinyxml.a /usr/local/lib/libhnet.a /usr/local/lib/libprotobuf.a lib
***
./lib/目录下所有文件均来自其他目录，并应以其他目录为准。建议在编译扩展时实时覆盖该目录下的所有文件
cp -rf /usr/local/webserver/hlbs/extension/c++/common/* lib
cp -rf /usr/local/webserver/hlbs/command/* lib
***

# /usr/local/php/bin/phpize
# ./configure --with-php-config=/usr/local/php/bin/php-config
# make && make test
# make install
# echo extension=hlbs.so >> /usr/local/php/etc/php.ini  #添加 extension=hlbs.so
# /usr/local/php/bin/php –m |grep hlbs  #查看是否安装成功
```
