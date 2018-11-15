#ifndef __CONFIG_DEFINE_H__
#define __CONFIG_DEFINE_H__

#include <memory>
#include <stdint.h>  
#include <string>

class TaskPriorityBase;

namespace simutgw
{
	//
	// Members
	//
	// �Ǹ�����ģʽsleep time
	static const int g_iUnHighPfmSleepTime(1 * 1000);

	// ������ģʽsleep time
	static const int g_iHighPfmSleepTime(10);

	/* ���������� */
	static const int Flow_MatchType_SZ_BUY = 1;
	/* ������������ */
	static const int Flow_MatchType_SZ_SELL = 2;

	/* �����Ϻ��� */
	static const int Flow_MatchType_SH_BUY = 3;
	/* �����Ϻ����� */
	static const int Flow_MatchType_SH_SELL = 4;

	/* ����������Ϣ������ȡ����ί�кͷ��ر� */
	static const int Flow_MatchType_SZ_MSG = 5;
	/* �����Ϻ���Ϣ������ȡ����ί�кͷ��ر� */
	static const int Flow_MatchType_SH_MSG = 6;

	/* ��������redis���У�ȡredisί�е��ڴ���� */
	static const int Flow_MatchType_SZ_REDIS = 7;
	/* �����Ϻ�redis���У�ȡredisί�е��ڴ���� */
	static const int Flow_MatchType_SH_REDIS = 8;

	// A����������
	static const std::string g_Key_AStockQuotationChange("hqk_change");

	// ��̬����ǰ׺
	static const std::string g_Key_AStockStaticQuotation_Prefix("hqk_static_");

	// A�������Դ�ǰ׺
	static const std::string g_Key_AStockQuotTGW_Prefix("tgwhqk_");

	// A����������
	static const std::string g_Key_AStockQuotTGW_TradeVolume("tgwhqk_tv");

	// ���ж����Ŵ���
	static const std::string g_redis_Key_OrderId_Record("tgw_orderids");

	/*
	ϵͳ����ģʽ
	*/
	enum WebManMode
	{
		/* 0 -- ��Web����ģʽ */
		LocalMode = 0,

		/* 1 -- Web����ģʽ */
		WebMode = 1
	};

	/*
	ϵͳ����ģʽ
	*/
	enum SysRunMode
	{
		/* 1 -- ѹ��ģʽ */
		PressureMode = 1,

		/* 2 -- ����ģʽ */
		MiniMode = 2,

		/* 3 -- ��ͨģʽ */
		NormalMode = 3
	};

	/*
	ϵͳ�ɽ�ģʽ
	*/
	enum SysMatchMode
	{
		/* Ĭ�ϣ��������� */
		EnAbleQuta = 0,

		/* 1 --�������飬ȫ���ɽ� */
		SimulMatchAll = 1,

		/* 2 --�������飬�ֱʳɽ� */
		SimulMatchByDivide = 2,

		/* 3 --�������飬�ҵ������ɽ����ɳ��� */
		SimulNotMatch = 3,

		/* 4 --���� */
		SimulErrMatch = 4,

		/* 5 --���ֳɽ����ȳɽ�һ�룬ʣ��ҵ��ɳ��� */
		SimulMatchPart = 5
	};

	enum QuotationType
	{
		// ���ݱ䶯�ɽ��ܽ��ͳɽ���������ľ���
		AveragePrice = 0,

		// ��һ��һ
		SellBuyPrice = 1,

		// ����ɽ���
		RecentMatchPrice = 2
	};

	/*
	�ɽ�����
	*/
	enum MatchType
	{
		//ȫ���ɽ�
		MatchAll = 0,
		//���ֳɽ�
		MatchPart = 1,
		//����
		CancelMatch = 2,
		//δ�ɽ�
		NotMatch = -1,
		// �����ǵ���
		OutOfRange = -2,
		// ͣ��
		StopTrans = -3,
		// ���ײ��Ϸ�
		ErrorMatch = -4
	};
};

typedef std::shared_ptr<TaskPriorityBase> T_PTR_TaskPriorityBase;

#endif
