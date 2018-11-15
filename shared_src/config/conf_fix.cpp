#include "config/conf_fix.h"

namespace SzStepOrderFieldName
{	
	std::map<std::string, std::string> mapNumber2String;
	std::map<std::string, std::string> mapString2Number;
}

int SzStepOrderFieldName::InitMap()
{
	//初始化配置
	//
	SzStepOrderFieldName::mapNumber2String["8"] = "beginstring";

	//
	SzStepOrderFieldName::mapNumber2String["9"] = "bodylength";

	//
	SzStepOrderFieldName::mapNumber2String["10"] = "checksum";

	//客户订单编号
	SzStepOrderFieldName::mapNumber2String["11"] = "clordid";

	//
	SzStepOrderFieldName::mapNumber2String["14"] = "cumqty";

	//交易所赋予的执行编号，单个交易日内不重复 17
	SzStepOrderFieldName::mapNumber2String["17"] = "execid";

	//证券代码源102=深圳证券交易所
	SzStepOrderFieldName::mapNumber2String["22"] = "securityidsource";

	//
	SzStepOrderFieldName::mapNumber2String["34"] = "msgseqnum";

	//
	SzStepOrderFieldName::mapNumber2String["35"] = "msgtype";

	//
	SzStepOrderFieldName::mapNumber2String["36"] = "newseqno";

	//
	SzStepOrderFieldName::mapNumber2String["37"] = "orderid";

	//订单数量
	SzStepOrderFieldName::mapNumber2String["38"] = "orderqty";

	//
	SzStepOrderFieldName::mapNumber2String["39"] = "ordstatus";

	//订单类型1=市价,2=限价,U=本方最优
	SzStepOrderFieldName::mapNumber2String["40"] = "ordtype";

	//原始订单的客户订单编号
	SzStepOrderFieldName::mapNumber2String["41"] = "origclordid";

	//指示该消息序号的消息可能重复发送，取值：=Y: 可能重复,=N: 首次发送
	SzStepOrderFieldName::mapNumber2String["43"] = "possdupflag";

	//价格OrdType 为 2 即市价委托时填写
	SzStepOrderFieldName::mapNumber2String["44"] = "price";

	//证券代码
	SzStepOrderFieldName::mapNumber2String["48"] = "securityid";

	//
	SzStepOrderFieldName::mapNumber2String["49"] = "sendercompid";

	//
	SzStepOrderFieldName::mapNumber2String["52"] = "sendingtime";

	//买卖方向, 1=买，2=卖
	SzStepOrderFieldName::mapNumber2String["54"] = "side";

	//
	SzStepOrderFieldName::mapNumber2String["56"] = "targetcompid";

	//
	SzStepOrderFieldName::mapNumber2String["58"] = "text";

	//有效时期类型0=当日有效,3=即时成交或取消,9=港股通竞价限价盘,缺省值为 0
	SzStepOrderFieldName::mapNumber2String["59"] = "timeinforce";
	//订单发起时间
	SzStepOrderFieldName::mapNumber2String["60"] = "transacttime";

	//开平仓标识，衍生品交易填写,O=开仓,C=平仓,缺省值为 O
	SzStepOrderFieldName::mapNumber2String["77"] = "positioneffect";

	//
	SzStepOrderFieldName::mapNumber2String["95"] = "rawdatalength";

	//
	SzStepOrderFieldName::mapNumber2String["96"] = "rawdata";

	//
	SzStepOrderFieldName::mapNumber2String["98"] = "encryptmethod";

	//止损价，预留
	SzStepOrderFieldName::mapNumber2String["99"] = "stoppx";

	//拒绝原因代码
	SzStepOrderFieldName::mapNumber2String["102"] = "cxlrejreason";

	//
	SzStepOrderFieldName::mapNumber2String["103"] = "ordrejreason";

	//
	SzStepOrderFieldName::mapNumber2String["108"] = "heartbtint";

	//最低成交数量,缺省值为 0
	SzStepOrderFieldName::mapNumber2String["110"] = "minqty";

	//
	SzStepOrderFieldName::mapNumber2String["122"] = "origsendingtime";

	//
	SzStepOrderFieldName::mapNumber2String["123"] = "gapfillflag";

	//
	SzStepOrderFieldName::mapNumber2String["141"] = "resetseqnumflag";

	//
	SzStepOrderFieldName::mapNumber2String["150"] = "exectype";

	//
	SzStepOrderFieldName::mapNumber2String["151"] = "leavesqty";

	//订单金额
	SzStepOrderFieldName::mapNumber2String["152"] = "cashorderqty";

	//备兑标签，衍生品交易填写,0=Covered，表示备兑仓,1=UnCovered，表示普通仓,缺省值为 1
	SzStepOrderFieldName::mapNumber2String["203"] = "coveredoruncovered";

	//
	SzStepOrderFieldName::mapNumber2String["383"] = "maxmessagesize";

	//参与人代码个数
	/*
	448			447				452
	证券账户 5=中国投资者编号 5=投资者证券账户
	交易单元（ PBU） C=通用市场参与者标识 1=申报交易单元
	营业部代码 D=自定义代码 4001=营业部代
	*/
	SzStepOrderFieldName::mapNumber2String["453"] = "nopartyids";

	//参与人代码源
	SzStepOrderFieldName::mapNumber2String["447"] = "partyidsource";

	//参与人代码
	SzStepOrderFieldName::mapNumber2String["448"] = "partyid";

	//参与人代码角色
	SzStepOrderFieldName::mapNumber2String["452"] = "partyrole";

	//
	SzStepOrderFieldName::mapNumber2String["464"] = "testmessageindicator";

	//订单所有者类型
	SzStepOrderFieldName::mapNumber2String["522"] = "ownertype";

	//订单限定
	SzStepOrderFieldName::mapNumber2String["529"] = "orderrestrictions";

	//融资融券信用标识,1=Cash，普通交易,2=Open，融资融券开仓,3=Close，融资融券平仓,缺省值为 1
	SzStepOrderFieldName::mapNumber2String["544"] = "cashmargin";

	//
	SzStepOrderFieldName::mapNumber2String["553"] = "username";

	//
	SzStepOrderFieldName::mapNumber2String["554"] = "password";

	//约定号,协议交易点击成交填写
	SzStepOrderFieldName::mapNumber2String["664"] = "confirmid";

	//
	SzStepOrderFieldName::mapNumber2String["789"] = "nextexpectedmsgseqnum";

	//最多成交价位数,0 表示不限制成交价位数,缺省值为 0
	SzStepOrderFieldName::mapNumber2String["1090"] = "maxpricelevels";

	//应用标识
	SzStepOrderFieldName::mapNumber2String["1180"] = "applid";

	//
	SzStepOrderFieldName::mapNumber2String["1137"] = "defaultapplverid";

	//
	SzStepOrderFieldName::mapNumber2String["1328"] = "rejecttext";

	//
	SzStepOrderFieldName::mapNumber2String["1408"] = "defaultcstmapplverid";

	//
	SzStepOrderFieldName::mapNumber2String["10179"] = "reportindex";


	// -------------------------------------------------------------------------------

	//
	SzStepOrderFieldName::mapString2Number["beginstring"] = "8";

	//
	SzStepOrderFieldName::mapString2Number["bodylength"] = "9";

	//
	SzStepOrderFieldName::mapString2Number["checksum"] = "10";

	//客户订单编号
	SzStepOrderFieldName::mapString2Number["clordid"] = "11";

	//cumqty
	SzStepOrderFieldName::mapString2Number["cumqty"] = "14";

	//客户订单编号
	SzStepOrderFieldName::mapString2Number["execid"] = "17";

	//证券代码源102=深圳证券交易所
	SzStepOrderFieldName::mapString2Number["securityidsource"] = "22";

	//
	SzStepOrderFieldName::mapString2Number["lastpx"] = "31";

	//
	SzStepOrderFieldName::mapString2Number["lastqty"] = "32";

	//
	SzStepOrderFieldName::mapString2Number["msgseqnum"] = "34";

	//
	SzStepOrderFieldName::mapString2Number["msgtype"] = "35";

	//
	SzStepOrderFieldName::mapString2Number["newseqno"] = "36";

	//
	SzStepOrderFieldName::mapString2Number["orderid"] = "37";

	//订单数量
	SzStepOrderFieldName::mapString2Number["orderqty"] = "38";

	//
	SzStepOrderFieldName::mapString2Number["ordstatus"] = "39";

	//订单类型1=市价,2=限价,U=本方最优
	SzStepOrderFieldName::mapString2Number["ordtype"] = "40";

	//原始订单的客户订单编号
	SzStepOrderFieldName::mapString2Number["origclordid"] = "41";

	//
	SzStepOrderFieldName::mapString2Number["possdupflag"] = "43";

	//价格OrdType 为 2 时填写
	SzStepOrderFieldName::mapString2Number["price"] = "44";

	//证券代码
	SzStepOrderFieldName::mapString2Number["securityid"] = "48";

	//
	SzStepOrderFieldName::mapString2Number["sendercompid"] = "49";

	//
	SzStepOrderFieldName::mapString2Number["sendingtime"] = "52";

	//买卖方向, 1=买，2=卖
	SzStepOrderFieldName::mapString2Number["side"] = "54";

	//
	SzStepOrderFieldName::mapString2Number["targetcompid"] = "56";

	//
	SzStepOrderFieldName::mapString2Number["text"] = "58";

	//有效时期类型0=当日有效,3=即时成交或取消,9=港股通竞价限价盘,缺省值为 0
	SzStepOrderFieldName::mapString2Number["timeinforce"] = "59";
	//订单发起时间
	SzStepOrderFieldName::mapString2Number["transacttime"] = "60";

	//开平仓标识，衍生品交易填写,O=开仓,C=平仓,缺省值为 O
	SzStepOrderFieldName::mapString2Number["positioneffect"] = "77";

	//
	SzStepOrderFieldName::mapString2Number["rawdatalength"] = "95";

	//
	SzStepOrderFieldName::mapString2Number["rawdata"] = "96";

	//
	SzStepOrderFieldName::mapString2Number["encryptmethod"] = "98";

	//止损价，预留
	SzStepOrderFieldName::mapString2Number["stoppx"] = "99";

	//拒绝原因代码
	SzStepOrderFieldName::mapString2Number["cxlrejreason"] = "102";

	//ordrejreason
	SzStepOrderFieldName::mapString2Number["ordrejreason"] = "103";

	//
	SzStepOrderFieldName::mapString2Number["heartbtint"] = "108";

	//最低成交数量,缺省值为 0
	SzStepOrderFieldName::mapString2Number["minqty"] = "110";

	//
	SzStepOrderFieldName::mapString2Number["origsendingtime"] = "122";

	//
	SzStepOrderFieldName::mapString2Number["gapfillflag"] = "123";

	//
	SzStepOrderFieldName::mapString2Number["resetseqnumflag"] = "141";

	//
	SzStepOrderFieldName::mapString2Number["exectype"] = "150";

	//leavesqty
	SzStepOrderFieldName::mapString2Number["leavesqty"] = "151";

	//订单金额
	SzStepOrderFieldName::mapString2Number["cashorderqty"] = "152";

	//备兑标签，衍生品交易填写,0=Covered，表示备兑仓,1=UnCovered，表示普通仓,缺省值为 1
	SzStepOrderFieldName::mapString2Number["coveredoruncovered"] = "203";

	//
	SzStepOrderFieldName::mapString2Number["underlyingsecurityidsource"] = "305";

	//
	SzStepOrderFieldName::mapString2Number["underlyingsecurityid"] = "309";

	//
	SzStepOrderFieldName::mapString2Number["maxmessagesize"] = "383";

	//参与人代码个数
	/*
	448			447				452
	证券账户 5=中国投资者编号 5=投资者证券账户
	交易单元（ PBU） C=通用市场参与者标识 1=申报交易单元
	营业部代码 D=自定义代码 4001=营业部代
	*/
	SzStepOrderFieldName::mapString2Number["nopartyids"] = "453";

	//参与人代码源
	SzStepOrderFieldName::mapString2Number["partyidsource"] = "447";

	//参与人代码
	SzStepOrderFieldName::mapString2Number["partyid"] = "448";

	//参与人代码角色
	SzStepOrderFieldName::mapString2Number["partyrole"] = "452";

	//
	SzStepOrderFieldName::mapString2Number["testmessageindicator"] = "464";

	//订单所有者类型
	SzStepOrderFieldName::mapString2Number["ownertype"] = "522";

	//订单限定
	SzStepOrderFieldName::mapString2Number["orderrestrictions"] = "523";

	//融资融券信用标识,1=Cash，普通交易,2=Open，融资融券开仓,3=Close，融资融券平仓,缺省值为 1
	SzStepOrderFieldName::mapString2Number["cashmargin"] = "544";

	//
	SzStepOrderFieldName::mapString2Number["username"] = "553";

	//
	SzStepOrderFieldName::mapString2Number["password"] = "554";

	//约定号,协议交易点击成交填写
	SzStepOrderFieldName::mapString2Number["confirmid"] = "664";

	//
	SzStepOrderFieldName::mapString2Number["nextexpectedmsgseqnum"] = "789";

	//最多成交价位数,0 表示不限制成交价位数,缺省值为 0
	SzStepOrderFieldName::mapString2Number["maxpricelevels"] = "1090";

	//应用标识
	SzStepOrderFieldName::mapString2Number["applid"] = "1180";

	//
	SzStepOrderFieldName::mapString2Number["defaultapplverid"] = "1137";

	//
	SzStepOrderFieldName::mapString2Number["rejecttext"] = "1328";

	//
	SzStepOrderFieldName::mapString2Number["defaultcstmapplverid"] = "1408";

	//
	SzStepOrderFieldName::mapString2Number["nosecurity"] = "8902";

	//
	SzStepOrderFieldName::mapString2Number["deliveryqty"] = "8903";

	//
	SzStepOrderFieldName::mapString2Number["substcash"] = "8904";

	//
	SzStepOrderFieldName::mapString2Number["reportindex"] = "10179";

	return 0;
}
