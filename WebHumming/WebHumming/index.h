#ifndef INDEX_H
#define INDEX_H
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <math.h>
#include <algorithm>
#include <string> 
#include "pitch.h"
#include "DTW.h"
#include "define.h"
#include "LSHhumming.h"
using namespace std;

PPointT readPointVector(vector<float> &VectorTone);	//����LSH�㵽PPoint�У�ͳ��ƽ����
int LSHFilter(char *wavename,map<string ,int> &LSHFilterMap,vector<IntT> &CandidatesFilter,
			  map<unsigned long , pair<string,short>> &IndexLSH);
void VectorSmoothToHalf(vector <float> &queryPitch);	//��֡��Ϊһ֡

//��LSH���������ж����ݼ�������dataSet�У�����ÿ��LSH��ʱ��¼��ź�LSH���ƽ����
PPointT * readDataSetFromVector(vector<vector<float>> &LSHVector);

//����pv�ļ�������ά�ģ�����������ʱ�䣩����ת��Ϊһά����
int readIndexPitch8MinutesNewPv(string dataIndex,map<string , vector<float>> &indexSongName);

void smooth(vector <float> &queryPitch);	//�������ֵ��ƽ��������Ϊ5������Ϊ1
float MiddleFive(float a, float b, float c,float d,float e);	//�����ֵ

int charToVector(char * wavename,vector <float> &queryPitch);//����Ƶchar��vectorת��
int readIndexPitch(string dataIndex,map<string , vector<vector<double>>> &indexSongName);//��������Ƶ��ÿһ����һ����Ƶ
int readindex(string dataIndex,map<string , vector<vector<double>>> &indexSongName);//��������Ƶ������Ϊ�Ļ�Ƶ�ļ�

//��һά�������г�ȡLSH��
int IndexPitchToLSHVector(map<string , vector<float>> &indexSongName, 
	int StepFactor, int LSHsize, int LSHshift, int maxFrame, 
	vector<vector<float>> &LSHVector, map<unsigned long , pair<string,short>> &IndexLSH);

//��һά�������г�ȡNLSH��
int IndexPitchToLSHVectorNote(map<string , vector<float>> &indexSongName, 
	int noteMaxFrame, int NLSHsize, int maxFrame, 
	vector<vector<float>> &LSHVector, map<unsigned long , pair<string,pair<short,short>>> &IndexLSH);

int QueryPitchToLSHVectorLinearStretchingShortToMore(vector <float> &queryPitch,vector <vector<vector<float>>> &LSHQueryVectorLinearStretching,
													 float FloorLevel, float UpperLimit,int stepFactor,float stepRatio,float StretchStep,int recur);

int LSHresult(char *wavename,int sizeQuery,int stepFactor,float stretch,vector<vector<IntT>> &IndexCandidatesStretch,
			  map<unsigned long ,pair<string,short>> &IndexLSH,float StretchStep);

int IndexSignToQueryAndDataVectorHummingBeginQueryLengthFixedAndNoClearMatchLeast(vector <float> &DisCandidates,vector<IntT> &IndexCandidatesStretch,vector<IntT> &CandidatesNumStretch,float stretchFactor,
																				  map<unsigned long , pair<string,short>> &IndexLSH,int stepFactor,vector <float> queryPitch,
																				   vector<float>  &queryX, vector< vector<float>  >&dataY,
																				  map<string , vector<float>> &indexSongName,vector <string> &SongNameMapToDataY,
																				  int &CandidatesSizeInDWT,int MatchBeginPos,map <string , short > &SongMapPosition,
																				  vector<float>  &CandidatesDataYDis);
int IndexSignToQueryAndDataVectorHummingMatchLeastALL(vector<IntT> &IndexCandidatesStretch,vector<IntT> &CandidatesNumStretch,float stretchFactor,
													  map<unsigned long , pair<string,short>> &IndexLSH,int stepFactor,vector <float> queryPitch,
													  vector<float>  &queryX,vector<  vector<float>  >&dataY,
													  map<string , vector<float>> &indexSongName,vector <string> &SongNameMapToDataY,
													  int &CandidatesSizeInDWT,int MatchBeginPos,map <string , short > &SongMapPosition,
													  map <string , vector<pair<short, double>> > &SongMapPositionAll,int offsetbegin, int offsetLength);

int QueryPitchToLSHVectorLinearStretchingShortToMoreNote(vector<pair<short, short>>& posPairvector, vector <float> &queryPitch,vector <vector <vector<float>>> &LSHQueryVectorLinearStretching,
														 float FloorLevel, float UpperLimit,int stepFactor,float stepRatio,float StretchStep);
int IndexSignToQueryAndDataVectorHummingMatchLeastALLNote(vector<pair<short, short>>& posPair,vector<IntT> &IndexCandidatesStretch,vector<IntT> &CandidatesNumStretch,float stretchFactor,
														  map<unsigned long , pair<string,pair<short,short>>> &IndexLSH,int stepFactor,vector <float> queryPitch,
														  vector<float>  &queryX, vector< vector<float> > &dataY,
														  map<string , vector<float>> &indexSongName,vector <string> &SongNameMapToDataY,
														  int &CandidatesSizeInDWT,int MatchBeginPos,map <string , short > &SongMapPosition,
														  map <string , vector<pair<short, double>> >  &SongMapPositionAll,int offsetbegin, int offsetLength);
int LSHresultRate(char *wavename,int sizeQuery,int stepFactor,float stretch,vector<vector<IntT>> &IndexCandidatesStretch,
				  map<unsigned long , pair<string,short>> &IndexLSH,map<unsigned long , pair<string,pair<short,short>>> &IndexLSHNote,
				  float StretchStep,vector<vector<float>> &IndexCandidatesDis);

//��ȡһά�������еĵ�һ��NLSH��
int QueryPitchToLSHVectorLinearStretchingShortToMoreNoteFirst(vector<pair<short, short>>& posPairvector, 
	vector <float> &queryPitch,vector <vector <vector<float>>> &LSHQueryVectorLinearStretching,
	int noteMinFrame,int noteMaxFrame,int NLSHsize);

#endif