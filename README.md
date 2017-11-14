uvws
====

based on libUV Websocket Server

基于libuv实现的Websocket通讯框架，实现RFC6455规范，目前仅提供ws方式。

wss未来视需要再进行实现。

第一行代码写于2017.4.28。


<br><br>

#功能介绍

* 支持RFC6455，暂时不支持SSL
* 服务器框架
* 客户端框架
* 异步任务管理


<br><br>

#开发成员

华海 : 421093703@qq.com

海生 : yuhaisheng163@yeah.net


<br><br>

#普通编译

mkdir build

cd build

cmake ..

make 

make install

<br><br>


#交叉编译

export THIRDPARTY_HOME=/root/local

export CTC_ATOM_HOME=/root/ctc-linux64-atom-2.5.2.74

mkdir build

cd build

cmake -DCMAKE_INSTALL_PREFIX=/root/local -DCMAKE_TOOLCHAIN_FILE=pepper-toolchain.cmake ..

make 

make install

<br><br>


