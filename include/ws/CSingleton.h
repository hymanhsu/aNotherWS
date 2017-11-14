/**
* @file       CSingleton.h
* @brief      单例模式的实现
* @details    使用例子：
*
* class Test1
* {
* DEFINE_SINGLETON(Test1)
* public:
*    Test1(){
*        printf("test1 construct\n");
*    }
*    void doit(){
*        printf("test1\n");
*    }
* };
*
* arc::CSingleton<Test1>::Instance()->doit();
*
* @author     Kevin.XU
* @date       2017.5.3
* @version    1.0.0
* @par Copyright (c):
*      ...
* @par History:
*   1.0.0: Kevin.XU, 2017.5.3, Create\n
*/

#ifndef UVTEST_CSINGLETON_H
#define UVTEST_CSINGLETON_H


//#include <ws/CLock.h>

#include <cstdlib>


//namespace arc{

/**
 * @brief 定义一个单例的宏
 */
#define DEFINE_SINGLETON(T)			\
private:					\
	T();					\
	T(const T& s);				\
	T& operator = (const T& s);		\
	~T();					\
public:						\
	static T& instance(){			\
		static T _uniqueInstance;	\
		return _uniqueInstance;		\
	}					\


//}


#endif //UVTEST_SINGLETON_H
