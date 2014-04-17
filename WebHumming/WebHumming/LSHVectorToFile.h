#ifndef LSHVECTORTOFILE_H
#define LSHVECTORTOFILE_H

#include<fstream>
#include<iostream>
#include<vector>
#include<map>
#include<cstring>
#include<string>
using namespace std;

//��LSH���NLSH��д���ļ�
int LSHVectorToFile(vector<vector<float>> &LSHVector, string filename);
//��LSH����д���ļ�����һ������ļ�����ͳ��ÿ�������ļ���������
int IndexLSHToFile(map<unsigned long , pair<string,short>> &IndexLSH, string filename, string cfilename);
//��NLSH����д���ļ�����һ������ļ�����ͳ��ÿ�������ļ���������
int IndexLSHNoteToFile(map<unsigned long , pair<string,pair<short,short>>> &IndexLSH, string filename, string cfilename);
//��LS���LSH��д���ļ�
int LSHVectorLSToFile(vector<vector<vector<float>>> &LSHVectorLS, string filename);
//��LS���LSH����д���ļ�����һ������ļ�����ͳ��ÿ����ѯ�ļ���������
int IndexLSHLSToFile(vector<vector<vector<float>>> &LSHVectorLS, string wavname, string filename, string cfilename);

#endif