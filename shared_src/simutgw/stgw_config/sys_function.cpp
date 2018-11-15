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
		�ؼ������Ҫ�������
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

		//�ͷ���Դ
		MySqlCnnC602_Guard::MysqlLibraryEnd();

		Tgw_RedisHelper::FreeHiredisLibrary();
	}

	/*
	�ؼ������Ҫ�������
	ָ����������
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

		//�ͷ���Դ
		MySqlCnnC602_Guard::MysqlLibraryEnd();

		Tgw_RedisHelper::FreeHiredisLibrary();
	}

	/*
	����
	@param const std::vector<std::string>& in_vctSettleGroup : ����ر���
	@param std::string& out_strDay : ��ǰ�����ַ���
	@param std::string& out_strSettlementFilePath : �����ļ�����·��

	Return :
	0 -- �ɹ�
	��0 -- ʧ��
	*/
	int Simutgw_Settle(const std::vector<std::string>& in_vctSettleGroup,
		std::string& out_strDay, std::string& out_strSettlementFilePath)
	{
		static const std::string strTag("Method_Settle() ");
		//"{"recno": 1,"fields" : {"S1":"000000","S2":"1","S3":"2","S4":"3"}}"
		//"{\"recno\": 1,\"fields\" : {\"S1\":\"000000\",\"S2\":\"1\",\"S3\":\"2\",\"S4\":\"3\"}}"

		Settlement settle;

		//��mysql���ӳ�ȡ����
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//ȡ����mysql����ΪNULL

			//�黹����
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			EzLog::e(strTag, "Get Connection is NULL");

			return -1;
		}

		// �����ļ�·��
		string strDay;
		string strSettlePath;

		int iRes = settle.MakeSettlement(mysqlConn, in_vctSettleGroup, strDay, strSettlePath);
		if (iRes < 0)
		{
			EzLog::e(strTag, "MakeSettlement() faild");

			//�黹����
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			return -1;
		}

		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

		out_strDay = strDay;
		out_strSettlementFilePath = strSettlePath;
		
		return 0;
	}

	/*
	����
	*/
	int Simutgw_DayEnd()
	{
		static const std::string strTag("Method_DayEnd()");

		//��mysql���ӳ�ȡ����
		std::shared_ptr<MySqlCnnC602> mysqlConn = simutgw::g_mysqlPool.GetConnection();
		if (NULL == mysqlConn)
		{
			//ȡ����mysql����ΪNULL

			//�黹����
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			EzLog::e(strTag, "Get Connection is NULL");

			return -1;
		}

		//����mysql����
		DayEndClean end;

		int iRes = end.RunDayEndClean(mysqlConn);
		if (iRes < 0)
		{
			EzLog::e(strTag, "DayEndClean() faild");

			//�黹����
			simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

			return -1;
		}

		simutgw::g_mysqlPool.ReleaseConnection(mysqlConn);

		// ��ջ������û��ɷ�
		UserStockHelper::DayEndCleanUp();

		OrderRepeatChecker::DayEndCleanUpInRedis();

		return 0;
	}
}