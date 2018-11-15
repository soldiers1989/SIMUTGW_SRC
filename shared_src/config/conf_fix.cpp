#include "config/conf_fix.h"

namespace SzStepOrderFieldName
{	
	std::map<std::string, std::string> mapNumber2String;
	std::map<std::string, std::string> mapString2Number;
}

int SzStepOrderFieldName::InitMap()
{
	//��ʼ������
	//
	SzStepOrderFieldName::mapNumber2String["8"] = "beginstring";

	//
	SzStepOrderFieldName::mapNumber2String["9"] = "bodylength";

	//
	SzStepOrderFieldName::mapNumber2String["10"] = "checksum";

	//�ͻ��������
	SzStepOrderFieldName::mapNumber2String["11"] = "clordid";

	//
	SzStepOrderFieldName::mapNumber2String["14"] = "cumqty";

	//�����������ִ�б�ţ������������ڲ��ظ� 17
	SzStepOrderFieldName::mapNumber2String["17"] = "execid";

	//֤ȯ����Դ102=����֤ȯ������
	SzStepOrderFieldName::mapNumber2String["22"] = "securityidsource";

	//
	SzStepOrderFieldName::mapNumber2String["34"] = "msgseqnum";

	//
	SzStepOrderFieldName::mapNumber2String["35"] = "msgtype";

	//
	SzStepOrderFieldName::mapNumber2String["36"] = "newseqno";

	//
	SzStepOrderFieldName::mapNumber2String["37"] = "orderid";

	//��������
	SzStepOrderFieldName::mapNumber2String["38"] = "orderqty";

	//
	SzStepOrderFieldName::mapNumber2String["39"] = "ordstatus";

	//��������1=�м�,2=�޼�,U=��������
	SzStepOrderFieldName::mapNumber2String["40"] = "ordtype";

	//ԭʼ�����Ŀͻ��������
	SzStepOrderFieldName::mapNumber2String["41"] = "origclordid";

	//ָʾ����Ϣ��ŵ���Ϣ�����ظ����ͣ�ȡֵ��=Y: �����ظ�,=N: �״η���
	SzStepOrderFieldName::mapNumber2String["43"] = "possdupflag";

	//�۸�OrdType Ϊ 2 ���м�ί��ʱ��д
	SzStepOrderFieldName::mapNumber2String["44"] = "price";

	//֤ȯ����
	SzStepOrderFieldName::mapNumber2String["48"] = "securityid";

	//
	SzStepOrderFieldName::mapNumber2String["49"] = "sendercompid";

	//
	SzStepOrderFieldName::mapNumber2String["52"] = "sendingtime";

	//��������, 1=��2=��
	SzStepOrderFieldName::mapNumber2String["54"] = "side";

	//
	SzStepOrderFieldName::mapNumber2String["56"] = "targetcompid";

	//
	SzStepOrderFieldName::mapNumber2String["58"] = "text";

	//��Чʱ������0=������Ч,3=��ʱ�ɽ���ȡ��,9=�۹�ͨ�����޼���,ȱʡֵΪ 0
	SzStepOrderFieldName::mapNumber2String["59"] = "timeinforce";
	//��������ʱ��
	SzStepOrderFieldName::mapNumber2String["60"] = "transacttime";

	//��ƽ�ֱ�ʶ������Ʒ������д,O=����,C=ƽ��,ȱʡֵΪ O
	SzStepOrderFieldName::mapNumber2String["77"] = "positioneffect";

	//
	SzStepOrderFieldName::mapNumber2String["95"] = "rawdatalength";

	//
	SzStepOrderFieldName::mapNumber2String["96"] = "rawdata";

	//
	SzStepOrderFieldName::mapNumber2String["98"] = "encryptmethod";

	//ֹ��ۣ�Ԥ��
	SzStepOrderFieldName::mapNumber2String["99"] = "stoppx";

	//�ܾ�ԭ�����
	SzStepOrderFieldName::mapNumber2String["102"] = "cxlrejreason";

	//
	SzStepOrderFieldName::mapNumber2String["103"] = "ordrejreason";

	//
	SzStepOrderFieldName::mapNumber2String["108"] = "heartbtint";

	//��ͳɽ�����,ȱʡֵΪ 0
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

	//�������
	SzStepOrderFieldName::mapNumber2String["152"] = "cashorderqty";

	//���ұ�ǩ������Ʒ������д,0=Covered����ʾ���Ҳ�,1=UnCovered����ʾ��ͨ��,ȱʡֵΪ 1
	SzStepOrderFieldName::mapNumber2String["203"] = "coveredoruncovered";

	//
	SzStepOrderFieldName::mapNumber2String["383"] = "maxmessagesize";

	//�����˴������
	/*
	448			447				452
	֤ȯ�˻� 5=�й�Ͷ���߱�� 5=Ͷ����֤ȯ�˻�
	���׵�Ԫ�� PBU�� C=ͨ���г������߱�ʶ 1=�걨���׵�Ԫ
	Ӫҵ������ D=�Զ������ 4001=Ӫҵ����
	*/
	SzStepOrderFieldName::mapNumber2String["453"] = "nopartyids";

	//�����˴���Դ
	SzStepOrderFieldName::mapNumber2String["447"] = "partyidsource";

	//�����˴���
	SzStepOrderFieldName::mapNumber2String["448"] = "partyid";

	//�����˴����ɫ
	SzStepOrderFieldName::mapNumber2String["452"] = "partyrole";

	//
	SzStepOrderFieldName::mapNumber2String["464"] = "testmessageindicator";

	//��������������
	SzStepOrderFieldName::mapNumber2String["522"] = "ownertype";

	//�����޶�
	SzStepOrderFieldName::mapNumber2String["529"] = "orderrestrictions";

	//������ȯ���ñ�ʶ,1=Cash����ͨ����,2=Open��������ȯ����,3=Close��������ȯƽ��,ȱʡֵΪ 1
	SzStepOrderFieldName::mapNumber2String["544"] = "cashmargin";

	//
	SzStepOrderFieldName::mapNumber2String["553"] = "username";

	//
	SzStepOrderFieldName::mapNumber2String["554"] = "password";

	//Լ����,Э�齻�׵���ɽ���д
	SzStepOrderFieldName::mapNumber2String["664"] = "confirmid";

	//
	SzStepOrderFieldName::mapNumber2String["789"] = "nextexpectedmsgseqnum";

	//���ɽ���λ��,0 ��ʾ�����Ƴɽ���λ��,ȱʡֵΪ 0
	SzStepOrderFieldName::mapNumber2String["1090"] = "maxpricelevels";

	//Ӧ�ñ�ʶ
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

	//�ͻ��������
	SzStepOrderFieldName::mapString2Number["clordid"] = "11";

	//cumqty
	SzStepOrderFieldName::mapString2Number["cumqty"] = "14";

	//�ͻ��������
	SzStepOrderFieldName::mapString2Number["execid"] = "17";

	//֤ȯ����Դ102=����֤ȯ������
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

	//��������
	SzStepOrderFieldName::mapString2Number["orderqty"] = "38";

	//
	SzStepOrderFieldName::mapString2Number["ordstatus"] = "39";

	//��������1=�м�,2=�޼�,U=��������
	SzStepOrderFieldName::mapString2Number["ordtype"] = "40";

	//ԭʼ�����Ŀͻ��������
	SzStepOrderFieldName::mapString2Number["origclordid"] = "41";

	//
	SzStepOrderFieldName::mapString2Number["possdupflag"] = "43";

	//�۸�OrdType Ϊ 2 ʱ��д
	SzStepOrderFieldName::mapString2Number["price"] = "44";

	//֤ȯ����
	SzStepOrderFieldName::mapString2Number["securityid"] = "48";

	//
	SzStepOrderFieldName::mapString2Number["sendercompid"] = "49";

	//
	SzStepOrderFieldName::mapString2Number["sendingtime"] = "52";

	//��������, 1=��2=��
	SzStepOrderFieldName::mapString2Number["side"] = "54";

	//
	SzStepOrderFieldName::mapString2Number["targetcompid"] = "56";

	//
	SzStepOrderFieldName::mapString2Number["text"] = "58";

	//��Чʱ������0=������Ч,3=��ʱ�ɽ���ȡ��,9=�۹�ͨ�����޼���,ȱʡֵΪ 0
	SzStepOrderFieldName::mapString2Number["timeinforce"] = "59";
	//��������ʱ��
	SzStepOrderFieldName::mapString2Number["transacttime"] = "60";

	//��ƽ�ֱ�ʶ������Ʒ������д,O=����,C=ƽ��,ȱʡֵΪ O
	SzStepOrderFieldName::mapString2Number["positioneffect"] = "77";

	//
	SzStepOrderFieldName::mapString2Number["rawdatalength"] = "95";

	//
	SzStepOrderFieldName::mapString2Number["rawdata"] = "96";

	//
	SzStepOrderFieldName::mapString2Number["encryptmethod"] = "98";

	//ֹ��ۣ�Ԥ��
	SzStepOrderFieldName::mapString2Number["stoppx"] = "99";

	//�ܾ�ԭ�����
	SzStepOrderFieldName::mapString2Number["cxlrejreason"] = "102";

	//ordrejreason
	SzStepOrderFieldName::mapString2Number["ordrejreason"] = "103";

	//
	SzStepOrderFieldName::mapString2Number["heartbtint"] = "108";

	//��ͳɽ�����,ȱʡֵΪ 0
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

	//�������
	SzStepOrderFieldName::mapString2Number["cashorderqty"] = "152";

	//���ұ�ǩ������Ʒ������д,0=Covered����ʾ���Ҳ�,1=UnCovered����ʾ��ͨ��,ȱʡֵΪ 1
	SzStepOrderFieldName::mapString2Number["coveredoruncovered"] = "203";

	//
	SzStepOrderFieldName::mapString2Number["underlyingsecurityidsource"] = "305";

	//
	SzStepOrderFieldName::mapString2Number["underlyingsecurityid"] = "309";

	//
	SzStepOrderFieldName::mapString2Number["maxmessagesize"] = "383";

	//�����˴������
	/*
	448			447				452
	֤ȯ�˻� 5=�й�Ͷ���߱�� 5=Ͷ����֤ȯ�˻�
	���׵�Ԫ�� PBU�� C=ͨ���г������߱�ʶ 1=�걨���׵�Ԫ
	Ӫҵ������ D=�Զ������ 4001=Ӫҵ����
	*/
	SzStepOrderFieldName::mapString2Number["nopartyids"] = "453";

	//�����˴���Դ
	SzStepOrderFieldName::mapString2Number["partyidsource"] = "447";

	//�����˴���
	SzStepOrderFieldName::mapString2Number["partyid"] = "448";

	//�����˴����ɫ
	SzStepOrderFieldName::mapString2Number["partyrole"] = "452";

	//
	SzStepOrderFieldName::mapString2Number["testmessageindicator"] = "464";

	//��������������
	SzStepOrderFieldName::mapString2Number["ownertype"] = "522";

	//�����޶�
	SzStepOrderFieldName::mapString2Number["orderrestrictions"] = "523";

	//������ȯ���ñ�ʶ,1=Cash����ͨ����,2=Open��������ȯ����,3=Close��������ȯƽ��,ȱʡֵΪ 1
	SzStepOrderFieldName::mapString2Number["cashmargin"] = "544";

	//
	SzStepOrderFieldName::mapString2Number["username"] = "553";

	//
	SzStepOrderFieldName::mapString2Number["password"] = "554";

	//Լ����,Э�齻�׵���ɽ���д
	SzStepOrderFieldName::mapString2Number["confirmid"] = "664";

	//
	SzStepOrderFieldName::mapString2Number["nextexpectedmsgseqnum"] = "789";

	//���ɽ���λ��,0 ��ʾ�����Ƴɽ���λ��,ȱʡֵΪ 0
	SzStepOrderFieldName::mapString2Number["maxpricelevels"] = "1090";

	//Ӧ�ñ�ʶ
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
