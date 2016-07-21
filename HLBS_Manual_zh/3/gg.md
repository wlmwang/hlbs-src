# tinyxml

```
$ cd lib/tinyxml
$ mv Makefile.so Makefile
$ make
$ make install
```


# log4cpp

```
$ cd lib/log4cpp
$ tar –zxvf log4cpp-1.1.2rc1.tar.gz
$ cd log4cpp
$ ./configure
$ make
$ make check
$ make install #(默认路劲，库：/usr/local/lib	头：/usr/local/include/log4cpp)
```

# 加载相关安装库

```
$ cd /etc/ld.so.conf.d
$ touch hlbs.conf
$ echo "/usr/local/lib" >> hlbs.conf
$ ldconfig
```


