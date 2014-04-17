#ifndef DTW_H
#define DTW_H
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <math.h>
#include <algorithm>
#include <string> 
#include "index.h"
#include "songname.h"
#include "LSHhumming.h"
#include "time.h"
#include "STypes.h"
#include "SMelody.h"
#include "SUtil.h"
#include "SDSP.h"
#include "SModel.h"
#include "STester.h"
#include <windows.h>
#include <process.h>    /* _beginthread, _endthread */
#include <stddef.h>
#include <stdlib.h>
#include <conio.h>
#include "LSHVectorToFile.h"
#include "LSHCandidateCorrect.h"
#define MyMinTwo(x,y) ((x)<(y)?(x):(y))
using namespace std;


//typedef struct 
//{ 
//	char wavename[300];
//	map<string , vector<float>> indexSongName;
//	PRNearNeighborStructT IndexHuming;
//	map<unsigned long , pair<string,short>> IndexLSH;
//	map<unsigned long , pair<string,pair<short,short>>> IndexLSHNote;
//	int stepFactor;
//	IntT RetainNum;
//	IntT LSHFilterNum;
//	float stepRatio;
//}ParamInfo, *pParamInfo;

void NoZero(vector<float>  &x);	//ȥ��

//��ֵ
void MinusMean(vector<float> &x);	//����ֵ
void MinusMeanWithThd( vector<float>  &x, int thd);	//����ֵ����������thd������������ǰthd��Ԫ�صľ�ֵ
float MinusMeanSmooth(vector<float> &x);	//����ֵ����������ֵ��>12��-12��<-12��+12�����ؾ�ֵ
float Mean(vector<float>::iterator Ybegin, vector<float>::iterator Yend);
float MeanLSH(vector<float> &x);	//�����ֵ
void MeanNoteLSH(vector<float> &x, float mean);	//����ֵ
void Mean8MinutesInt(vector< vector<double> > &x);
float MeanPlus( vector<float>  &x, float plus);
void Mean(vector< vector<float> > &x);	//��ֵ��
void MeanOld(vector< vector<double> > &x);
void MeanInt(vector< vector<double> > &x);

void Dimention20LSHToToneInteger(vector<double> &x);
double DTWBeginAndRA(vector<vector<double>> &D);
double DTWBeginNewLSH(vector<vector<double>> &D);
void ZeroToForward(vector< vector<double> > &x);
void OneFileToMultiFile(string fileName,int ThreadNum);
int realPitchToToneShengda(vector <float> &queryPitch);	//��������ת��Ϊ������������
double PitchDistance(vector<vector<double>> &a, vector<vector<double>> &b);
double StringMatchToDisMapRALSHNewPairVariance(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int recurse,int pairNum);
double StringMatchToDisMapRALSHNew(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int recurse);//�̶����ȵ�RA�㷨
double StretchRAThree(vector< vector<double> > &queryX, vector< vector<double> > &dataY);
int WavToResult8Minutes(char *wavename,map<string , vector<vector<double>>> &indexSongName );
int readIndexPitch8Minutes(string dataIndex,map<string , vector<vector<double>>> &indexSongName);
double StringMatchToDisMyRA(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int recurse);//�Լ���RA
void StringToStringMean(vector< vector<double> > &queryX, vector< vector<double> > &dataY,double stretch);
double StringMatchToDisAndRA(vector< vector<double> > &queryX, vector< vector<double> > &dataY);//�����������ؿ��ж�Ӧ�ĳ���
int StringMatchToSizeY(vector< vector<double> > &queryX, vector< vector<double> > &dataY);//�����������ؿ��ж�Ӧ�ĳ���
double StretchFiveLength(vector< vector<double> > &queryX, vector< vector<double> > &dataY);
double DTWBeginNew(vector<vector<double>> &D);//��ͷ��ʼƥ��
int WavToResultTowToOne(char *wavename,map<string , vector<vector<double>>> &indexSongName );
double StretchMyRA(vector< vector<double> > &queryX, vector< vector<double> > &dataY);
double StringMatchToDisMap(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int recurse);//RA�㷨
double StretchRA(vector< vector<double> > &queryX, vector< vector<double> > &dataY);
double DTWBeginQueryAndSongSeven(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D);//�ݹ��������ܺ���
double DTWdisRecur(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D,double disMid,double num,double bound);//�ݹ�����̾���
void MeanTowToOne(vector< vector<double> > &x);//�����㻯Ϊһ����
int ReadToneToResultCre(char *wavename,map<string , vector<vector<double>>> &indexSongName );//���׷���������
int WavCepstrumToResult(char *wavename,map<string , vector<vector<double>>> &indexSongName );//���׷���
float StringMatch( vector<float>  &queryX,  vector<float>  &dataY);//vectorƥ�������
double StringMatchUltimate(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int &ultimateNum);//vectorƥ�������
void Var(vector< vector<double> > &x);
bool CompareLastName(const pair<int,string> &p1,const pair<int,string> &p2);
double LinearToDisUltimate(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int &ultimateNum);
int WavToResult8MinutesLinearAndRA (char *wavename,map<string , vector<vector<double>>> &indexSongName );
float StringMatchToDisMapRALSHNewPairVariancePositionVariance(vector< vector<float> > &queryX, vector< vector<float> > &dataY,int recurse,
										   int pairNum,double MidPercentage,int &ultimateNum);
