# 升级gcc到4.8+以支持c++11

* CentOs

    cent6 自带的gcc版本是4.4.7, 不过可以使用red hat出的devtoolset-3(其实这是一套, 有1,2 现在版本是3, 参照官方教程即可)升级gcc到4.8+, 使用yum可以直接搞定。

```
$ cd /usr/local/src
$ rpm –ivh https://www.softwarecollections.org/en/scls/rhscl/devtoolset-3/epel-6-x86_64/download/rhscl-devtoolset-3-epel-6-x86_64.noarch.rpm
$ yum install devtoolset-3-gcc-c++	#devtoolset-3-gcc会被作为依赖安装
$ scl enable devtoolset-3 bash		#安装完毕后使用命令临时切换gcc版本
$ gcc –v #查看版本
永久覆盖：
$ echo “source /opt/rh/devtoolset-3/enable” >> /etc/bashrc	#重新登录终端生效。
```

* Ubuntu

```
$ add-apt-repository ppa:ubuntu-toolchina-r/test #添加ppa源
$ apt-get update
$ apt-install g++-4.8
$ export CXX="g++-4.8"
$ gcc –v	#如果是4.8+，则忽略以下步骤
$ cd /usr/bin
$ ll gcc*		#若gcc->gcc-4.6符号没有替换，需重新链接
$ rm gcc
$ ln –s gcc-4.8 gcc
$ rm g++
$ ln –s g++-4.8 g++
```

* 源码安装

```
$ yum -y install gcc texinfo-tex flex zip libgcc.i686 glibc-devel.i686 glibc-devel	#centos x86_64 安装libgcc.i686 glibc-devel.i686
$ apt-get install libc6-dev-i386		#Ubuntu下执行该命令
$ curl –o ftp://ftp.gnu.org/gnu/gcc/gcc-4.8.2.tar.gz
$ tar –zxvf gcc-4.8.2.tar.gz
$ cd gcc-4.8.2
$ ./contrib/download_prerequisites #下载gcc依赖库，若服务器无法下载。则需手动下载gmp、mpfr、mpc。
$ mkdir gcc-build-4.8.2
$ cd gcc-build-4.8.2
$../configure--enable-checking=release --enable-languages=c,c++ --disable-multilib
--enable-language表示你要让你的gcc支持那些语言，--disable-multilib不生成编译为其他平台可执行代码的交叉编译器。--disable-checking生成的编译器在编译过程中不做额外检查，也可以使用--enable-checking=xxx来增加一些检。
如果不加–disable-multilib ，则需要  yum install glibc-devel.i686 glibc-devel

$ make -j4	#大约1小时。。。
$ make install
$ gcc –v	#如果是4.8+，则忽略以下步骤
$ source /etc/profile
$ source ~/.bash_profile #如果软连都没有，则手动ln
```




