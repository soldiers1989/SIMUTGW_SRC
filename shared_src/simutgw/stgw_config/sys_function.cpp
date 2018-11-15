#include "sys_function.h"

#include "simutgw/stgw_config/g_values_inner.h"
#include "simutgw/stgw_config/g_values_net.h"
#include "simutgw_config/g_values_sys_run_config.h"

#include "tool_redis/Tgw_RedisHelper.h"

#include "simutgw/settlement/Settlement.h"
#include "simutgw/settlement/DayEndClean.h"

#include "simutgw/work_manage/SystemInit.h"
#include "simutgw/stgw_config/g_values_net.h"

#include "cache/UserStockHelper.h"
#include "simutgw/order/OrderRepeatChecker.h"

using namespace std;

namespace simutgw
{
	/*
		控件自身必要程序回收
		*/
	void SimuTgwSelfExit(void)
	{
		static const string strTag("SimuTgwSelfExit() ");

		if (nullptr != simutgw::g_SocketClient)
		{
			simutgw::g_SocketClient->Disconnect(0);
			simutgw::g_SocketClient.reset();
		}

		if (nullptr != simutgw::g_SocketIOCPServer)
		{
			simutgw::g_SocketIOCPServer->StopServer();
			EzLog::i(strTag, "g_SocketIOCPServer->StopServer()");
			simutgw::g_SocketIOCPServer.reset();
		}

		simutgw::g_fixaccptor.Stop();

		simutgw::g_mtskPool_valid.StopPool_WaitAllFinish();
		EzLog::i(strTag, "g_mtskPool_valid StopPool_WaitAllFinish");

		simutgw::g_mtskPool_match_cancel.StopPool_WaitAllFinish();
		EzLog::i(strTag, "g_mtskPool_match_cancel StopPool_WaitAllFinish");

		simutgw::g_flowManage.StopFlows();
		EzLog::i(strTag, "g_flowManage StopFlows");

		simutgw::g_asyncDbwriter.Stop();
		EzLog::i(strTag, "g_asyncDbwriter Stop");

		simutgw::g_redisPool.Stop();

		//释放资源
		MySqlCnnC602_Guard::MysqlLibraryEnd();

		Tgw_RedisHelper::FreeHiredisLibrary();
	}

	/*
	控件自身必要程序回收
	指令重启功能
	*/
	void SimuTgwSelfExit_remoterestart(void)
	{
		static const string strTag("SimuTgwSelfExit() ");

		if (nullptr != simutgw::g_SocketClient)
		{
			simutgw::g_SocketClient->Disconnect(0);
		}

		if (nullptr != simutgw::g_SocketIOCPServer)
		{
			simutgw::g_SocketIOCPServer->StopServer();
			EzLog::i(strTag, "g_SocketIOCPServer->StopServer()");
			simutgw::g_SocketIOCPServer.reset();
		}

		simutgw::g_fixaccptor.Stop();

		simutgw::g_mtskPool_valid.StopPool_WaitAllFinish();
		EzLog::i(strTag, "g_mtskPool_valid StopPool_WaitAllFinish");

		simutgw::g_mtskPool_match_cancel.StopPool_WaitAllFinish();
		EzLog::i(strTag, "g_mtskPool_match_cancel StopPool_WaitAllFinish");

		simutgw::g_flowManage.StopFlows();
		EzLog::i(strTag, "g_flowManage StopFlows");

		simutgw::g_asyncDbwriter.Stop();
		EzLog::i(strTag, "g_asyncDbwriter Stop");

		simutgw::g_redisPool.Stop();

		//释放资源
		MySqlCnnC602_Guard::MysqlLibraryEnd();

		Tgw_RedisHelper::FreeHiredisLibrary();
	}

	/*
	结算
	@param const std::vector<std::string>& in_vctSettleGroup : 清算池别名
	@param std::string& out_strDay : 当前日期字符串
	@param std::string& out_strSettlementFilePath : 清算文件所在路径

	Return :
	0 -- 成功
	非0 -- 失败
	*/
	int Simutgw_Settle(const std::vector<std::string>& in_vctSettleGroup,
		std::string& out_strDay, std::string& out_strSettlementFilePath)
	{
		static const std::string strTag("Method_Settle() ");
		//"{"recno": 1,"fields" : {"S1":"000000","S2":"1","S3":"2","S4":"3"}}"
		//"{\"recno\": 1,\"fields\" : {\"S1\":\"000000\",\"S2\":\"1\",\"S3\":\"2\",\"S4\":\"3\"}}"

		Settlement settle;

		//从mysql连接池取连接
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//取出的mysql连接为NULL

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			EzLog::e(strTag, "Get Connection is NULL");

			return -1;
		}

		// 清算文件路径
		string strDay;
		string strSettlePath;

		int iRes = settle.MakeSettlement(mysqlConn, in_vctSettleGroup, strDay, strSettlePath);
		if (iRes < 0)
		{
			EzLog::e(strTag, "MakeSettlement() faild");

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			return -1;
		}

		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

		out_strDay = strDay;
		out_strSettlementFilePath = strSettlePath;
		
		return 0;
	}

	/*
	日终
	*/
	int Simutgw_DayEnd()
	{
		static const std::string strTag("Method_DayEnd()");

		//从mysql连接池取连接
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//取出的mysql连接为NULL

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			EzLog::e(strTag, "Get Connection is NULL");

			return -1;
		}

		//设置mysql连接
		DayEndClean end;

		int iRes = end.RunDayEndClean(mysqlConn);
		if (iRes < 0)
		{
			EzLog::e(strTag, "DayEndClean() faild");

			//归还连接
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			return -1;
		}

		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

		// 清空缓存中用户股份
		UserStockHelper::DayEndCleanUp();

		OrderRepeatChecker::DayEndCleanUpInRedis();

		return 0;
	}
}