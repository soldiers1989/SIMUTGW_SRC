#ifndef __FILE_OPER_HELPER_H__
#define __FILE_OPER_HELPER_H__

#include <string>
#include <vector>

class FileOperHelper
{
public:
	virtual ~FileOperHelper(void);

	/*
	�������·����������ǰĿ¼�ĸ�Ŀ¼�µ�Ŀ¼
	@param const std::string& in_strChildDirPath : �ڵ�ǰĿ¼�ĸ�Ŀ¼�´�����Ŀ¼��
	@param std::string& out_strAbsolutePath : ���صľ���·��

	Return:
	0--�ɹ�
	��0--ʧ��
	*/
	static int MakeDir_InParentPath(const std::string& in_strDirPath, std::string& out_strAbsolutePath);

	/*
		���ݾ���·��������Ŀ¼
		Param:
		statement
		Return:
		0--�ɹ�
		��0--ʧ��
		*/
	static int MakeDir(const std::string& in_strAbsolutePath);

	/*
	�ڵ�ǰĿ¼�ĸ�Ŀ¼�£�����Ŀ¼���Զ��������Ŀ¼
	@param const std::string& in_strDirPath : �ڵ�ǰĿ¼�ĸ�Ŀ¼�´�����Ŀ¼��
	@param std::string& out_strDay : ��ǰ�����ַ���
	@param std::string& out_strAbsolutePath : ���صľ���·��

	Return:
	0--�ɹ�
	��0--ʧ��

	���� ../statement/20170608 Ŀ¼
	*/
	static int MakeDirDate_InParentPath(const std::string& in_strDirPath, std::string& out_strDay, std::string& out_strAbsolutePath);

	/*
	Retrieves the fully qualified path for the file that contains the specified module.

	Return:
	0--�ɹ�
	��0--ʧ��
	*/
	static int GetProgramDir(std::string& out_ProgramPath);

	/*
	�г��ļ����������ļ�����Ƕ������

	@param const std::string& in_strFolder : �ļ���
	@param std::vector<std::string>& out_vectFiles : �ļ��б�

	Return:
	0--�ɹ�
	��0--ʧ��
	*/
	static int ListFilesInPath(const std::string& in_strFolder, std::vector<std::string>& out_vectFiles);

private:
	// ��ֹĬ�Ϲ��캯��
	FileOperHelper(void);
};

#endif