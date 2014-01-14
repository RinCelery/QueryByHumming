#include"LSHVectorToFile.h"

//��LSH��д���ļ�
//���룺LSHVector��LSH�㣬ÿ����Ϊһ����������
//filename������ļ�·��
int LSHVectorToFile(vector<vector<float>> &LSHVector, string filename)
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
int IndexLSHToFile(map<unsigned long , pair<string,short>> &IndexLSH, string filename)
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

//��LS���LSH��д���ļ�
//���룺LSHVectorLS��LS���LSH�㣬ÿ����Ϊһ���������У�vector[i][j][k]��ʾ��i�����������µĵ�j��������ĵ�k������
//filename������ļ�·��������ļ��У�ÿ��һ��LSH��
int LSHVectorLSToFile(vector<vector<vector<float>>> &LSHVectorLS, string filename)
{
	if(!LSHVectorLS.size())
		return 0;

	ofstream file;
	file.open(filename.c_str(),ios::app);	//�����Զ���ļ��Ĳ�ѯ����Ҫ׷�Ӵ�

	//file << LSHVectorLS[0][0].size() << endl;

	for(int i=0; i<LSHVectorLS.size(); i++)
	{
		for(int j=0; j<LSHVectorLS[i].size(); j++)
		{
			for(int k=0; k<LSHVectorLS[i][j].size(); k++)
				file << LSHVectorLS[i][j][k] << " ";
			file << endl;
		}
	}

	file.close();

	return 1;
}

//��LS���LSH����д���ļ�
//���룺LSHVectorLS��LS���LSH�㣬ÿ����Ϊһ���������У�vector[i][j][k]��ʾ��i�����������µĵ�j��������ĵ�k������
//filename������ļ�·��������ļ��У�ÿ��Ϊ����LSH���Ӧ��������Ŀǰʹ��LSH���������ļ�����
int IndexLSHLSToFile(vector<vector<vector<float>>> &LSHVectorLS, string wavname, string filename)
{
	if(!LSHVectorLS.size())
		return 0;

	ofstream file;
	file.open(filename.c_str(),ios::app);	//�����Զ���ļ��Ĳ�ѯ����Ҫ׷�Ӵ�

	//file << LSHVectorLS[0][0].size() << endl;

	for(int i=0; i<LSHVectorLS.size(); i++)
	{
		for(int j=0; j<LSHVectorLS[i].size(); j++)
		{
			file << wavname << endl;
		}
	}

	file.close();

	return 1;
}