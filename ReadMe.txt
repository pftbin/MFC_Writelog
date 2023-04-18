使用方法：


//头文件
#include "WriteMsgToFile.h"
#pragma comment(lib, "WriteLog.lib")


//定义对象
IMPLEMENT_LOGER(Loger_Config);

//设定路径
INIT_LOGER(Loger_Config, _T(""), _T("Config"));

//外部extern使用
DECLARE_LOGER(Loger_Config);

//写入文件
WRITE_LOG(Loger_Config, 0, FALSE, _T("%s"), strLogInfo);