float LinearToDis( vector<float>  &queryX,  vector<float>  &dataY);//���������㷨
double StringMatchToDisQueryConst(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int recurse);
double StringMatchToDis(vector< vector<double> > &queryX, vector< vector<double> > &dataY);//�������������
void StringToString( vector<float>  &queryX,  vector<float>  &dataY,float stretch);	//��������
double DTWBeginQueryAndSongSevenNo(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D);//�������²�����7��DTW�����õݹ�
int realPitchToTone(vector <double> &queryPitch);//Ƶ�ʵ�����
int WavToResultFive(char *wavename,map<string , vector<vector<double>>> &indexSongName );
int WavToResult8MinutesLinearAndDTW(char *wavename,map<string , vector<vector<double>>> &indexSongName );
int PitchToTone(vector <vector <double>> &queryPitch);//Ƶ�ʶ�Ӧ�ĵ㵽����
double DTWbegin(vector<vector<double>> &D);//��ͷ��ʼƥ��

double DTWBeginRecurseLinear(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D);
int WavToResultStretchAndDTW(char *wavename,map<string , vector<vector<double>>> &indexSongName );
void Var8Minutes(vector< vector<double> > &x);
int ToneToResultStretch(char *wavename,map<string , vector<vector<double>>> &indexSongName );
double StringMatchToDisMyRANewPair(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int recurse);
double DTWtotalOrig(vector<vector<double>> &D);

double Scale(vector< vector<double> > &x);

int WavToResult8MinutesInt(char *wavename,map<string , vector<vector<double>>> &indexSongName );
double StringMatchToDisMapMidMatch(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int recurse);
double StretchThree(vector< vector<double> > &queryX, vector< vector<double> > &dataY);//����������5������������
double StringMatchToDis(vector< vector<double> > &queryX, vector< vector<double> > &dataY);
double DTWBeginQueryAndSongFive(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D);//DTW��5������5��DTW��
double DTWtotalPlusPe(vector<vector<double>> &D);
int WavToResultStretch(char *wavename,map<string , vector<vector<double>>> &indexSongName );//��wav����������
double DTWtotal(vector<vector<double>> &D);//���ⲿ�ֿ�ʼƥ��
double DTW(vector<vector<double>> &D);//ԭʼƥ��
double StretchRAOne(vector< vector<double> > &queryX, vector< vector<double> > &dataY);
int DistanceMatrix(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D);//��������
double MyDistance(vector<double> &a, vector<double> &b);//�����
double MyMin(double a, double b, double c);	//����Сֵ
void readinstance(char *wavename,vector <vector <double>> &queryPitch);//������
void discre(vector <vector <double>> &x);//��������
void Zerodiscre(vector <vector <double>> &x);//��ȥ����������
int WavToResult(char *wavename,map<string , vector<vector<double>>> &indexSongName );//��WAV��ƥ��
int DatawavToPitch(char *wavename);//ת����WAV����Ƶ�ļ������������
int ToneTorealPitch(vector <vector <double>> &queryPitch);//��������Ƶ��ת��
int ReadToneToResult(char *wavename,map<string , vector<vector<double>>> &indexSongName );//���ļ���ƥ��
int WavToResult8MinutesLSH (char *wavename,map<string , vector<vector<double>>> &indexSongName,PRNearNeighborStructT &IndexHuming,
							map<unsigned long , pair<string,short>> &IndexLSH,int stepFactor,IntT RetainNum);
void ZeroToForwardThreshold(vector< vector<double> > &x , int BeginFrame);
void ZeroToForwardThresholdAndLongZero(vector< vector<double> > &x , int BeginFrame);
void ZeroToForwardThresholdAndLongZeroToHalfBefor(vector< vector<float> > &x , int BeginFrame);
int realPitchToThreeTone(vector <vector <double>> &queryPitch,vector <vector <double>> &queryPitchTow,vector <vector <double>> &queryPitchThree);
int realPitchToAnotherTowTone(vector <vector <double>> &queryPitch,vector <vector <double>> &queryPitchTow,vector <vector <double>> &queryPitchThree);
int WavToResult8MinutesThreeMatrix (char *wavename,map<string , vector<vector<double>>> &indexSongName );

vector<pair<int,int>> DuplicateSegmentBegin(vector<float> & tone);	//����������tone����λ��ƥ�俪ͷ����ƥ�䳤�ȴ���100֡����¼��λ�úͳ���֡��������
float LinearToDisIter( vector<float>::iterator  Xbegin,  vector<float>::iterator  Xend,
					   vector<float>::iterator  Ybegin,  vector<float>::iterator  Yend);

double RAPositionVarianceOptimal( vector<double>  &queryX,  vector<double>  &dataY,
								 int recurse,int pairNum,double MidPercentage,int &ultimateNum);

//����
void PRNearNeighborStructTCopy(PRNearNeighborStructT des,PRNearNeighborStructT src);	//RNN���ݽṹ����
void FloatCopyToVector(vector <float> &des, float *src, int len);	//��float���鸴�Ƶ�vector��

#endif