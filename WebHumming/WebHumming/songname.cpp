#include "songname.h"
#include "lib.h"

//���ļ��������ID�͸���ӳ���
//�ļ�ÿ��Ϊ��ID ����
void songread(string IDAndNameFile, map <string ,string> &songIDAndName)
{
	ifstream file(IDAndNameFile);
	string line;
	while (getline(file,line))
	{
		int pos = line.find_first_of(' ');
		if(pos != string::npos)
		{
			string name(line,0,pos);
			line.erase(0,pos+1);
			songIDAndName.insert(make_pair(name,line));
		}
	}
}