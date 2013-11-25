#include"LSHVectorToFile.h"

//��LSH��д���ļ�
//���룺LSHVector��LSH�㣬ÿ����Ϊһ����������
//filename������ļ�·��
int LSHVectorToFile(vector<vector<float>> LSHVector, string filename)
{
	if(!LSHVector.size())
		return 0;

	ofstream file(filename);

	file << LSHVector[0].size() << endl;

	for(int i=0; i<LSHVector.size(); i++)
	{
		for(int j=0; j<LSHVector[i].size(); j++)
		{
			file << LSHVector[i][j] << " ";
		}
		file << endl;
	}

	file.close();

	return 1;
}


//��LSH����д���ļ�
//���룺IndexLSH��LSH��������¼��LSHVector����ţ���·�����ļ�������ʼλ��
//filename������ļ�·��
int IndexLSHToFile(map<unsigned long , pair<string,short>> IndexLSH, string filename)
{
	if(!IndexLSH.size())
		return 0;

	ofstream file(filename);

	file << "3" << endl;	//ÿ��3����������ţ���·�����ļ�������ʼλ��

	map<unsigned long , pair<string,short>>::iterator it;

	for(it=IndexLSH.begin(); it!=IndexLSH.end(); it++)
	{
		file << it->first << " " << it->second.first << " " <<  it->second.second << endl;
	}

	file.close();

	return 1;
}

//��NLSH����д���ļ�
//���룺IndexLSH��NLSH��������¼��LSHVector����ţ���·�����ļ�������ʼλ�ã�����֡��
//filename������ļ�·��
int IndexLSHNoteToFile(map<unsigned long , pair<string,pair<short,short>>> &IndexLSH, string filename)
{
	if(!IndexLSH.size())
		return 0;

	ofstream file(filename);

	file << "4" << endl;	//ÿ��4����������ţ���·�����ļ�������ʼλ�ã�����֡��

	map<unsigned long , pair<string,pair<short,short>>>::iterator it;

	for(it=IndexLSH.begin(); it!=IndexLSH.end(); it++)
	{
		file << it->first << " " << it->second.first << " " <<  
			it->second.second.first << " " << it->second.second.second << endl;
	}

	file.close();

	return 1;
}