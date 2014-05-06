#include "DTW.h"
#include "emd.h"
#include "lib.h"

//DBN��ѯ����ļ���ÿ��һ����ѯ�����Ϊ��ѡ��źͶ�Ӧ���룬�������С��������
ifstream queryResultFile;
//��ѯ��Ϣ�ļ�����Ϊ��������ļ�Ϊ׷�Ӵ򿪣���дǰ��ɾ��
string query_vector_fname = "QueryLSHLSVector.txt";
string query_index_fname = "QueryLSHLSIndex.txt";
string query_counter_fname = "QueryLSHLSCounter.txt";

const int inf=100000;
const int Dpenalty=0.0;
const int emdLength=23000; //ƥ��5.5�볤�ȵ�EMD���߳��������е�֡������
const double penalty=0;
const double disConst=4.0;
const double disConstString=4.0;//RA�Ĺ̶�����  ԭʼ��16
const double disConstLSHRA=9.0;
map<int,vector<int>> samePoint;
map<string,int> ListSong;
map<string,vector <float>> ResultDis1;
map<string,vector <string>> ResultLable1;
map<string,vector <float>> ResultDis2;
map<string,vector <string>> ResultLable2;
map<string,vector <float>> ResultDis3;
map<string,vector <string>> ResultLable3;
CRITICAL_SECTION g_cs;


float distemd(feature_t *F1, feature_t *F2)
{
	return abs( *F1 - *F2); 
}

double MyMin(double a, double b, double c)
{
	double min;
	if (a<b)
	{
		min=a;
	}
	else
	{
		min=b;
	}
	if (c<min)
	{
		min=c;
	}
	return min;
}

bool sortRule(const pair<float,int>& s1, const pair<float,int>& s2) 
{
  return s1.first < s2.first;
}

double MyDistance(double &a, double &b)
{
	double D=0;
	D+=abs(a-b);

	return D;
}

double PitchDistance(vector<vector<double>> &a, vector<vector<double>> &b)
{
	int i,j,m,n,k;
	m = MyMinTwo(a.size(),b.size());
	if (m>0)
	{
		n=a[0].size();
	}
	else
		n=0;
	double D=0;
	double disPitch=0;
	for (i=0;i<m;i++)
	{
		disPitch=0;
		for (j=0;j<n;j++)
		{
			disPitch+=MyMinTwo(pow(a[i][j]-b[i][j],2),disConstLSHRA);
		}
		D+=disPitch;
	}
	return D;
}


//ȥ�������е���ֵ
void NoZero( vector<float>  &x)
{
	vector <float>::iterator iter=x.begin();
	for (;iter!=x.end();)
	{
		if (*iter==0)
			iter=x.erase(iter);
		else
			iter++;
	}
}


void ZeroToForwardThreshold(vector< vector<double> > &x , int BeginFrame)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	int Nozero=0;
	int vec=0;
	int Threshold=0;//�о�ʲôʱ��ʼ��

	vector <vector <double>>::iterator iter=x.begin();
	for (i=0;i<BeginFrame;i++)
	{
		x.erase(iter);
		iter=x.begin();
	}

	m=x.size();
	if	(m>0)
	{
		if (x[0][0] !=0 )
		{
			Threshold=1;
		}

	}
	for (i=0;i<m-6 && Threshold==0;i++)
	{
		
		x[i].erase(x[i].begin());
		if (x[i+1][0] !=0)
		{
			Threshold=1;
		}
	}

	iter=x.begin();
	for (;iter!=x.end();)
	{
		if (iter->empty())
		{
			x.erase(iter);
			iter=x.begin();
		}
		else
			iter++;
	}
	m=x.size();

	for (i=1;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{			
				;
			}
			else
			{
				x[i][j]=x[i-1][j];
			}
		}
	}
}


void ZeroToForwardThresholdAndLongZero(vector< vector<double> > &x , int BeginFrame)	//ȥ���м��Լ���β�ľ�������
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	int Nozero=0;
	int vec=0;
	int Threshold=0;//�о�ʲôʱ��ʼ��

	vector <vector <double>>::iterator iter=x.begin();
	for (i=0;i<BeginFrame;i++)
	{

		x.erase(iter);
		iter=x.begin();
	}
	m=x.size();
	if	(m>0)
	{
		if (x[0][0] !=0 )
		{
			Threshold=1;
		}

	}
	for (i=0;i<m-6 && Threshold==0;i++)
	{

		x[i].erase(x[i].begin());
		if (x[i+1][0] !=0)
		{
			Threshold=1;
		}
	}
	iter=x.begin();
	for (;iter!=x.end();)
	{
		if (iter->empty())
		{
			x.erase(iter);
			iter=x.begin();
		}
		else
			iter++;

	}
	m=x.size();
	int ZeroSequenceNum=0;
	int LastZero=-10;//�ж���һ��������
	int ZeroBegin=-10;
	bool ExistSilence=FALSE;
	bool NoSilence=FALSE;

	while (!NoSilence)//ȥ���м��Լ���β�ľ�������
	{
		m=x.size();
		ExistSilence=FALSE;
		for (i=0;i<m-1 && ExistSilence==FALSE;i++)
		{
			if (i==m-2)
			{
				NoSilence=TRUE;
			}
			if (x[i][0] ==0 /*|| x[i+1][0] ==0*/)
			{
				if (LastZero+1==i)//�����һ������
				{
					LastZero=i;
					ZeroSequenceNum++;
				}
				else//����ϴβ����㣬�������ţ�����ZeroSequenceNum����
				{
					LastZero=i;
					ZeroBegin=i;//�����￪ʼ�Ǿ���
					ZeroSequenceNum=0;
				}
			}
			if (ZeroSequenceNum>25)
			{
				
				ExistSilence=TRUE;
				for (j=ZeroBegin;j<m-1 && x[j][0] ==0 && j<ZeroBegin+ZeroSequenceNum-15;j++)//ֻɾ��15������
				{

					x[j].erase(x[j].begin());
				}
				LastZero=-10;
				ZeroBegin=-10;
				ZeroSequenceNum=0;
			}
			if (ExistSilence==TRUE)
			{
				iter=x.begin();
				for (;iter!=x.end();)
				{
					if (iter->empty())
					{
						x.erase(iter);
						iter=x.begin();
					}
					else
						iter++;

				}
			}

		}
	}
	
	m=x.size();

	for (i=1;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{			
				;
			}
			else
			{
				x[i][j]=x[i-1][j];
			}
		}
	}
}




void ZeroToForwardThresholdAndLongZeroToHalfBefor( vector<float>  &x , int BeginFrame)//�����м��Լ���β�����㰴������Ϊǰ���������
{
	float mean=0;
	int m=x.size();
	int i,j;
	int Nozero=0;
	int vec=0;
	int Threshold=0;//�о�ʲôʱ��ʼ��

	vector <float>::iterator iter=x.begin();
	for (i=0;i<BeginFrame;i++)
	{

		x.erase(iter);
		iter=x.begin();
	}
	m=x.size();
	if	(m>0)
	{
		if (x[0] !=0 )
		{
			Threshold=1;
		}

	}
	for (i=0;i<m-6 && Threshold==0;i++)
	{
		if (x[i+1] !=0/* && x[i+2][0] !=0*/ )
		{
			Threshold=1;
		}
	}
	iter=x.begin();
	for (;iter!=x.end();)
	{
		if (*iter==0)
		{
			x.erase(iter);
			iter=x.begin();
		}
		else
			iter++;

	}
	m=x.size();
	int ZeroSequenceNum=0;
	int LastZero=-10;//�ж���һ��������
	int ZeroBegin=-10;
	bool ExistSilence=FALSE;
	bool NoSilence=FALSE;

	while (!NoSilence)//ȥ���м��Լ���β�ľ�������
	{
		m=x.size();
		ExistSilence=FALSE;
		for (i=0;i<m-1 && ExistSilence==FALSE;i++)
		{
			if (i==m-2)
			{
				NoSilence=TRUE;
			}
			if (x[i] ==0 /*|| x[i+1][0] ==0*/)
			{
				if (LastZero+1==i)//�����һ������
				{
					LastZero=i;
					ZeroSequenceNum++;
				}
				else//����ϴβ����㣬�������ţ�����ZeroSequenceNum����
				{
					LastZero=i;
					ZeroBegin=i;//�����￪ʼ�Ǿ���
					ZeroSequenceNum=0;
				}
			}
			if (ZeroSequenceNum>25)//����25֡Ϊ������
			{

				ExistSilence=TRUE;
				//for (j=ZeroBegin;j<m-1 && (x[j][0] ==0 || x[j+1][0] ==0);j++)
				for (j=ZeroBegin;j<m-1 && x[j] ==0 && j<ZeroBegin+ZeroSequenceNum-10;j++)//ֻɾ��10������
				{

					x[j]=0;
				}
				LastZero=-10;
				ZeroBegin=-10;
				ZeroSequenceNum=0;
			}
			if (ExistSilence==TRUE)
			{
				iter=x.begin();
				for (;iter!=x.end();)
				{
					if (*iter==0)
					{
						x.erase(iter);
						iter=x.begin();
					}
					else
						iter++;
				}
			}
		}
	}
	ZeroSequenceNum=0;
	LastZero=-10;//�ж���һ��������
	ZeroBegin=-10;
    ExistSilence=FALSE;
	NoSilence=FALSE;
	bool ZeroExist=FALSE;
	while (!NoSilence)//�������ְ�������Ϊǰ��֡���ߺ���֡
	{
		m=x.size();
		ExistSilence=FALSE;
		for (i=0;i<m-1 && ExistSilence==FALSE;i++)
		{
			if (i==m-2)
			{
				NoSilence=TRUE;
			}
			if (x[i] ==0 /*|| x[i+1][0] ==0*/)
			{
				if (LastZero+1==i)//�����һ������
				{
					LastZero=i;
					ZeroSequenceNum++;
				}
				else//����ϴβ����㣬�������ţ�����ZeroSequenceNum����
				{
					LastZero=i;
					ZeroBegin=i;//�����￪ʼ�Ǿ���
					ZeroSequenceNum=0;//�µľ�����ʼ
				}
			}
			if (x[i] ==0 && x[i+1] !=0)
			{
				if (i>ZeroSequenceNum)
				{
					for (int k=i-ZeroSequenceNum;k<=i-ZeroSequenceNum/2;k++)
					{
						x[k]=x[k-1];
					}
					for (int k=i;k>i-ZeroSequenceNum/2;k--)
					{
						x[k]=x[k+1];
					}
				}
				ExistSilence=TRUE;
			}

		}
	}

	m=x.size();
	ZeroSequenceNum=0;
	LastZero=-10;//�ж���һ��������
	ZeroBegin=-10;
	ExistSilence=FALSE;
	NoSilence=FALSE;

	m=x.size();

	for (i=1;i<m;i++)
	{
	
		if(x[i]!=0)
		{			
			;
		}
		else
		{
			x[i]=x[i-1];
		}

	}
}

void ZeroToForward(vector< vector<double> > &x )
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	int Nozero=0;
	int vec=0;
	int Threshold=0;//�о�ʲôʱ��ʼ��

	for (i=0;i<m-6 && Threshold==0;i++)
	{
		
		x[i].erase(x[i].begin());
		int numZero=0;
		for (j=i;j<i+12 && j<m-6;j++)
		{
			if (x[j+1][0] ==0)
			{
				numZero++;
			}
		}

		if (x[i+1][0] !=0 && x[i+2][0] !=0)
		{
			Threshold=1;
		}
	}
	vector <vector <double>>::iterator iter=x.begin();
	for (;iter!=x.end();)
	{
		if (iter->empty())
		{
			x.erase(iter);
			iter=x.begin();
		}
		else
			iter++;

	}
	m=x.size();

	for (i=1;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{			
				;
			}
			else
			{
				x[i][j]=x[i-1][j];
			}
		}
	}
}



void MeanTowToOne(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=0;j<m;j++)
		{
			mean+=x[j][i]/m;
		}
		XMean.push_back(mean);
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				;
			}
		}
	}  //��ֵ����
	for (i=0;i<n;i++)
	{
		for (j=1;j<m;j++,j++)
		{
			x[j][i]=(x[j][i]+x[j-1][i])/2;
			x[j-1][i]=0;
		}
	}
}


double Scale(vector< vector<double> > &x)
{
	int i,j,a0=0,a1=0,a2=0,a3=0,a4=0,a5=0,a6=0,a7=0,a8=0,a9=0;
	int m=x.size();
	int n=x[0].size();
	for (i=0;i<n;i++)
	{
		for (j=4;j<m-4;j++)
		{
			if (x[j][i]-int(x[j][i])>0.9)
			{
				a9++;
			}
			if (x[j][i]-int(x[j][i])>0.8)
			{
				a8++;
			}
			if (x[j][i]-int(x[j][i])>0.7 && x[j][i]-int(x[j][i])<0.9)
			{
				a7++;
			}
			if (x[j][i]-int(x[j][i])>0.6 && x[j][i]-int(x[j][i])<0.8)
			{
				a6++;
			}
			if (x[j][i]-int(x[j][i])>0.5 && x[j][i]-int(x[j][i])<0.7)
			{
				a5++;
			}
			if (x[j][i]-int(x[j][i])>0.4 && x[j][i]-int(x[j][i])<0.6)
			{
				a4++;
			}
			if (x[j][i]-int(x[j][i])>0.3 && x[j][i]-int(x[j][i])<0.5)
			{
				a3++;
			}
			if (x[j][i]-int(x[j][i])>0.2 && x[j][i]-int(x[j][i])<0.4)
			{
				a2++;
			}
			if (x[j][i]-int(x[j][i])>0.1 && x[j][i]-int(x[j][i])<0.3)
			{
				a1++;
			}
			if (x[j][i]-int(x[j][i])<0.2)
			{
				a0++;
			}

		}
	}
	vector <int>dis;
	dis.push_back(a0);
	dis.push_back(a1);
	dis.push_back(a2);
	dis.push_back(a3);
	dis.push_back(a4);
	dis.push_back(a5);
	dis.push_back(a6);
	dis.push_back(a7);
	dis.push_back(a8);
	dis.push_back(a9);
	vector <int> ::iterator iter;

	iter=max_element(dis.begin(),dis.end());
	int pos=iter-dis.begin();
	double scale_my=(pos+1)*0.1;
	return scale_my;

}

void MeanInt(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[0].size();
	int Nozero=0;
	int vec=0;
	int i,j;
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=8;j<m-8;j++)
		{
			mean+=x[j][i]/(m-16);
		}
		XMean.push_back(mean);
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				if (x[i][j]>10)
				{
					x[i][j]=x[i][j]-12;
				}
				if (x[i][j]<-10)
				{
					x[i][j]=x[i][j]+12;
				}
			}
		}
	}  //��ֵ����
	double scale=Scale(x);
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=scale;
				if (x[i][j]-(int)x[i][j]>0.7 || x[i][j]-(int)x[i][j]<0.3)
				{
					x[i][j]=int(x[i][j]+0.5);
				}
				
				cout<<i<<","<<x[i][j]<<" ";
			}
		}
	}  //��ֵ����
}


float MeanPlus( vector<float>  &x, float plus)
{
	float mean=0;
	int m=x.size();
	int Nozero=0;
	int i,j;
	mean=0;
	for (j=0;j<m;j++)
	{
		mean+=x[j]/m;
	}
	mean+=plus;
	for (i=0;i<m;i++)
	{
		if(x[i]!=0)
		{			
			x[i]-=mean;
			if (x[i]>12)
			{
				x[i]=x[i]-12;
			}
			if (x[i]<-12)
			{
				x[i]=x[i]+12;
			}
		}
	}  //��ֵ����
	return mean;
}

//����ֵ����������ֵ��>12��-12��<-12��+12
float MinusMeanSmooth(vector<float> &x)
{
	float mean=0;
	int m=x.size();
	int i,j;

	for (j=0;j<m;j++)
	{
	    mean+=x[j]/m;
	}
	for (i=0;i<m;i++)
	{
		if(x[i]!=0)
		{			
			x[i]-=mean;
			if (x[i]>12)
			{
				x[i]=x[i]-12;
			}
			if (x[i]<-12)
			{
				x[i]=x[i]+12;
			}
		}
	}  //��ֵ����
	return mean;
}


float Mean( vector<float>::iterator  Ybegin,  vector<float>::iterator  Yend)
{
	float mean=0;
	int m=Yend-Ybegin;
	int Nozero=0;
	int i,j;
	mean=0;
	for (j=0;j<m;j++)
	{
		mean+=*(Ybegin+j)/m;
	}
	for (i=0;i<m;i++)
	{
		if(*(Ybegin+i)!=0)
		{			
			*(Ybegin+i)-=mean;
			if (*(Ybegin+i)>12)
			{
				*(Ybegin+i)=*(Ybegin+i)-12;
			}
			if (*(Ybegin+i)<-12)
			{
				*(Ybegin+i)=*(Ybegin+i)+12;
			}
		}
	}  //��ֵ����
	return mean;
}



void Mean( float  *x ,int length)
{
	double mean=0;
	int Nozero=0;
	int i,j;
	mean=0;
	for (j=0;j<length;j++)
	{
		mean+=x[j]/length;
	}
	for (i=0;i<length;i++)
	{
		if(x[i]!=0)
		{			
			x[i]-=mean;
			if (x[i]>12)
			{
				x[i]=x[i]-12;
			}
			if (x[i]<-12)
			{
				x[i]=x[i]+12;
			}
		}
	}  //��ֵ����
}


void MeanFirst(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=0;
	if (m>0)
	{
		n=x[0].size();
	}
	int Nozero=0;
	int vec=0;
	int i,j;
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		mean=0;
		for (j=0;j<m;j++)
		{
			mean+=x[j][i]/m;
		}
		XMean.push_back(mean);
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				if (x[i][j]>10)
				{
					x[i][j]=x[i][j]-12;
				}
				if (x[i][j]<-10)
				{
					x[i][j]=x[i][j]+12;
				}
			}
		}
	}  //��ֵ����
}

void MeanBefor8AndAfter8(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[0].size();
	int Nozero=0;
	int vec=0;
	int i,j;

	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=8;j<m-8;j++)
		{
			mean+=x[j][i]/(m-16);
		}
		XMean.push_back(mean);
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				cout<<i<<","<<x[i][j]<<" ";
				//outf<<i<<","<<x[i][j]<<" ";
				if (x[i][j]>10)
				{
					x[i][j]=x[i][j]-12;
				}
				if (x[i][j]<-10)
				{
					x[i][j]=x[i][j]+12;
				}
			}
		}
	}  //��ֵ����

}

//����ֵ
void MinusMean(vector<float> &x)
{
	float mean=0;
	int m=x.size();
	for (int i=0;i<m;i++)
	{
		mean+=x[i]/m;
	}
	for (int i=0;i<m;i++)
	{
		x[i]-=mean;
	}
}

void Dimention20LSHToToneInteger(vector<double> &x)
{
	int m=x.size();
	for (int i=0;i<m;i++)
	{
		x[i]=(int)(x[i]+0.5);
	}

}

void MeanOld(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[0].size();
	int Nozero=0;
	int vec=0;
	int i,j;
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{			
				vec=1;
			}
		}
		if (vec==1)
		{
			Nozero++;
		}
		vec=0;
	}
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=0;j<m;j++)
		{
			mean+=x[j][i]/Nozero;
		}
		XMean.push_back(mean);
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				cout<<i<<","<<x[i][j]<<" ";
			}
		}
	}  //��ֵ����
}


void MeanVar(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[0].size();
	int i,j;
	double variance=0;
	vector<double> XVariance;
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=0;j<m;j++)
		{
			mean+=x[j][i]/m;
		}
		XMean.push_back(mean);
		mean=0;
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
				;
			}
		}
	}  //��ֵ����
	for (i=0;i<n;i++)
	{
		for (j=0;j<m;j++)
		{
			variance+=x[j][i]*x[j][i];
		}
		variance=sqrt(variance)/m;
		XVariance.push_back(variance);
		variance=0;
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]/=XVariance[j];
				;
			}
		}
	}  //�����
}


void Var(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[0].size();
	int i,j;
	double variance=0;
	vector<double> XVariance;
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=0;j<m;j++)
		{
			variance+=x[j][i]*x[j][i];
		}
		variance=sqrt(variance/m);
		XVariance.push_back(variance);
		variance=0;
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{	
				if (XVariance[j]>8)
				{
				}
				x[i][j]/=2;
				;
			}
		}
	}  //�����
}

//����ֵ������������ǰthd֡�ľ�ֵ
void MinusMeanWithThd(vector<float>  &x, int thd)
{
	double mean=0;
	int m=x.size();
	int i,j;
	int min = MyMinTwo(m,thd);

	for (j=0;j<min;j++)
	{
		mean+=x[j]/min;
	}
	for (i=0;i<m;i++)
	{
		if(x[i]!=0)
		{			
			x[i]-=mean;
		}
	}
}

void Mean8MinutesInt(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	int min=MyMinTwo(m,251);
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=0;j<min;j++)
		{
			mean+=x[j][i]/min;
		}
		mean=int(mean+0.5);
		XMean.push_back(mean);
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
			}
		}
	}  //��ֵ����
}

void Var8Minutes(vector< vector<double> > &x)
{
	double variance=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	int min=MyMinTwo(m,251);
	vector<double> XVariance;
	for (i=0;i<n;i++)
	{
		for (j=0;j<min;j++)
		{
			variance+=x[j][i]*x[j][i];
		}
		variance=sqrt(variance/min);
		XVariance.push_back(variance);
		variance=0;
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]/=XVariance[j];
				;
			}
		}
	}  //�����
}

void Mean8MinutesVar(vector< vector<double> > &x)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;
	double variance=0;
	vector<double> XVariance;
	int min=MyMinTwo(m,251);
	vector<double> XMean;
	for (i=0;i<n;i++)
	{
		for (j=0;j<min;j++)
		{
			mean+=x[j][i]/min;
		}
		XMean.push_back(mean);
		mean=0;
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]-=XMean[j];
			}
		}
	}  //��ֵ����
	for (i=0;i<n;i++)
	{
		for (j=0;j<min;j++)
		{
			variance+=x[j][i]*x[j][i];
		}
		variance=sqrt(variance)/min;
		XVariance.push_back(variance);
		variance=0;
	}
	for (i=0;i<m;i++)
	{
		for (j=0;j<x[0].size();j++)
		{
			if(x[i][j]!=0)
			{			
				x[i][j]/=XVariance[j];
			}
		}
	}  //�����
}

int DistanceMatrix(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D)
{
	int i,j,m,n;
	m=queryX.size();
	n=dataY.size();
	for (i=0;i<m;i++)
	{
		for (j=0;j<n && j<m*3;j++)
		{
			//D[i][j]=MyDistance(queryX[i],dataY[j]);
		}
	}
	return 0;
}

double DTW(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	D[1][1]+=D[0][0];
	D[2][1]+=D[0][0]+penalty;
	for (j=2;j<n-(m-1)/2;j++)
	{
		D[1][j]+=min(D[0][j-1],D[0][j-2]+penalty);
	}
	for (j=2;j<n-(m-2)/2;j++)
	{
		D[2][j]+=MyMin(D[0][j-1]+penalty,D[1][j-1],D[1][j-2]+penalty);

	}
	for (i=3;i<m;i++)
	{
		for (j=(i+1)/2;j<n-(m-i)/2;j++)
		{
			D[i][j]+=MyMin(D[i-2][j-1]+penalty,D[i-1][j-1],D[i-1][j-2]+penalty);
		}
	}
	for (j=1;j<D[i-1].size();j++)
	{
		DL.push_back(D[i-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	return *disIter;
}
double DTWtotalOrig(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	for (j=1;j<m;j++)
	{
		D[j][0]+=D[j-1][0];
	}
	for (i=1;i<m;i++)
	{
		for (j=1;j<n;j++)
		{
			D[i][j]+=MyMin(D[i][j-1]+penalty,D[i-1][j-1],D[i-1][j]+penalty);
		}
	}
	for (j=1;j<D[i-1].size();j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	return *disIter;
}

double DTWtotalFive(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	for (j=1;j<m;j++)
	{
		D[j][0]+=D[j-1][0];
	}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty);
	for (j=2;j<m;j++)
	{
		D[j][1]+=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]+=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n;j++)
		{
			D[i][j]+=MyMin(MyMin(D[i][j-1]+penalty,D[i-1][j-1],D[i-1][j]+penalty),D[i-1][j-2]+penalty,D[i-2][j-1]+penalty);
		}
	}
	for (j=1;j<D[i-1].size();j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	return *disIter;
}

double DTWtotal(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	for (j=2;j<m;j++)
	{
		D[j][0]=inf;
	}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty);
	for (j=2;j<m;j++)
	{
		D[j][1]+=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]+=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n;j++)
		{
			D[i][j]+=MyMin(D[i-1][j-1],D[i-1][j-2]+penalty,D[i-2][j-1]+penalty);
		}
	}
	for (j=1;j<D[i-1].size();j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	return *disIter;
}


double DTWBeginQueryAndSongSevenNo(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D)
{
	double distanceM1;
	double distanceM2;
	double distanceM3;
	double distanceM4;
	double distanceM5;
	double distanceMin;
	double distanceM6;
	double distanceM7;
	int m=queryX.size();
	int n=queryX[0].size();
	int i,j;
	vector<vector<double>>query(m,vector<double>(n,0));
	DistanceMatrix(queryX,dataY,D);
	distanceM1=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			query[i][j]=queryX[i][j]+2;
		}
	}
	DistanceMatrix(query,dataY,D);
	distanceM2=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			query[i][j]=queryX[i][j]+1;
		}
	}
	DistanceMatrix(query,dataY,D);
	distanceM3=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			query[i][j]=queryX[i][j]-2;
		}
	}
	DistanceMatrix(query,dataY,D);
	distanceM4=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			query[i][j]=queryX[i][j]-1;
		}
	}
	DistanceMatrix(query,dataY,D);
	distanceM5=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			query[i][j]=queryX[i][j]-3;
		}
	}
	DistanceMatrix(query,dataY,D);
	distanceM6=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			query[i][j]=queryX[i][j]+3;
		}
	}
	DistanceMatrix(query,dataY,D);
	distanceM7=DTWBeginNew(D);
	distanceMin=MyMin(MyMin(distanceM1,distanceM2,distanceM3),distanceM4,distanceM5);
	distanceMin=MyMin(distanceMin,distanceM6,distanceM7);
	return distanceMin;
}


double DTWdisRecur(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D,double disMid,double num,double bound)
{                  
	int m=queryX.size();
	int n=queryX[0].size();
	int i,j;
	vector<vector<double>> queryPlus(m,vector<double>(n,0));
	vector<vector<double>> queryMinus(m,vector<double>(n,0));
	double disMinus=0,disPlus=0,distanceMin=0;
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			queryMinus[i][j]=queryX[i][j]-num;//num������ƫ��ֵ
		}
	}
	DistanceMatrix(queryMinus,dataY,D);
	disMinus=DTWBeginNew(D);
	for (i=0;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			queryPlus[i][j]=queryX[i][j]+num;
		}
	}
	DistanceMatrix(queryPlus,dataY,D);
	disPlus=DTWBeginNew(D);
	distanceMin=MyMin(disMid,disMinus,disPlus);
	if (num<=bound)//bound��������ƫ����Ŀ
	{
		return distanceMin;
	}
	else if (distanceMin==disMid)
	{
		return DTWdisRecur(queryX,dataY,D,distanceMin,num/2,bound);
	}
	else if (distanceMin==disMinus)
	{
		return DTWdisRecur(queryMinus,dataY,D,distanceMin,num/2,bound);
	}
	else if (distanceMin==disPlus)
	{
		return DTWdisRecur(queryPlus,dataY,D,distanceMin,num/2,bound);
	}
	else
		return distanceMin;
}

double DTWBeginQueryAndSongSeven(vector< vector<double> > &queryX, vector< vector<double> > &dataY,vector< vector<double> > &D)
{
	double distanceMin;
	int m=queryX.size();
	int n=queryX[0].size();
	int i,j;
	DistanceMatrix(queryX,dataY,D);
	distanceMin=DTWBeginNew(D);
	return DTWdisRecur(queryX,dataY,D,distanceMin,1,1);//���������1��1��ʾ��������ƽ��һ��
}

double DTWBeginNew(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	for (j=1;j<m;j++)
	{
		D[j][0]=inf;
	}
	for (j=1;j<n;j++)
	{
		D[0][j]=0;
	}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty)+MyMinTwo(D[1][1],disConst);
	for (j=2;j<m;j++)
	{
		D[j][1]=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty)+MyMinTwo(D[j][1],disConst);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty)+MyMinTwo(D[1][j],disConst);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n && j<m*1.7;j++)
		{
			D[i][j]=MyMin(D[i-1][j-1]+penalty,D[i-1][j-2],D[i-2][j-1]+penalty)-Dpenalty+MyMinTwo(D[i][j],disConst);
		}
	}
	for (j=1;j<D[i-1].size() && j<m*1.7;j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	return *disIter;
}

double DTWBeginNewLSH(vector<vector<double>> &D)//�õ��ǹ�һ���ľ���
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	for (j=1;j<m;j++)
	{
		D[j][0]=inf;
	}
	for (j=1;j<n;j++)
	{
		D[0][j]=inf;
	}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty)+MyMinTwo(D[1][1],disConst);
	for (j=2;j<m;j++)
	{
		D[j][1]=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty)+MyMinTwo(D[j][1],disConst);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty)+MyMinTwo(D[1][j],disConst);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n && j<m*1.7;j++)
		{
			D[i][j]=MyMin(D[i-1][j-1],D[i-1][j-2]+penalty,D[i-2][j-1]+penalty)-Dpenalty+MyMinTwo(D[i][j],disConst);
		}
	}
	for (j=1;j<D[i-1].size() && j<m*1.7;j++)
	{
		DL.push_back(D[m-1][j]/m);//�õ��ǹ�һ���ľ���,���query���Ȳ�һ��
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	if	(n>m+1)
	{
		return D[m-1][n-1]/m;
    }
	else
		return *disIter;
	return D[m-1][n-1]/m;//������������Ǹ���������֪�������
}


double DTWBeginAndRA(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	for (j=1;j<m;j++)
	{
		D[j][0]=inf;
	}
	for (j=1;j<n;j++)
	{
		D[0][j]=inf;
	}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty)+MyMinTwo(D[1][1],disConst);
	for (j=2;j<m;j++)
	{
		D[j][1]=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty)+MyMinTwo(D[j][1],disConst);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty)+MyMinTwo(D[1][j],disConst);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n && j<m*1.7;j++)
		{
			D[i][j]=MyMin(D[i-1][j-1],D[i-1][j-2]+penalty,D[i-2][j-1]+penalty)-Dpenalty+MyMinTwo(D[i][j],disConst);
		}
	}
	return D[m-1][n-1];//������������Ǹ�
}


double DTWBeginThreeOld(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	for (j=2;j<m;j++)
	{
		D[j][0]=inf;
	}
	for (j=2;j<n;j++)
	{
		D[0][j]=inf;
	}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty);
	for (j=2;j<m;j++)
	{
		D[j][1]+=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]+=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n && j<m*1.4;j++)
		{
			D[i][j]+=MyMin(D[i-1][j-1],D[i-1][j-2]+penalty,D[i-2][j-1]+penalty)-Dpenalty;
		}
	}
	for (j=1;j<D[i-1].size() && j<m*1.4;j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	return *disIter;
}


double DTWtotalPlusPe(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	for (j=1;j<m;j++)
	{
		D[j][0]+=D[j-1][0];
	}
	D[1][1]=MyMin(D[1][0]+penalty,D[0][0],D[0][1]+penalty);
	for (j=2;j<m;j++)
	{
		D[j][1]+=MyMinTwo(MyMin(D[j][0]+penalty,D[j-1][0],D[j-1][1]+penalty),D[j-2][0]+penalty);
	}
	for (j=2;j<n;j++)
	{
		D[1][j]+=MyMinTwo(MyMin(D[0][j]+penalty,D[0][j-1],D[1][j-1]+penalty),D[0][j-2]+penalty);
	}
	for (i=2;i<m;i++)
	{
		for (j=2;j<n;j++)
		{
			D[i][j]+=MyMin(D[i-1][j-1],D[i-1][j-2]+penalty,D[i-2][j-1]+penalty)-0.3;
		}
	}
	for (j=1;j<D[i-1].size();j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	return *disIter;
}

double DTWbegin(vector<vector<double>> &D)
{
	int i,j,m,n;
	vector<double> DL;
	m=D.size();
	n=D[0].size();

	for (j=1;j<m;j++)
	{
		D[j][0]+=D[j-1][0];
	}
	for (i=1;i<m;i++)
	{
		for (j=1;j<n && j<m*2;j++)
		{
			D[i][j]+=MyMin(D[i][j-1]+penalty,D[i-1][j-1],D[i-1][j]+penalty);
		}
	}
	for (j=1;j<D[i-1].size() && j<m*2;j++)
	{
		DL.push_back(D[m-1][j]);
	}
	vector<double>::iterator disIter;
	disIter=min_element(DL.begin(),DL.end());
	return *disIter;
}

int PitchToTone(vector <vector <double>> &queryPitch)
{
	int n=queryPitch.size();
	double pitchnum=0;
	int k=FREQ,sam=win;
	for (int i=0;i!=n;i++)
	{
		if (queryPitch[i][0]!=0)
		{
			pitchnum=queryPitch[i][0];
			pitchnum=k*1000/pitchnum;
			pitchnum=69+12*log(pitchnum/440)/log(2.0);
			cout<<i<<","<<pitchnum<<" ";
			queryPitch[i][0]=pitchnum;
		}
	}
	return 0;
}

//��������ת��Ϊ������������
int realPitchToToneShengda(vector <float> &queryPitch)
{
	int n = queryPitch.size();
	float pitchnum = 0;
	for (int i=0;i!=n;i++)
	{
		if (queryPitch[i]!=0)
		{
			pitchnum = queryPitch[i];
            pitchnum = (12.0f*(pitchnum-log(440.0f)/log(2.0f))+69.0f);
			queryPitch[i] = pitchnum;
		}
	}
	return 0;
}


int realPitchToTone(vector <float> &queryPitch)
{
	int n=queryPitch.size();
	float pitchnum=0;
	for (int i=0;i!=n;i++)
	{
		if (queryPitch[i]!=0)
		{

			pitchnum=queryPitch[i];
			pitchnum=69+12*log(pitchnum/440)/log(2.0);
			queryPitch[i]=pitchnum;
		}
	}
	return 0;
}

int realPitchToThreeTone(vector <vector <double>> &queryPitch,vector <vector <double>> &queryPitchTow,vector <vector <double>> &queryPitchThree)
{
	int n=queryPitch.size();
	double pitchnum=0;
	for (int i=0;i!=n;i++)
	{
		queryPitchTow.push_back(queryPitch[i]);
		queryPitchThree.push_back(queryPitch[i]);
		if (queryPitch[i][0]!=0)
		{

			pitchnum=queryPitch[i][0];
			pitchnum=69+12*log(pitchnum/440)/log(2.0);
			int pitch=queryPitch[i][0];
			if ((pitch >=82.4) && (pitch <=1046.5))
			{
				;
			}
			queryPitchTow[i][0]=69+12*log(queryPitch[i][0]*2/440)/log(2.0);
			queryPitchThree[i][0]=69+12*log(queryPitch[i][0]/2/440)/log(2.0);
			queryPitch[i][0]=pitchnum;
		}
	}
	return 0;
}

int realPitchToAnotherTowTone(vector <vector <double>> &queryPitch,vector <vector <double>> &queryPitchTow,vector <vector <double>> &queryPitchThree)
{
	int n=queryPitch.size();
	double pitchnum=0;
	for (int i=0;i!=n;i++)
	{
		queryPitchTow.push_back(queryPitch[i]);
		queryPitchThree.push_back(queryPitch[i]);
		if (queryPitch[i][0]!=0)
		{

			pitchnum=queryPitch[i][0];
			pitchnum=440*pow(2.0,(pitchnum-69)/12);//��ԭΪԭʼ��Ƶ
			int pitch=queryPitch[i][0];
			if ((pitch >=82.4) && (pitch <=1046.5))
			{
				;
			}
			queryPitchTow[i][0]=69+12*log(pitchnum*1.2/440)/log(2.0);//�ӱ�
			queryPitchThree[i][0]=69+12*log(pitchnum/1.2/440)/log(2.0);//����
		}
	}
	return 0;
}


int ToneTorealPitch(vector <vector <double>> &queryPitch)
{
	int n=queryPitch.size();
	double pitchnum=0;
	for (int i=0;i!=n;i++)
	{
		if (queryPitch[i][0]!=0)
		{

			pitchnum=queryPitch[i][0];
			pitchnum=pow(2.0,(pitchnum-69)/12)*440;
			queryPitch[i][0]=pitchnum;
		}
	}
	return 0;
}

void readinstance(char *wavename,vector <vector <double>> &queryPitch)
{
	ifstream indexFile(wavename);
	vector <double> pitchNum;
	string songPitchNum;
	while (indexFile>>songPitchNum)
	{
		pitchNum.clear();
		pitchNum.push_back(atof(songPitchNum.c_str()));
		queryPitch.push_back(pitchNum);

	}
	indexFile.close();
}

void discre(vector <vector <double>> &x)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;

	for (i=1;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0)
			{			
				x[i-1][j]=x[i][j]-x[i-1][j];
			}
		}

	}
	for (i=0;i<n;i++)
	{
		x[m-1][i]=0;
	}
}


void Zerodiscre(vector <vector <double>> &x)
{
	double mean=0;
	int m=x.size();
	int n=x[m-1].size();
	int i,j;

	for (i=1;i<m;i++)
	{
		for (j=0;j<n;j++)
		{
			if(x[i][j]!=0&&x[i-1][j]!=0)
			{			
				x[i-1][j]=x[i][j]-x[i-1][j];
			}
			else
			{
				x[i-1][j]=0;
			}
		}
	}
	for (i=0;i<n;i++)
	{
		x[m-1][i]=0;
	}
}

//��������
//���룺queryX��ԭ����stretch����������
//�����dataY��������Ĵ�
void StringToString( vector<float>  &queryX,  vector<float>  &dataY,float stretch)
{
	int i,j,m,n,k;
	m = queryX.size();
	float ratio = 0;
	for (i=0;i<(m-1)*stretch;i++)
	{
		float pitch_query=0;
		k = i/stretch;
		ratio=i/stretch-k;
		if (k<m-1)
		{
			pitch_query=(queryX[k]*(1-ratio)+queryX[k+1]*ratio);
			dataY.push_back(pitch_query);
		}
	}
}

void StringToStringSame( vector<float>  &queryX,  vector<float>  &dataY,float stretch)
{
	int i,j,m,n,k;
	m=queryX.size();
	float ratio=0;
	for (i=0;i<(m-1)*stretch;i++)
	{
		float pitch_query=0;
		k=i/stretch;
		ratio=i/stretch-k;
		if (k<m-1)
		{
			dataY.push_back(queryX[k]);
		}
	}
}

float LinearToDis( vector<float>  &queryX,  vector<float>  &dataY)
{
	int i,j,m,n=0,k;
	m=queryX.size();
	if	(m>0 && dataY.size()>0)
	{
	   float stretch=((double)dataY.size())/queryX.size();
	   float ratio=0;
	   float Dis=0;
	   vector<float>  queryStretchX;
	   int totalm=(m-1)*stretch;
	   for (i=0;i<totalm;i++)
	   {
		   double pitch_query=0;
		   k=i/stretch;
		   ratio=i/stretch-k;
		   if (k<m-1)
		   {

			   pitch_query=(queryX[k]*(1-ratio)+queryX[k+1]*ratio);

			   queryStretchX.push_back(pitch_query);
		   }
	   }
	   Dis=StringMatch(queryStretchX,dataY);
	   return Dis;
	}
	else
		return 0;
}

//queryX��������������� DataY��Ӧ�ĺ�ѡ�������� length=12  ����ѡ���н����������̲��� �Ƚ���߳��ξ��� ȥ������Сֵ
float CalculateOptimalEdge( vector<float>  &queryX,  vector<float>  &dataY,int &left,int &right,int length,float ratio)
{
	int n=queryX.size();
	int m=dataY.size();
	int bestLeft=0;
	int bestRight=0;
	float bestDis=10000;
	float Cdis=0;
	int step=4;
	vector<float>::iterator  Ybegin=dataY.begin();
	vector<float>::iterator  Yend=dataY.end();
	for (int i=0;i< length;i+=step)
	{
		for (int j=0;j<length;j+=step)
		{
			Mean(Ybegin+i,Yend-j);////����ֵ����������ֵ��>12��-12��<-12��+12

			Cdis=LinearToDisIter(queryX.begin(),queryX.end(),Ybegin+i,Yend-j);//�����ѡ��߳��ε���̾���
			if (bestDis>Cdis)
			{//ȡ����̾��� ����ѡƬ�����һ�����֡��
				bestDis=Cdis;
				bestLeft=i;
				bestRight=j;
			}
		}
	}
	left=bestLeft;
	right=bestRight;
	if (bestLeft!=0)
	{
		dataY.erase(dataY.begin(),dataY.begin()+bestLeft);//����������
	}
	if (bestRight!=0)
	{
		dataY.erase(dataY.end()-bestRight,dataY.end());//����������
	}
	Mean(dataY.begin(),dataY.end());//����ֵ����
	return bestDis;//������̾���
}
//�߳����м���ѡ����  �Ƚ����߾��� ȡ��̾���ֵ����
float LinearToDisIter( vector<float>::iterator  Xbegin,  vector<float>::iterator  Xend,
					   vector<float>::iterator  Ybegin,  vector<float>::iterator  Yend)
{
	int i,j,m,n=0,k;
	m=Xend-Xbegin;
	n=Yend-Ybegin;
	float dis=0;
	if	(m>0 && n>0)
	{
		float stretch=((double)n)/m;
		float ratio=0;
		float Dis=0;
		int numY=0;
		int totalm=(m-1)*stretch;
		for (i=0;i<totalm;i++)
		{
			float pitch_query=0;
			k=i/stretch;
			ratio=i/stretch-k;
			if (k<m-1)
			{

				numY++;
				pitch_query=(*(Xbegin+k)*(1-ratio)+*(Xbegin+k+1)*ratio);
				Dis+=MyMinTwo(abs(pitch_query-*(Ybegin+i)),disConstString);

			}
		}

		if(numY!=0)
		{
			Dis/=numY;
		}
		return Dis;
	}
	else
		return 0;
}


double LinearToDisUltimate(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int &ultimateNum)
{
	int i,j,m,n=0,k;
	m=queryX.size();
	if	(m>0 && dataY.size()>0)
	{
		n=queryX[0].size();
		double stretch=((double)dataY.size())/queryX.size();
		double ratio=0;
		double Dis=0;
		vector< vector<double> > queryStretchX;
		int totalm=(m-1)*stretch;
		for (i=0;i<totalm;i++)
		{
			vector<double> pitch_query;
			k=i/stretch;
			ratio=i/stretch-k;
			if (k<m-1)
			{
				for (j=0;j<n;j++)
				{
					pitch_query.push_back(queryX[k][j]*(1-ratio)+queryX[k+1][j]*ratio);
				}
				queryStretchX.push_back(pitch_query);
			}
		}
		Dis=StringMatchUltimate(queryStretchX,dataY,ultimateNum);
		return Dis;
	}
	else
		return 0;
}



void StringToStringNoMean(vector< vector<double> > &queryX, vector< vector<double> > &dataY,double stretch)
{
	int i,m,n,k;
	m=queryX.size();
	n=queryX[0].size();
	for (i=0;i<m*stretch;i++)
	{
		k=i/stretch;
		if (k<m)
		{
			dataY.push_back(queryX[k]);
		}
	}
}

float StringMatch( vector<float>  &queryX,  vector<float>  &dataY)
{
	int i,j,m,n;
	m=queryX.size();
	n=dataY.size();
	float dis=0;
	int num=MyMinTwo(m,n);
	for (i=0;i< num;i++)
	{
		dis+=MyMinTwo(abs(queryX[i]-dataY[i]),disConstString);
	}
	if(num!=0)
	{
		dis/=num;
	}
	return dis;
}


double StringMatchUltimate(vector< vector<double> > &queryX, vector< vector<double> > &dataY,int &ultimateNum)
{
	int i,j,m,n;
	m=queryX.size();
	n=dataY.size();
	double dis=0;
	int num=MyMinTwo(m,n);
	for (i=0;i< num;i++)
	{
		//dis+=MyMinTwo(MyDistance(queryX[i],dataY[i]),disConstString);
	}
	ultimateNum+=num;
	return dis;
}

double StringMatchToDis(vector< vector<double> > &queryX, vector< vector<double> > &dataY)
{
	int n=dataY.size();
	double stretch=0.75;
	double distanceM;
	vector <double> distanceStretch;
	for (;stretch<1.3;)
	{
		vector<vector<double>> queryStretch;
		stretch+=0.05;
		distanceStretch.push_back(distanceM);
	}
	stable_sort(distanceStretch.begin(),distanceStretch.end());
	distanceM=distanceStretch[0];
	return distanceM;
}


double StringMatchToDisMapRALSHNewPairVariancePositionVariance( vector<float>  &queryX,  vector<float>  &dataY,
															   int recurse,int pairNum,double MidPercentage,int &ultimateNum)
{
	int i;
	int sizeX=queryX.size();
	int sizeY=dataY.size();
	int MidPos=sizeX/2;
	int BeginMatchPos=sizeX*MidPercentage*1/3;
	double MovePoints=(sizeX-BeginMatchPos*2)/(double)pairNum;
	if (MovePoints<1)
	{
		MovePoints=1;
	}
	int MidDataYSize=dataY.size()/2;
	int MinDisPos=0;//��С�����λ��
	double distanceM;
	map <double,int>disMap;
	vector <double> distanceStretch;
	if (sizeX<=8 || sizeY<=8)
	{
		distanceM=LinearToDis(queryX,dataY);
		ultimateNum++;
		return distanceM;
	}
	vector<float> DataY_L(dataY.begin(),dataY.begin()+MidDataYSize);
	vector<float>  DataY_R(dataY.begin()+MidDataYSize,dataY.end());

	for (i=0;i<=pairNum;i++)
	{
		vector<float>  queryX_L(queryX.begin(),queryX.begin()+BeginMatchPos+i*MovePoints);
		vector<float>  queryX_R(queryX.begin()+BeginMatchPos+i*MovePoints,queryX.end());

		distanceM=LinearToDis(queryX_L,DataY_L)+LinearToDis(queryX_R,DataY_R);
		disMap.insert(make_pair(distanceM,i));
		distanceStretch.push_back(distanceM);

		if (queryX.begin()+BeginMatchPos+i*MovePoints==queryX.end()-2)
		{
			i+=pairNum;
		}
	}
	stable_sort(distanceStretch.begin(),distanceStretch.end());
	distanceM=distanceStretch[0];
	if (recurse==0)
	{
		ultimateNum+=2;
		return distanceM;
	}
	else
	{
		if (disMap.count(distanceM))
		{
			MinDisPos=disMap[distanceM];
			recurse--;
			MidPercentage=MidPercentage*1.1;
			pairNum=pairNum-2;
			if (pairNum<3)
			{
				pairNum=3;
			}
			 vector<float>  queryX_L(queryX.begin(),queryX.begin()+BeginMatchPos+MinDisPos*MovePoints);
			vector<float>  queryX_R(queryX.begin()+BeginMatchPos+MinDisPos*MovePoints,queryX.end());
			distanceM=StringMatchToDisMapRALSHNewPairVariancePositionVariance(queryX_L,DataY_L,recurse,pairNum,MidPercentage,ultimateNum)+
				StringMatchToDisMapRALSHNewPairVariancePositionVariance(queryX_R,DataY_R,recurse,pairNum,MidPercentage,ultimateNum);

			return distanceM;
		}
	}
	return distanceM;
}

//KTRA������С����
float RAPositionVarianceOptimal( vector<float>  &queryX,  vector<float>  &dataY,
															   int recurse,int pairNum,float MidPercentage,int &ultimateNum)//3��5,1
{
	int i;
	int sizeX=queryX.size();
	int sizeY=dataY.size();
	int MidPos=sizeX/2;
	int BeginMatchPos=sizeX*MidPercentage*1/3;
	float MovePoints=(sizeX-BeginMatchPos*2)/(double)pairNum;
	if (MovePoints<1)
	{
		MovePoints=1;
	}
	int MidDataYSize=dataY.size()/2;
	int MinDisPos=0;//��С�����λ��
	double distanceM;
	map <float,int>disMap;
	vector <float> distanceStretch;
	if (sizeX<=8 || sizeY<=8)
	{
		distanceM=LinearToDis(queryX,dataY);//������С����ֵ
		ultimateNum++;
		return distanceM;
	}
	vector<float> DataY_L(dataY.begin(),dataY.begin()+MidDataYSize);
	vector<float>  DataY_R(dataY.begin()+MidDataYSize,dataY.end());

	for (i=0;i<=pairNum;i++)
	{
		distanceM=LinearToDisIter(dataY.begin(),dataY.begin()+MidDataYSize,queryX.begin(),queryX.begin()+BeginMatchPos+i*MovePoints)+
			LinearToDisIter(dataY.begin()+MidDataYSize,dataY.end(),queryX.begin()+BeginMatchPos+i*MovePoints,queryX.end());//����ѡ��߳��ηֳ����Σ���ѯ��ѭ���ָ� ����RA����
		disMap.insert(make_pair(distanceM,i));//�洢��̾��뼰��Ӧѭ���������
		distanceStretch.push_back(distanceM);//�洢��̾���
		if (queryX.begin()+BeginMatchPos+i*MovePoints==queryX.end()-2)
		{//�ж��Ƿ�����ĩβ���˳�ѭ��
			i+=pairNum;
		}
	}
	stable_sort(distanceStretch.begin(),distanceStretch.end());//������С��������
	distanceM=distanceStretch[0];//ȥ��С����ֵ
	if (recurse==0)//ѭ������Ϊ�� ����
	{
		ultimateNum+=2;
		return distanceM;
	}
	else
	{

		if (disMap.count(distanceM))
		{
			MinDisPos=disMap[distanceM];//�õ���С����ѭ������i
			recurse--;
			MidPercentage=MidPercentage*1.1;
			pairNum=pairNum-2;
			if (pairNum<3)
			{
				pairNum=3;
			}
			vector<float>  queryX_L(queryX.begin(),queryX.begin()+BeginMatchPos+MinDisPos*MovePoints);
			vector<float>  queryX_R(queryX.begin()+BeginMatchPos+MinDisPos*MovePoints,queryX.end());
			distanceM=RAPositionVarianceOptimal(queryX_L,DataY_L,recurse,pairNum,MidPercentage,ultimateNum)+
				RAPositionVarianceOptimal(queryX_R,DataY_R,recurse,pairNum,MidPercentage,ultimateNum);//����ѯ�����ѡ�ηֶν���RA����
			return distanceM;
		}
	}
	return distanceM;
}

void StringTosignature(vector<float>  &dataY,  signature_t  &Y)
{
	int n=dataY.size();
	int m=emdLength;
	n=MyMinTwo(n,m);
	int num=1;
	for (int i=0;i<n-1 ;i++)
	{
		if (dataY[i]!=dataY[i+1])
		{
			num++;
		}
	}
	if (num<(MAX_SIG_SIZE-1))
	{
		Y.n=num;
	}
	else
		Y.n=MAX_SIG_SIZE-1;

	int currentNote=0;
	if (n>0)
	{
		Y.Features[0]=dataY[0];
		Y.Weights[0]=1;
	}
	for (int i=0;i<n-1 && currentNote<MAX_SIG_SIZE-1;i++)
	{
		if (dataY[i]==dataY[i+1])
		{
			Y.Weights[currentNote]+=1;
		}
		else
		{
			++currentNote;
			Y.Features[currentNote]=dataY[i+1];
			Y.Weights[currentNote]=1;
		}
	}
}

bool CompareLastName(const pair<int,string> &p1,const pair<int,string> &p2)
{
	return p1.first>p2.first;
}

vector<pair<int,int>> DuplicateSegmentBegin(vector<float> &tone)	//����������tone����λ��ƥ�俪ͷ����ƥ�䳤�ȴ���100֡����¼��λ�úͳ���֡��������
{
	vector<pair<int,int>> dupl;
	pair<int,int> segment;
	int sizeA=tone.size();
	double temp=0;
	int dupNum=0;
	for (int i=1;i<sizeA;i++)
	{
		temp=tone[i];
		dupNum=0;
		if (temp==tone[0])
		{
			for (int j=i;j<sizeA;j++)
			{
				temp=tone[j];
				if (temp==tone[j-i])
				{
					dupNum++;
				}
				else
					j+=sizeA;
			}
		}
		if (dupNum>100)
		{
			segment.first=i;
			segment.second=i+dupNum;
			dupl.push_back(segment);
		}
	}
	return dupl;
}


void OneFileToMultiFile(string fileName,int ThreadNum)
{
	ifstream wavList(fileName.c_str());
	string oneLine;
	char buffer[20];
	int totalWav=0;
	int fileCnum=0;
	for (int i=0;i<ThreadNum;i++)
	{
		string cmd("del ");
		cmd+=fileName;
		cmd+=itoa(i,buffer,10);
		system(cmd.c_str());
	}
	while (wavList>>oneLine)
	{
		totalWav++;
		fileCnum=totalWav%ThreadNum;
		string fileN=fileName+itoa(fileCnum,buffer,10);
		ofstream fileSplit(fileN.c_str(),ofstream::app);
		fileSplit<<oneLine<<endl;
		fileSplit.close();
	}
	wavList.close();
}


void MultiFileDel(string fileName,int ThreadNum)
{
	ifstream wavList(fileName.c_str());
	string oneLine;
	char buffer[20];
	for (int i=0;i<ThreadNum;i++)
	{
		string cmd("del ");
		cmd+=fileName;
		cmd+=itoa(i,buffer,10);
		system(cmd.c_str());
	}

}

//��ȥָ����ֵ
void MeanNoteLSH(vector<float> &x, float mean)
{
	int m=x.size();
	for (int i=0;i<m;i++)
	{
		x[i]-=mean;
	} 
}

//�����ֵ
float MeanLSH(vector<float> &x)
{
	float mean=0;
	int m=x.size();
	for (int i=0;i<m;i++)
	{
		mean+=x[i]/m;
	}
	return mean;

}

//��float���鸴�Ƶ�vector��
void FloatCopyToVector(vector <float> &des, float *src, int len)
{
	for (int i=0; i<len; i++)
	{
		des.push_back(src[i]);	
	}
}


//��ѯ������
//���룺wavename,��ѯ�ļ�·����param,������Ϣ��
//isWriteLSHQueryToFile,�Ƿ�LS���LSH��ѯ���ݼ�����д���ļ�
//isUseDBNResult���Ƿ���DBN��ѯ���
//�����songFive����ѯ���
int WavToSongFive(char *wavename, ParamInfo *param, vector<string>& songFive, bool isWriteLSHQueryToFile, bool isUseDBNResult)
{
	string wavOut("wavPitch.txt");	//���wav�������н�����ļ�
	ofstream wavPitch(wavOut.c_str(),ofstream::app);

	//�Ӳ����ṹ��param����ȡ��ѯ�õ��Ĳ���
	ParamInfo *pci = param;	//LSH�����ṹ��
	ParamInfo *pciNote = param+1;	//NLSH�����ṹ��

	map<string,vector<float>> indexSongName(pci->indexSongName);	//����pv�ļ�·���Ͷ�Ӧ��һά��������
	PRNearNeighborStructT IndexHuming = pci->IndexHuming;	//LSH���RNN����
	PRNearNeighborStructT IndexHumingNote = pciNote->IndexHuming;	//NLSH���RNN����
	map<unsigned long,pair<string,short>> IndexLSH(pci->IndexLSH);	//LSH��������¼LSH�����ţ���·�����ļ�������ʼλ��
	map<unsigned long,pair<string,pair<short,short>>> IndexLSHNote(pciNote->IndexLSHNote);	//NLSH��������¼NLSH�����ţ���·�����ļ�������ʼλ�ã�����֡��
	int stepFactor = pci->stepFactor;	//ѡ��LSH��ļ����������������ȡһ���㣩
	IntT RetainNum = pci->RetainNum;		//LSHÿ����������ĵ���
	IntT RetainNumNote = pciNote->RetainNum;	//NLSHÿ����������ĵ���
	IntT LSHFilterNum = pci->LSHFilterNum;	//LSH�˲������ĵ���
	IntT LSHFilterNumNote = pciNote->LSHFilterNum;	//NLSH�˲������ĵ���
	float stepRatio = pci->stepRatio;	//query��LSH�任֡��  ʲô�Ǳ任֡�ƣ���
	songFive.clear();	//��ѯ�������

	//ͳ��ʱ����ر���
	clock_t firstTime,lastTime,firstTimeTemp,lastTimeTemp,firstTimeTempLSH1,
		lastTimeTempLSH1,firstTimeTempLSH2,lastTimeTempLSH2;
	double OneSongLSHTime = 0;
	static double totalLSHTime = 0;
	double OneSongLSTime = 0;
	static double totalLSTime = 0;
	double OneSonglshnoteTime = 0;
	static double totalLSHNoteTime = 0;
	double OneSonglshPitchTime = 0;
	static double totalLSHOnlyNoteTime = 0;
	static double totalLSHOnlyPitchTime = 0;
	static double totalLSHRetrievalPostProcessPitchTime = 0;
	static double totalLSHRetrievalPostProcessPitchTimeNote = 0;
	static double totalLSHPitchTime = 0;
	static double totalLSHPitchFirstTime = 0;
	static double totalLSHNoteFirstTime = 0;
	static double totalLSHoneTime = 0;
	static double totalLSHtwoTime = 0;
	static double totalLSHthreeTime = 0;
	double OneSongEMDTime = 0;
	static double totalEMDTime = 0;
	double OneSongRATime = 0;
	static double totalRATime = 0;
	static int total = 0;

	//ʢ��Դ���룺������ȡ�����������
	float *pFeaBuf = NULL;	//��������
	int nFeaLen = 0;	//�������г���
	SNote *QueryNotes = NULL;	//��������
	int nNoteLen = 0;	//�������г���
	float ratio = 0.5;	//�����ز����������еı�������

	vector <float> queryPitch;	//pFeaBufֱ��ת������������
	vector <float> queryPitchNote;	//QueryNotesת����һά��������
	map<float ,string> songDis;//��¼��ѯ�����ѡ����С���� ��ѡ�θ�����
	
	feature_t * NoteEmd = NULL;	//��¼����ֵ���������������Ϊ��λ
	float * NoteDuration = NULL;	//��¼ÿ����������ʱ��

	signature_t query;	//��¼�������ȣ�����ֵ���������ÿ����������ʱ��
	static int shengdaPitch = 0;	//��¼������ȡ����
	vector <float> Dis;//�������бȶԺ�ľ��� 
	bool returnN = false;//�Ƿ����LSH����
	static int enhance = 0;

	//ʢ��Դ���룺������ȡ��������ȡwav�еĻ���֡�����ߵ���������
	//���룺filename��wav�ļ�·��
	//ratio�������ز����������еı�������
	//�����pFeaBuf���������У�nFeaLen���������г���
	//Query���������У�nNoteLen���������г���
	//����ֵ��0��������ERROR_CODE_TOO_SHORT_INPUT���������г���С��20����ʱpFeaBufΪ��
	cout<<wavename<<endl;
	int reNum = SMelodyFeatureExtraction(wavename,pFeaBuf,nFeaLen,QueryNotes,nNoteLen,ratio);

	if (reNum != ERROR_CODE_TOO_SHORT_INPUT)	//��ȡ���������г��Ȳ�С��20
	{
		shengdaPitch++;	//��¼������ȡ����
		FloatCopyToVector(queryPitch,pFeaBuf,nFeaLen);	//ԭʼ�������У�δ�з�֮ǰ�����ߣ�������vector��

		int emdl = MyMinTwo(nFeaLen,emdLength);	//��������ʱ����emdLengthΪ�߳��������е�ʱ������
		int lengC = 0;	//��¼����������ʱ��
		int lengNote = 0;	//��¼ʵ���������г���

		realPitchToToneShengda(queryPitch);	//��������ת��Ϊ������������

		float meanNote = MinusMeanSmooth(queryPitch);	//����ֵ����������ֵ��>12��-12��<-12��+12�����ؾ�ֵ  
		wavPitch<<wavename<<":"<<endl;
		if(!wavPitch)
		{
			cout<<"fail open wavPitch"<<endl;	
		}
		for(int k=0;k<queryPitch.size();k++)
		{
			
			wavPitch<<queryPitch[k]<<" ";
		}
		wavPitch<<endl;
		NoteEmd = (feature_t *)malloc(nNoteLen*sizeof(feature_t));	//��¼����ֵ���������������Ϊ��λ
		NoteDuration = (float *)malloc(nNoteLen*sizeof(float));	//��¼ÿ����������ʱ��

		//��QueryNotes����ȡ����ֵ���������ÿ����������ʱ��
		for (int i=0;i<nNoteLen;i++)
		{
			lengNote++;	//��¼ʵ���������г���

			if (QueryNotes[i].fNoteValue != 0)	//����ֵ��Ϊ��
				NoteEmd[i] = QueryNotes[i].fNoteValue - meanNote;	//��¼����ֵ������
			else	//����ֵΪ��
				NoteEmd[i] = QueryNotes[i].fNoteValue;	//ֱ�Ӽ�¼��������ֵ

			//ת��Ϊһά��������
			for (int j=0;j<QueryNotes[i].fNoteDuration;j++)	
				queryPitchNote.push_back(NoteEmd[i]);

			//��¼ÿ����������ʱ��
			if (lengC+QueryNotes[i].fNoteDuration <= emdLength)	//��������ʱ��δ������������ʱ����ֱ�Ӽ�¼
				NoteDuration[i] = QueryNotes[i].fNoteDuration;
			else	//��������ʱ��������������ʱ�����������һ����������ʱ��
				NoteDuration[i] = QueryNotes[i].fNoteDuration-(emdLength-lengC);

			lengC += QueryNotes[i].fNoteDuration;

			if (lengC >= emdl) break;	//��������ʱ��������������ʱ��������
		}
		query.n = lengNote;	//��¼��������
		query.Features = NoteEmd;	//��¼����ֵ���������������Ϊ��λ
		query.Weights = NoteDuration;	//��¼ÿ����������ʱ��
	}
	ofstream shengdaTimes("wav.result",ofstream::app);	//׷�Ӵ��ļ�
	shengdaTimes<<"ʢ����ȡ������"<<shengdaPitch<< endl;	//����ǵڼ�����ȡ
	shengdaTimes.close();

	//�ͷ�pFeaBuf��QueryNotes�ռ�
	if(NULL != pFeaBuf)
	{
		delete[] pFeaBuf;
		pFeaBuf=NULL;
	}
	if(NULL!=QueryNotes)
	{
		delete[] QueryNotes;
		QueryNotes=NULL;
	}

	signature_t candiY;

	candiY.Features = (feature_t *) malloc((MAX_SIG_SIZE-1)*sizeof(feature_t));
	candiY.Weights = (float *) malloc((MAX_SIG_SIZE-1)*sizeof(float));
	
	//LS(��������)����
	float FloorLevelInitial=0.6;	//�������ӳ�ʼֵ
	float FloorLevel = FloorLevelInitial;	//�������ӵ�ǰֵ
	float UpperLimit=1.7;	//������������
	float StretchStep=0.1;	//�������Ӳ���
	int MatchBeginPos=6;	//����query��dataY���������������1/MatchBeginPos��Χ֮�ڲŽ���ƥ��

	static int CandidatesDTWAll=0;
	float ratioAll=2.6;
	static int filter0ne=0;
	static int filterTwo=0;
	static int filterThree=0;
	vector <string> tempList;
	vector <float > tempDis1;
	vector <float > tempDis2;
	vector <float > tempDis3;

	bool isPrintLSHQueryVectorLS = 0;

	vector<int> allStretchCandidates;//��¼����������к�ѡ��
	vector<int> allCorrectCan;//��¼������ȷ��ѡ
	ofstream filename("LSHCandidateCorrect.txt",ofstream::app);
	static int exitCandidateNum=0;

	for (int recur=0;recur<3;recur++)
	{
		if (recur==0)
		{
			filter0ne++;
		}
		else if (recur==1)
		{
			filterTwo++;
		}
		else if (recur==2)
		{
			filterThree++;
		}
		vector <vector<vector<float>>> LSHQueryVectorLinearStretching;	//��¼��ͬ���������³�ȡ��LSH�㼯
		vector <vector<vector<float>>> LSHQueryVectorLinearStretchingNote;	//��¼��ͬ�֡�³�ȡ��NLSH�㼯
		vector<pair<short, short>> posPair;	//��¼NLSH�����ʼλ�úͳ���֡��
		

		if (recur==0)
		{

			//����LS���� û������

			FloorLevelInitial = 1;
			FloorLevel = FloorLevelInitial;
			StretchStep = 0.1;
			UpperLimit = 1;

			//��ȡһά�������е�NLSH��
			//���룺queryPitchNote����ѯ��һά��������
			//noteMinFrame��������̳���֡����������ȥ��
			//noteMaxFrame�����������֡�����������з�
			//NLSHsize,NLSH��ά��

			//�����posPairvector����¼LSH�����ʼλ�úͳ�ȡLSH����������
			//LSHQueryVectorLinearStretchingNote����¼NLSH�㣬��ά��

			QueryPitchToLSHVectorLinearStretchingShortToMoreNoteFirst(posPair,queryPitchNote,
				LSHQueryVectorLinearStretchingNote,
				param[1].noteMinFrame,param[1].noteMaxFrame,param[1].LSHsize);
		}
		else if (recur==1)
		{

			//����LS���� ��0.8��1.4 ����Ϊ0.2 ����4������

			FloorLevelInitial=0.8;
			FloorLevel=FloorLevelInitial;
			StretchStep=0.2;
			UpperLimit=1.4;
		}
		else if (recur==2)
		{

			//����LS���� ����12������

			FloorLevelInitial=0.6;
			FloorLevel=FloorLevelInitial;
			StretchStep=0.1;
			UpperLimit=1.7;

			//�ı������֡���ٳ�ȡNLSH�㣨û����LS��
			for(int recur_t=0; recur_t<1; recur_t++)

			{//ѭ��һ��

				int noteMaxFrame_t;
				if (recur_t==0)
				{
					noteMaxFrame_t = param[1].noteMaxFrame+2;
				}
				else
				{
					noteMaxFrame_t = param[1].noteMaxFrame-2;
				}
				
				//��ȡһά�������е�NLSH��
				//���룺queryPitch����ѯ��һά��������
				//noteMinFrame��������̳���֡����������ȥ��

				//noteMaxFrame�����������֡�����������з� noteMaxFrame_t=12
				//NLSHsize,NLSH��ά��
				//�����posPairvector����¼LSH�����ʼλ�úͳ�������
				//LSHQueryVectorLinearStretching����¼LSH��

				QueryPitchToLSHVectorLinearStretchingShortToMoreNoteFirst(posPair,queryPitchNote,
					LSHQueryVectorLinearStretchingNote,
					param[1].noteMinFrame,noteMaxFrame_t,param[1].LSHsize);
			}
		}

		//������������ȡLSH��
		//���룺queryPitch��һά��������
		//FloorLevel����ʼ�������ӣ�UpperLimit�������������ޣ�StretchStep����������
		//stepFactor����ȡ�����stepRatio����ȡ���ƣ�LSHsize����ȡ������recur���ϲ㺯����ѭ�����
		//�����LSHQueryVectorLinearStretching���������������µ�LSH�㼯
		QueryPitchToLSHVectorLinearStretchingShortToMore(queryPitch,LSHQueryVectorLinearStretching, 
			FloorLevel, UpperLimit,stepFactor,stepRatio, StretchStep,recur,param[0].LSHsize);
		
		if(!isPrintLSHQueryVectorLS)
		{
			if(isWriteLSHQueryToFile)
			{
				//��LS���LSH��д���ļ�
				//���룺LSHVectorLS��LS���LSH�㣬ÿ����Ϊһ���������У�vector[i][j][k]��ʾ��i�����������µĵ�j��������ĵ�k������
				//filename������ļ�·��
				LSHVectorLSToFile(LSHQueryVectorLinearStretching,query_vector_fname);
				//��LS���LSH����д���ļ�
				//���룺LSHVectorLS��LS���LSH�㣬ÿ����Ϊһ���������У�vector[i][j][k]��ʾ��i�����������µĵ�j��������ĵ�k������
				//filename������ļ�·��������ļ��У�ÿ��Ϊ����LSH���Ӧ��������Ŀǰʹ��LSH���������ļ�����
				//��һ������ļ�����ͳ��ÿ����ѯ�ļ���������
				IndexLSHLSToFile(LSHQueryVectorLinearStretching,wavename,query_index_fname,query_counter_fname);
				cout<<"��ӡ "<<wavename<<" ��QueryLSHLS ���ݺ����� ���"<<endl;
			}
			isPrintLSHQueryVectorLS = 1;
		}

		//ȡ��������LSH��Ҫ�޸������ط�
		int LinearCoe = 0;
		int LinearCoeTotal = LSHQueryVectorLinearStretching.size();	//����������LSH�㼯��Ŀ
		vector<vector<IntT>> IndexCandidatesStretch;
		vector<vector<float>> IndexCandidatesDis;
		vector<vector<IntT>> CandidatesNumStretch;

		vector<IntT> CandidatesFilter;
		firstTime=clock();
		map<int,vector<int>> :: iterator samePointIte;

		firstTimeTempLSH1 = clock();
		int edge=6;
		float LSratio=0.4;
		int qRecurse=1;
		if (recur==10)
		{
			edge=1;
		}
		else 
			edge=6;

		if (recur!=1) //��Ϊ�����ļ���
		{//ѭ������1
			//����Ϊ������LSH����
#if 1
			if(!isUseDBNResult)
			{
				LinearCoe=0;
				PPointT *QueriesArray=NULL;	//NLSH�㼯

				if (LSHQueryVectorLinearStretchingNote[LinearCoe].size()>0)	//����ǰ��NLSH�㼯��Ϊ��
				{
					//��LSH���������ж����ݼ�������PPointT*�У�����ÿ��LSH��ʱ��¼��ź�LSH���ƽ����
					//���룺LSHVector��LSH�㼯
					QueriesArray = readDataSetFromVector(LSHQueryVectorLinearStretchingNote[LinearCoe]);	//�õ���ǰNLSH�㼯

					Int32T nPointsQuery = LSHQueryVectorLinearStretchingNote[LinearCoe].size();	//��ǰNLSH�㼯��С

					IntT dimension = 6;	//NLSH��ά��
					if (nPointsQuery>0)
					{
						dimension = LSHQueryVectorLinearStretchingNote[LinearCoe][0].size();	//NLSH����ʵά��
					}

					IntT LSHFilterReturnNum = 0;
					IntT *IndexArray = NULL;
					int IndexArraySize = 1000000;	//�����Һ�ѡ��Ŀ

					IndexArray = (IntT *)MALLOC(IndexArraySize *sizeof(IntT));
					double *IndexArrayDis = (double *)MALLOC(IndexArraySize *sizeof(double));
					IntT *IndexFilterArray = NULL;

					IndexFilterArray = (IntT *)MALLOC(IndexArraySize/2 *sizeof(IntT));//����ʲô��
					IntT * NumArray = (IntT *)MALLOC(nPointsQuery *sizeof(IntT));	//ÿ���㷵�ص���Ŀ

					//�õ�NLSH���������ÿ��LSH�����RNN��ѯ������RetainNumNote����ѡ����󷵻����е�ĺ�ѡ��Ŀ
					//���룺QueriesArray,��ǰNLSH�㼯��nPointsQuery����ǰ�㼯��С
					//IndexArraySize�������Һ�ѡ��
					//IndexHumingNote��NLSH���RNN������RetainNum��NLSHÿ����������ĵ���
					//dimension��NLSH��ά����LSHFilterNum��NLSH�˲������ĵ���
					//�����
					//IndexArray����ѡ��ţ�NumArray��ÿ���㷵�صı�����Ŀ��IndexArrayDis�����к�ѡRA����
					//IndexFilterArray�����к�ѡ��ţ�sizeFilter��
					IntT ResultSize = LSHStructToResultOnePointRetainSeveral(QueriesArray,nPointsQuery,IndexArraySize, 
						IndexArray,IndexHumingNote,NumArray,RetainNumNote , dimension,LSHFilterNumNote,IndexFilterArray,
						LSHFilterReturnNum,IndexArrayDis);
				
					vector<IntT> IndexCandidates;
					vector <float> DisCandidates;
					vector<IntT> CandidatesNum;	//ÿ���㷵�ص���Ŀ����Vector
					int curreIndex=0;
					//���ص�ÿ��������к�ѡ��ž����뵽CandidatesFilter��
					for (int i=0;i<LSHFilterReturnNum;i++)
					{
						CandidatesFilter.push_back(IndexFilterArray[i]);
					}

					bool insertY=false;
					for (int i=0;i<nPointsQuery;i++)
					{//������ǰNLSH�㼯
						int numS=0;
						int siz=NumArray[i];//ÿ��LSH�㷵�صĺ�ѡ����
						for (int j=0;j<siz;j++)
						{
							if (samePoint.count(IndexArray[j+curreIndex]))//�鿴��ѡ��Ŵ��ڣ�����0��1�����Խ����ʾ��samepointû����ֵ����������
							{
								numS=0;

								samePointIte=samePoint.find(IndexArray[j+curreIndex]);//�õ���ѡ��Ŷ�Ӧ�ĺ�ѡ��ĵ�������ָ��
								numS=samePointIte->second.size();//��ѡ��Ĵ�С
								IndexCandidates.push_back(IndexArray[j+curreIndex]);//�����ѡ���
								DisCandidates.push_back(IndexArrayDis[j+curreIndex]);//�����ѡ��ľ���
								for (int k=0;k<samePointIte->second.size();k++)
								{//ȥ�����ֺ�ѡ��  ���򣿣�
									insertY=true;
									for (int l=0;l<siz;l++)
									{
										if (samePointIte->second[k]==IndexArray[l+curreIndex])
										{
											insertY=false;
										}
									}
									if (insertY==true)
									{
										IndexCandidates.push_back(samePointIte->second[k]);
										DisCandidates.push_back(IndexArrayDis[j+curreIndex]);
									}
									else
										numS--;

								}
								NumArray[i]+=numS;
							
							}
						
							else//�鿴��ѡ��Ų����ڣ������ѡ��ż�����
							{
								IndexCandidates.push_back(IndexArray[j+curreIndex]);
								DisCandidates.push_back(IndexArrayDis[j+curreIndex]);
							
							}

						}

						curreIndex+=siz;//������һ����
						CandidatesNum.push_back(NumArray[i]);//���뵱ǰLSH���ѡ����
					}
					IndexCandidatesStretch.push_back(IndexCandidates);//����㼯�ĺ�ѡ�����
					CandidatesNumStretch.push_back(CandidatesNum);//����˴�LSH���ѡ����
					IndexCandidatesDis.push_back(DisCandidates);
					for (int i=0;i<nPointsQuery;i++)//�ͷſռ�
					{
						free(QueriesArray[i]->coordinates);
						free(QueriesArray[i]);
					}
					free(IndexArray);
					free(IndexFilterArray);
					free(NumArray);
					free(QueriesArray);
					free(IndexArrayDis);
				}
				else//����ǰ��NLSH�㼯Ϊ��
				{
					vector <float> DisCandidates;
					vector<IntT> IndexCandidates;
					vector<IntT> CandidatesNum;//ÿ���㷵�ص���Ŀ����Vector
					IndexCandidatesStretch.push_back(IndexCandidates);//�����ѡ�����
					CandidatesNumStretch.push_back(CandidatesNum);//����˴�LSH���ѡ����
					IndexCandidatesDis.push_back(DisCandidates);
				}

				lastTime=clock();
				OneSongLSHTime=(double)(lastTime-firstTimeTempLSH1)/CLOCKS_PER_SEC;
				totalLSHOnlyNoteTime+=OneSongLSHTime;
				OneSongLSHTime=(double)(lastTime-firstTime)/CLOCKS_PER_SEC;
				totalLSHTime+=OneSongLSHTime;
			}
#endif

			//����Ϊͳ��LSH׼ȷ��
			FloorLevel=FloorLevelInitial;
			int sizeLSHNote=IndexCandidatesStretch.size();
			NLSHCandidateCorrect(wavename,IndexLSHNote,IndexCandidatesStretch[sizeLSHNote-1],CandidatesNumStretch[sizeLSHNote-1]);
			//LSHresultRate(wavename,queryPitch.size(),stepFactor,FloorLevel,IndexCandidatesStretch,IndexLSH,IndexLSHNote,StretchStep,IndexCandidatesDis);
			//LSHresult(wavename,queryPitch.size(),stepFactor,FloorLevel,IndexCandidatesStretch,IndexLSH,StretchStep);

			//����ΪLSH�˲�
			map<string ,int> LSHFilterMap;//�Ժ�ѡ������������<��ѡ����������Ӧ��ѡ����>
			LSHFilter(wavename,LSHFilterMap,CandidatesFilter,IndexLSH);//�õ�LSHFilterMap

#if 1

			firstTime=clock();
		
			for (int q=0;q<qRecurse;q++)//LS �������� �ٴ�ѭ��һ�� ȥ��α��ѡ
			{

				int sizeLSH=IndexCandidatesStretch.size();//���NLSH�㼯��С
				vector<float>  queryStretchPitch;
				vector< vector<float> >  CandidatesDataY;
				vector<float>  CandidatesDataYDis;
				vector <string> SongNameMapToDataY;
				int CandidatesSizeInDWT=0;//���ص�Ҫ��ȷƥ�����Ŀ
				map <string , short > SongMapPosition;

#if 0  //������frame�ĺ���������

				for	(FloorLevel=FloorLevelInitial,LinearCoe=0;FloorLevel<UpperLimit+StretchStep && LinearCoe<sizeLSH-1 &&LinearCoe< LinearCoeTotal;FloorLevel+=StretchStep,LinearCoe++)
				{
					map <string , vector<pair<short, double>> > SongMapPositionAll;
					StretchStep=0.1;
					int StepFactorCurrent=int(stepFactor*stepRatio*FloorLevel/**FloorLevel*/);

					if (q==0)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,edge,edge*2); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
					}
					else if (q==3)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,6,0); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
					}
					else if (q==4)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,-6,0); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
					}
					else if (q==1)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,4,8); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
					}
					else if (q==2)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,-4,-8); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
					}
				}	
#endif	

#if 1 //����Ϊ��������չ
				sizeLSH=IndexCandidatesStretch.size();
				map <string , vector<pair<short, double>> > SongMapPositionAll;
				map <string , vector<pair<short, double>> >::iterator iter;
				vector <string> AllCandidateSong;
				////MatchBeginPos����query��dataY���������������1/MatchBeginPos��Χ֮�ڲŽ���ƥ��
				//CandidatesSizeInDWT����Ҫ��ȷƥ�����Ŀ
				//posPair NLSH�����ʼλ�úͳ�������
				//IndexCandidatesStretch[sizeLSH-1],CandidatesNumStretch[sizeLSH-1] ��ѡ��ż���ѡ����
				//FloorLevel��ʼ����ֵ         IndexLSHNote NLSH���� NLSH�����ţ���·�����ļ�������ʼλ�ã�����֡��
				//queryPitchNote,wav����������ת����һά�������У�queryStretchPitch������������У�edge��ʼλ��ƫ����
				//��� SongNameMapToDataY ��ѡ��������CandidatesDataY NLSH���Ӧ�ĺ�ѡ�������� ��SongMapPositionAll �����ѡ������Ϣ��������ѡ���������߳��ζ�Ӧ�ĵ�ƫ��λ�ü���ʵ�ʳ��ȡ� 
				//indexSongName ����pv�ļ�·���Ͷ�Ӧ��һά��������
				IndexSignToQueryAndDataVectorHummingMatchLeastALLNote( posPair,IndexCandidatesStretch[sizeLSH-1],CandidatesNumStretch[sizeLSH-1],FloorLevel,
				IndexLSHNote,5,queryPitchNote,queryStretchPitch,CandidatesDataY,
				indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
				SongMapPositionAll,edge,edge*2);
				//��ȷ��ͳ��
				for(iter=SongMapPositionAll.begin();iter!=SongMapPositionAll.end();++iter)
				{
					AllCandidateSong.push_back(iter->first);
				}
				BALSnLSHCandidateCorrect(wavename,AllCandidateSong);

#endif	 

				lastTime=clock();
				OneSongLSHTime=(double)(lastTime-firstTimeTempLSH1)/CLOCKS_PER_SEC;
				totalLSHRetrievalPostProcessPitchTimeNote+=OneSongLSHTime;
				int sizeCandidates=SongNameMapToDataY.size();//���к�ѡ����������
				MinusMeanSmooth(queryPitchNote);////����ֵ����������ֵ��>12��-12��<-12��+12
				MinusMeanSmooth(queryStretchPitch);//����ֵ����������ֵ��>12��-12��<-12��+12

				int numLSH=0;//��ѡ���ִ���
				int thresholdMatch=0;//��ѡ���ִ�������

				firstTimeTemp=clock();
				vector <pair<float,int>> DisLS;
				vector <float> DisLSOriginal;
				for (int i=0;i<sizeCandidates;i++)
				{//�������к�ѡ����
					if (LSHFilterMap.count(SongNameMapToDataY[i]))
					{//���ں�ѡ����
						numLSH=LSHFilterMap[SongNameMapToDataY[i]];//�õ���ѡ�������ֵĴ���
						if (numLSH>thresholdMatch)//���ִ�������0
						{
							MinusMeanSmooth(CandidatesDataY[i]);
							vector <float> candidat;
							float pitch_query=0;
							int left=0;
							int right=0;
							//int Cedge=edge*(0.7+((float)i)/sizeCandidates);
							//queryStretchPitch����������� CandidatesDataY��Ӧ����������
							//����ѡ���н����������̲��� �Ƚ���߳��ξ��� ȡ������Сֵ����BALS
							float lsDis=CalculateOptimalEdge(queryStretchPitch,CandidatesDataY[i],left,right,edge*2,((float)i)/sizeCandidates);
							DisLSOriginal.push_back(lsDis);

							DisLS.push_back(make_pair(lsDis,i)); //����С���룬���ѡ��Ŵ��룬�õ����к�ѡ����С����
						}
					}

				}
				stable_sort(DisLS.begin(),DisLS.end(),sortRule);//�ȶ����򣬴�С��������

				set<int> LSResult;
				int LSLeft=DisLS.size()*LSratio;//LSratio LS�������� =0.4
				if (LSLeft<20)
				{
					LSLeft=DisLS.size();
				}
				for (int i=0;i<LSLeft;i++)
				{

					LSResult.insert(DisLS[i].second);//����ǰLSLeft��������̸����ĺ�ѡ���

				}
				lastTimeTemp=clock();
				OneSongLSTime=(double)(lastTimeTemp-firstTimeTemp)/CLOCKS_PER_SEC;
				totalLSTime+=OneSongLSTime;
				firstTimeTemp=clock();
				int disEmdNum=-1;
				lastTimeTemp=clock();
				OneSongEMDTime=(double)(lastTimeTemp-firstTimeTemp)/CLOCKS_PER_SEC;
				totalEMDTime+=OneSongEMDTime;
				firstTimeTemp=clock();

				for (int i=0;i<sizeCandidates;i++)
				{//�������к�ѡ����
					numLSH=LSHFilterMap[SongNameMapToDataY[i]];//�õ���ѡ�������ֵĴ���
					if (numLSH>thresholdMatch)
					{
						disEmdNum++;//�õ���ѡ�������ִ�������0�ĸ�����Ŀ
					}

					if (query.n==0 || (LSHFilterMap.count(SongNameMapToDataY[i]) && LSResult.count(i)))
					{

						if (numLSH>thresholdMatch)
						{
							MinusMeanSmooth(CandidatesDataY[i]);
							int QuerySize=queryStretchPitch.size();
							int DataYSize=CandidatesDataY[i].size();
							int sizeQandD=CandidatesDataY[i].size();
							int ultimateNum=0;
							float distanceM=10000000;
							float distanceF=10000000;
							float distanceM1=10000000;
							float distanceM2=10000000;
							float distanceM3=10000000;

							if (0/*recur==10*/)
							{
								MeanPlus(CandidatesDataY[i],0);
								distanceM1=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
							}
							else
							{
								for (int k=0;k<5;k++)
								{//ѭ�����KTRA�㷨
									ultimateNum=0;
									float plus=(float(k-2))/2;
									MeanPlus(CandidatesDataY[i],plus);//����ֵ����ȡ��ֵ=mean+plus
									////KTRA������С����
									distanceF=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
									distanceM1=MyMinTwo(distanceM1,distanceF);//ȡ��С����ֵ
								}
							}
						
							tempDis1.push_back(distanceM1);//ȡ��С����ֵ
							for (int k=0;k<0;k++)
							{//û��ѭ��
								ultimateNum=0;
								float plus=(float(k-2.5))/2;
								MeanPlus(CandidatesDataY[i],plus);
								distanceF=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
								distanceM2=MyMinTwo(distanceM2,distanceF);
							}
							if (QuerySize<2)
							{
								QuerySize=2;
							}
							distanceM2=((float)DataYSize)/QuerySize;
							distanceM2=abs(distanceM2-1);
							tempDis2.push_back(distanceM2);
							ultimateNum=0;
							MeanPlus(CandidatesDataY[i],0);
							
							distanceM3=((float)DataYSize)/QuerySize;
							if (distanceM3>1)
							{
								distanceM3=distanceM3-1;
							}
							else
								distanceM3=(1-distanceM3)*2;
							tempDis3.push_back(distanceM3);
							
							distanceM=distanceM1*0.8+distanceM2*0.2;//����������

							Dis.push_back(distanceM);//�������RA����
							string songName=SongNameMapToDataY[i];

							if (songDis.count(distanceM))
							{//���Ѿ����ڽ�������0.00001���Ա�ʾ����
								*(Dis.end()-1)+=0.00001;
								songDis.insert(make_pair(distanceM+0.00001,songName)); 
							}
							else
								songDis.insert(make_pair(distanceM,songName)); //��¼��ѯ�����ѡ����̾��� ��ѡ�θ�����
							songName.erase(0,6);
							int posSong=songName.rfind("pv");
							songName.erase(posSong-1,3);//�õ�ȥ��·����.pv������������
							tempList.push_back(songName);
						}
					}
				}

				lastTimeTemp=clock();
				OneSongRATime=(double)(lastTimeTemp-firstTimeTemp)/CLOCKS_PER_SEC;
				totalRATime+=OneSongRATime;
				CandidatesDTWAll+=Dis.size();
				string nameSong(wavename);

				string nameSongResult;
				stable_sort(Dis.begin(),Dis.end());//�ȶ����� ��С����
				set <string> SongFindAlready;
				float ratio=0.0;
				string songNameFive;

				for (int i=0;i!=songDis.size()&&i<=1000 ;i++)
				{//���ѭ��1000��
					songNameFive=songDis.find(Dis[i])->second;
					if (SongFindAlready.count(songNameFive))
					{
						;
					}
					else
					{

						SongFindAlready.insert(songNameFive);//���뵽���ҵ��ĸ����б���
					}
					if (SongFindAlready.size()==2)
					{//��ѡ������Ϊ2  �������Ŷȼ���

						if (Dis[0]/Dis[i]<=ratio)
						{//���Ŷȼ���  ratio=0.0;  ��ֻ�ܵ���0  ���Ѵﵽ
							returnN=true;
							recur+=3;
						}
						i+=10000;//����ѭ��
					}
				}
				if (returnN==true)
				{		
					nameSongResult.assign(nameSong,0,nameSong.size()-4);
					ResultDis1.insert(make_pair(nameSongResult,tempDis1));
					ResultDis2.insert(make_pair(nameSongResult,tempDis2));
					ResultDis3.insert(make_pair(nameSongResult,tempDis3));
					ResultLable1.insert(make_pair(nameSongResult,tempList));
				}
			}
		} //���������ļ����Ľ�β���õ��������������ĺ�ѡ������������

		lastTimeTempLSH1=clock();
		OneSonglshnoteTime=(double)(lastTimeTempLSH1-firstTimeTempLSH1)/CLOCKS_PER_SEC;
		totalLSHNoteTime+=OneSonglshnoteTime;

		if (recur==0 || recur==3)
		{
			lastTimeTempLSH1=clock();
			OneSonglshPitchTime=(double)(lastTimeTempLSH1-firstTimeTempLSH1)/CLOCKS_PER_SEC;
			totalLSHNoteFirstTime+=OneSonglshPitchTime;
		}

		if (returnN==false)
		{//��պ�ѡ����
			firstTimeTempLSH2=clock();
			IndexCandidatesStretch.clear();
			IndexCandidatesDis.clear();
			CandidatesNumStretch.clear();
			CandidatesFilter.clear();

#if 1 //����Ϊ֡��LSH����

			LinearCoe=0;
			FloorLevel=FloorLevelInitial;//��recur=0ʱ Ϊ1
			int LSHSizeDefine;
			LSHSizeDefine=LSHQueryVectorLinearStretching.size();

			int candidateSize;
			queryResultFile >> candidateSize;	//��һ��Ϊÿ����ѯ�ĺ�ѡ������ÿ������

			//samePointStream<<LSHSizeDefine<<endl;	//�����ѡ��
			
			
			
			for (;FloorLevel<UpperLimit+StretchStep && LinearCoe< LinearCoeTotal;FloorLevel+=StretchStep)
			{
				//������ͬ���������������LSH�㼯 ��recur=0ʱ ֻѭ��һ��
				PPointT *QueriesArray=NULL;//Ҫfree
				
				if ((recur==1 || recur==2 )&& FloorLevel<=1.01 && FloorLevel>=0.99)
				{
					continue;
				}
				if (recur==2 && ((FloorLevel<=0.81 && FloorLevel>=0.79)||(FloorLevel<=1.21 && FloorLevel>=1.19)||(FloorLevel<=1.41 && FloorLevel>=1.39)))
				{
					continue;
				}

				//LSHQueryVectorLinearStretching��LS���LSH�㣬ÿ����Ϊһ���������У�vector[i][j][k]��ʾ��i�����������µĵ�j��������ĵ�k������
				if (LSHQueryVectorLinearStretching[LinearCoe].size()>0)	//��ǰ����������������õ��ĵ㼯
				{
					QueriesArray=readDataSetFromVector(LSHQueryVectorLinearStretching[LinearCoe]);	//����LSH�㼯
					Int32T nPointsQuery=LSHQueryVectorLinearStretching[LinearCoe].size();	//�õ��㼯��С
					IntT dimension=20;
					if (nPointsQuery>0)
					{
						dimension=LSHQueryVectorLinearStretching[LinearCoe][0].size();	//�õ�LSH����ʵά��
					}
					IntT LSHFilterReturnNum=0;
					IntT *IndexArray=NULL;
					int IndexArraySize=500000;//�����Һ�ѡ��Ŀ

					IndexArray=(IntT *)MALLOC(IndexArraySize *sizeof(IntT));
					double *IndexArrayDis=(double *)MALLOC(IndexArraySize *sizeof(double));
					IntT *IndexFilterArray=NULL;
					IndexFilterArray=(IntT *)MALLOC(IndexArraySize/2 *sizeof(IntT));
					IntT * NumArray=(IntT *)MALLOC(nPointsQuery *sizeof(IntT));//ÿ���㷵�صĺ�ѡ��Ŀ

					if(!isUseDBNResult)
					{
						//�õ�LSH���������ÿ��LSH�����RNN��ѯ������RetainNum����ѡ����󷵻����е�ĺ�ѡ��Ŀ
						//���룺QueriesArray,��ǰLSH�㼯��nPointsQuery����ǰ�㼯��С
						//IndexArraySize�������Һ�ѡ��
						//IndexHuming��LSH���RNN������RetainNum��LSHÿ����������ĵ���
						//dimension��LSH��ά����LSHFilterNum��LSH�˲������ĵ���
						//�����
						//IndexArray����ѡ��ţ�NumArray��ÿ���㷵�صı�����Ŀ  IndexArrayDis�����к�ѡ��Ӧ�ľ���
						//IndexFilterArray�����к�ѡ��ţ�sizeFilter����ѡ��С
						IntT ResultSize=LSHStructToResultOnePointRetainSeveral(QueriesArray,nPointsQuery,IndexArraySize, 
							IndexArray,IndexHuming,NumArray,RetainNum,dimension,LSHFilterNum,IndexFilterArray,LSHFilterReturnNum,IndexArrayDis);//�õ������ÿ���㷵��RetainNum����ѡ
					}
					else
					{
						if(queryResultFile.eof())	//���ļ�β������
						{
							printf("DBN��ѯ����ļ�query_result.txt���ݲ��㣡\n");
							break;
						}
						for(int i=0; i<nPointsQuery*candidateSize; i++)	//���뵱ǰ��ѯ�㼯�ĺ�ѡ�;���
						{
							queryResultFile >> IndexArray[i] >> IndexArrayDis[i];
						}
						for(int i=0; i<nPointsQuery; i++)
						{
							NumArray[i] = candidateSize;
						}
					}

					vector<IntT> IndexCandidates;
					vector <float> DisCandidates;
					vector<IntT> CandidatesNum;//ÿ���㷵�ص���Ŀ����Vector
					int curreIndex=0;
					
					for (int i=0;i<LSHFilterReturnNum;i++)
					{
						//���ص�ÿ��������к�ѡ��ž����뵽CandidatesFilter��
						CandidatesFilter.push_back(IndexFilterArray[i]);
					}
					bool insertY=false;
					for (int i=0;i<nPointsQuery;i++)
					{//������ǰLSH�㼯
						int numS=0;
						int siz=NumArray[i];//ÿ���㱣���ĺ�ѡ����
						for (int j=0;j<siz;j++)
						{
							if (samePoint.count(IndexArray[j+curreIndex]))
							{
								//�鿴��ѡ��Ŵ��ڣ�����0��1�����Խ����ʾ��samepointû����ֵ����������
								numS=0;

								samePointIte=samePoint.find(IndexArray[j+curreIndex]);
								numS=samePointIte->second.size();
								IndexCandidates.push_back(IndexArray[j+curreIndex]);
								DisCandidates.push_back(IndexArrayDis[j+curreIndex]);
								for (int k=0;k<samePointIte->second.size();k++)
								{
									insertY=true;
									for (int l=0;l<siz;l++)
									{
										if (samePointIte->second[k]==IndexArray[l+curreIndex])
										{
											insertY=false;
										}
									}
									if (insertY==true)
									{
										IndexCandidates.push_back(samePointIte->second[k]);
										DisCandidates.push_back(IndexArrayDis[j+curreIndex]);
									}
									else
										numS--;
								}
								NumArray[i]+=numS;
							}
							else
							{//�鿴��ѡ��Ų����ڣ������ѡ��ż�����
								IndexCandidates.push_back(IndexArray[j+curreIndex]);//�����ѡ���
								DisCandidates.push_back(IndexArrayDis[j+curreIndex]);//�����ѡ��Ӧ�ľ���
							}
						}

						curreIndex+=siz;
						CandidatesNum.push_back(NumArray[i]);//ÿ���㱣���ĺ�ѡ��Ŀ
					}
					//׼ȷ��ͳ��

					LSHCandidateCorrect(wavename,IndexLSH,IndexCandidates,CandidatesNum,allStretchCandidates,allCorrectCan);
				
					IndexCandidatesStretch.push_back(IndexCandidates);//���뵱ǰ�㼯�����к�ѡ���
					CandidatesNumStretch.push_back(CandidatesNum);//ÿ���㱣���ĺ�ѡ��Ŀ
					IndexCandidatesDis.push_back(DisCandidates);//�����ѡ��Ӧ�ľ���

					for (int i=0;i<nPointsQuery;i++)
					{//�ͷſռ�
						free(QueriesArray[i]->coordinates);
						free(QueriesArray[i]);
					}
					free(IndexArray);
					free(IndexFilterArray);
					free(NumArray);
					free(QueriesArray);
					free(IndexArrayDis);
				}
				else//�����󲻴��ڵ㼯
				{//ͬ������ ʵ��Ϊ�� ���ý��м���Ŷ
					vector <float> DisCandidates;
					vector<IntT> IndexCandidates;
					vector<IntT> CandidatesNum;//ÿ���㷵�ص���Ŀ����Vector
					IndexCandidatesStretch.push_back(IndexCandidates);//�����ѡ���
					CandidatesNumStretch.push_back(CandidatesNum);//ÿ���㱣���ĺ�ѡ��Ŀ
					IndexCandidatesDis.push_back(DisCandidates);//�����ѡ��Ӧ�ľ���

				}
				LinearCoe++;

			}//������ͬ���������������LSH�㼯 ���� �õ���ѡ�㼯  ͳ����ȷ��ѡ����
			
#endif

			lastTimeTempLSH2=clock();
			OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH2)/CLOCKS_PER_SEC;
			totalLSHOnlyPitchTime+=OneSonglshPitchTime;


			for (int q=0;q<qRecurse;q++)
			{//��ѭ��һ�� ȥ��α��ѡ
				int sizeLSH=IndexCandidatesStretch.size();//�õ�������������
				vector<float>  queryStretchPitch;//���������������
				vector< vector<float> >  CandidatesDataY;//��ѡ��������
				vector<float>  CandidatesDataYDis;//��ѡ����
				vector <string> SongNameMapToDataY;//��ѡ������
				int CandidatesSizeInDWT=0;//���ص�Ҫ��ȷƥ�����Ŀ
				map <string , short > SongMapPosition;

#if 1  //������frame�ĺ���
				for	(FloorLevel=FloorLevelInitial,LinearCoe=0;FloorLevel<UpperLimit+StretchStep && LinearCoe<sizeLSH &&LinearCoe< LinearCoeTotal;FloorLevel+=StretchStep)
				{
					map <string , vector<pair<short, double>> > SongMapPositionAll;
					map <string , vector<pair<short, double>> >::iterator iter;
					vector <string> AllCandidateSong;
					if ((recur==1 || recur==2 )&& FloorLevel<=1.01 && FloorLevel>=0.99)
					{
						continue;
					}
					if (recur==2 && ((FloorLevel<=0.81 && FloorLevel>=0.79)||(FloorLevel<=1.21 && FloorLevel>=1.19)||(FloorLevel<=1.41 && FloorLevel>=1.39)))
					{
						continue;
					}
					int StepFactorCurrent=int(stepFactor*stepRatio*FloorLevel/**FloorLevel*/);//�������֡��

					if (q==0)
					{
						int beforeSize=SongNameMapToDataY.size();
						//�����������ӷ��غ�ѡ�����Լ���Ӧ�ĸ�������
						////MatchBeginPos����query��dataY���������������1/MatchBeginPos��Χ֮�ڲŽ���ƥ��
						//CandidatesSizeInDWT����Ҫ��ȷƥ�����Ŀ
						//IndexCandidatesStretch[sizeLSH-1],CandidatesNumStretch[sizeLSH-1] ��ѡ��ż���ѡ����
						//FloorLevel��ʼ����ֵ         IndexLSH //LSH��������¼LSH�����ţ���·�����ļ�������ʼλ��
						//queryPitch,wav����������ת����һά�������У�queryStretchPitch�������������
						//indexSongName ����pv�ļ�·���Ͷ�Ӧ��һά��������
						//��� SongNameMapToDataY ��ѡ��������dataY LSH���Ӧ�ĺ�ѡ�������� ��
						//SongMapPositionAll �����ѡ������Ϣ��������ѡ���������������Ӧ����ʼλ�ü��������ӡ�offsetbegin��ʼλ��ƫ���� 
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,edge,edge*2); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
						int afterSize=SongNameMapToDataY.size();

						//׼ȷ��ͳ��
						for(int k=beforeSize;k<afterSize;k++)
						{
							AllCandidateSong.push_back(SongNameMapToDataY[k]);
						}
						
						BALSLSHCandidateCorrect(wavename,AllCandidateSong);
					}
					else if (q==3)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,6,0); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
					}
					else if (q==4)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,-6,0); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
					}
					else if (q==1)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,4,8); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
					}
					else if (q==2)
					{
						IndexSignToQueryAndDataVectorHummingMatchLeastALL( IndexCandidatesStretch[LinearCoe],CandidatesNumStretch[LinearCoe],FloorLevel,
							IndexLSH,StepFactorCurrent,queryPitch,queryStretchPitch,CandidatesDataY,
							indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
							SongMapPositionAll,-4,-8); //��Ϊȫ��ƥ�䣨���Ǵ�ͷ��ʼ����
					}
					LinearCoe++;
				}

				lastTimeTempLSH2=clock();
				OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH2)/CLOCKS_PER_SEC;
				totalLSHRetrievalPostProcessPitchTime+=OneSonglshPitchTime;

#endif	

#if 0 //����Ϊ��������չ
				sizeLSH=IndexCandidatesStretch.size();
				map <string , vector<pair<short, double>> > SongMapPositionAll;
				IndexSignToQueryAndDataVectorHummingMatchLeastALLNote( posPair,IndexCandidatesStretch[sizeLSH-1],CandidatesNumStretch[sizeLSH-1],FloorLevel,
					IndexLSHNote,5,queryPitchNote,queryStretchPitch,CandidatesDataY,
					indexSongName,SongNameMapToDataY,CandidatesSizeInDWT,MatchBeginPos,SongMapPosition,
					SongMapPositionAll,edge,edge*2);
#endif	 

				int sizeCandidates=SongNameMapToDataY.size();//��ȡ��ѡ������С
				MinusMeanSmooth(queryPitchNote);//����ֵ
				MinusMeanSmooth(queryStretchPitch);//����ֵ
				int numLSH=0;
				int thresholdMatch=0;

				firstTimeTemp=clock();
				vector <pair<float,int>> DisLS;
				vector <float> DisLSOriginal;
				for (int i=0;i<sizeCandidates;i++)
				{
					MinusMeanSmooth(CandidatesDataY[i]);
					vector <float> candidat;
					float pitch_query=0;
					int left=0;
					int right=0;
					//queryX��������������� DataY��Ӧ�ĺ�ѡ�������� length=12  ������ѡ�߽� BALS ����ѡ���н����������̲��� �Ƚ���߳��ξ��� ȥ������Сֵ
					float lsDis=CalculateOptimalEdge(queryStretchPitch,CandidatesDataY[i],left,right,edge*2,((float)i)/sizeCandidates);
					DisLSOriginal.push_back(lsDis);//������С����ֵ
					DisLS.push_back(make_pair(lsDis,i)); //������С����ֵ����Ӧ�ĺ�ѡ�������
				}

				stable_sort(DisLS.begin(),DisLS.end(),sortRule);//�ȶ����� ��С����

				set<int> LSResult;
				int LSLeft=DisLS.size()*LSratio;//LSratio=0.4  LS�������������µĺ�ѡ
				if (LSLeft<20)
				{
					LSLeft=DisLS.size();
				}
				for (int i=0;i<LSLeft;i++)
				{//ȡ��ѡ����ǰLSLeft����С��ѡ
					LSResult.insert(DisLS[i].second);//�����Ӧ���������
				}

				lastTimeTemp=clock();
				OneSongLSTime=(double)(lastTimeTemp-firstTimeTemp)/CLOCKS_PER_SEC;
				totalLSTime+=OneSongLSTime;
				firstTimeTemp=clock();
				int disEmdNum=-1;
				lastTimeTemp=clock();
				OneSongEMDTime=(double)(lastTimeTemp-firstTimeTemp)/CLOCKS_PER_SEC;
				totalEMDTime+=OneSongEMDTime;
				firstTimeTemp=clock();

				for (int i=0;i<sizeCandidates;i++)
				{//�������к�ѡ����
					disEmdNum++;

					if ( LSResult.count(i))
					{//��̾��������д��ں�ѡ������Ӧ�����
						MinusMeanSmooth(CandidatesDataY[i]);//��ѡ�������м���ֵ
						int QuerySize=queryStretchPitch.size();//������Ĳ�ѯ�������г���
						int DataYSize=CandidatesDataY[i].size();//��ѡ�������г���
						int sizeQandD=CandidatesDataY[i].size();
						int ultimateNum=0;
						float distanceM=10000000;
						float distanceF=10000000;
						float distanceM1=10000000;
						float distanceM2=10000000;
						float distanceM3=10000000;

						if (0/*recur==10*/)
						{
							MeanPlus(CandidatesDataY[i],0);
							distanceM1=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
						}
						else
						{
							for (int k=0;k<5;k++)
							{
								ultimateNum=0;
								float plus=(float(k-2))/2;
								MeanPlus(CandidatesDataY[i],plus);//����ֵ����ȡ��ֵ=mean+plus
								////KTRA������С����
								distanceF=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
								distanceM1=MyMinTwo(distanceM1,distanceF);//ȡ��С����ֵ
							}
						}
						tempDis1.push_back(distanceM1);
						for (int k=0;k<0;k++)
						{
							ultimateNum=0;
							float plus=(float(k-2.5))/2;
							MeanPlus(CandidatesDataY[i],plus);
							distanceF=RAPositionVarianceOptimal(queryStretchPitch,CandidatesDataY[i],3,5,1,ultimateNum)/ultimateNum;
							distanceM2=MyMinTwo(distanceM2,distanceF);
						}
						if (QuerySize<2)
						{
							QuerySize=2;
						}
						distanceM2=((float)DataYSize)/QuerySize;//��ѡ�������ѯ������������
						distanceM2=abs(distanceM2-1);
						tempDis2.push_back(distanceM2);
						ultimateNum=0;
						MeanPlus(CandidatesDataY[i],0);

						distanceM3=((float)DataYSize)/QuerySize;
						if (distanceM3>1)
						{
							distanceM3=distanceM3-1;
						}
						else
							distanceM3=(1-distanceM3)*2;

						tempDis3.push_back(distanceM3);
						distanceM=distanceM1*0.8+distanceM2*0.2;//������������̾���

						Dis.push_back(distanceM);//������̾���
						string songName=SongNameMapToDataY[i];//�õ���ѡ������

						if (songDis.count(distanceM))
						{//������̾����Ѿ����� ������0.00001����ʾ����
							*(Dis.end()-1)+=0.00001;
							songDis.insert(make_pair(distanceM+0.00001,songName)); 
						}
						else
							songDis.insert(make_pair(distanceM,songName)); //�洢���RA���뼰��Ӧ�ĸ�����
						songName.erase(0,6);
						int posSong=songName.rfind("pv");
						songName.erase(posSong-1,3);
						tempList.push_back(songName);//�õ�ȥ��·������չ���ĸ�����
					}
				}
				lastTimeTemp=clock();
				OneSongRATime=(double)(lastTimeTemp-firstTimeTemp)/CLOCKS_PER_SEC;
				totalRATime+=OneSongRATime;
				CandidatesDTWAll+=Dis.size();
				string nameSong(wavename);

				string nameSongResult;

				stable_sort(Dis.begin(),Dis.end());//����̾�������
				set <string> SongFindAlready;

				float ratio=110.00;
				if (recur==10)
				{
					ratio=1.59;
				}
				else if (recur==11)
				{
					ratio=1.47;
				}
				string songNameFive;
				float Thr[1000]={0.0};
				int num=0;
					
				for (int i=0;i!=songDis.size()&&i<=1000 ;i++)
				{//�����ѡ����
					songNameFive=songDis.find(Dis[i])->second;//�ҵ���̾����Ӧ�ĸ�����
					if (SongFindAlready.count(songNameFive))
					{//���ҵ��ĺ�ѡ�������Ѵ��ڴ˸��� �򲻲���
						;
					}
					else
					{//������ �����˺�ѡ����
						num++;//����ĺ�ѡ��������
						SongFindAlready.insert(songNameFive);
						if (SongFindAlready.size()==num && num<1000)
						{
							Thr[num-1]=Dis[i];
							float totalDis=0;
							for (int l=1;l<num;l++)
							{
								totalDis+=Thr[l];//���о����
							}
							if (SongFindAlready.size()==5)
							{//������ҵ��ĸ�������Ϊ5

								if ( totalDis/(Dis[0]*(num-1)) >=ratio)
								{//���Ŷȼ��� ������ʴ�����ֵ110
									returnN=true;
									recur+=3;//��ʾ�����˲�ѯѭ�������ҵ�Ŀ�����
								}
								i+=10000;//������ǰѭ��
							}
						}
					}
				}

				if (returnN==true || recur>=2)
				{		
					nameSongResult.assign(nameSong,0,nameSong.size()-4);
					ResultDis1.insert(make_pair(nameSongResult,tempDis1));
					ResultDis2.insert(make_pair(nameSongResult,tempDis2));
					ResultDis3.insert(make_pair(nameSongResult,tempDis3));
					ResultLable1.insert(make_pair(nameSongResult,tempList));
				}
			}

			if (recur==0 || recur==3)
			{
				lastTimeTempLSH2=clock();
				OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH2)/CLOCKS_PER_SEC;
				totalLSHPitchFirstTime+=OneSonglshPitchTime;
			}

			lastTimeTempLSH2=clock();
			OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH2)/CLOCKS_PER_SEC;
			totalLSHPitchTime+=OneSonglshPitchTime;
		}
		if (recur==0 || recur==3)
		{
			lastTimeTempLSH2=clock();
			OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH1)/CLOCKS_PER_SEC;
			totalLSHoneTime+=OneSonglshPitchTime;
		}
		else if (recur==1 || recur==4)
		{
			lastTimeTempLSH2=clock();
			OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH1)/CLOCKS_PER_SEC;
			totalLSHtwoTime+=OneSonglshPitchTime;
		}
		else if (recur==2 || recur==5)
		{
			lastTimeTempLSH2=clock();
			OneSonglshPitchTime=(double)(lastTimeTempLSH2-firstTimeTempLSH1)/CLOCKS_PER_SEC;
			totalLSHthreeTime+=OneSonglshPitchTime;
		}
	}//for��recurѭ������
	vector<IntT>::iterator allStrCanBen;
	vector<IntT>::iterator allStrCanEnd;
	vector<IntT>::iterator allCorCanBen;
	vector<IntT>::iterator allCorCanEnd;
	allStrCanBen=allStretchCandidates.begin();
	allStrCanEnd=allStretchCandidates.end ();
	allCorCanBen=allCorrectCan.begin();
	allCorCanEnd=allCorrectCan.end();

	//��ȷ��ͳ��				
	int sumNum=0;
	int sumCorNum=0;
	float corCanRate=0;
	for(;allStrCanBen!=allStrCanEnd;++allStrCanBen)
	{
		
		sumNum+=*allStrCanBen;
	}
	for(;allCorCanBen!=allCorCanEnd;++allCorCanBen)
	{
		
		sumCorNum+=*allCorCanBen;
	}

	filename<<"���к�ѡ��"<<sumNum<<endl;
	filename<<"������ȷ��ѡ��"<<sumCorNum<<endl;
	if(sumNum>0&&sumCorNum>0)
	{
		corCanRate=(float)sumCorNum/sumNum;
		exitCandidateNum++;
	}
	else
		corCanRate=0;
	filename<<"����ȷ�ʣ�"<<corCanRate<<endl;
	filename<<"���ں�ѡ����Ŀ��"<<exitCandidateNum<<endl;
	filename.close();

	
	if(NULL!=NoteEmd){
		free (NoteEmd);
		NoteEmd=NULL;
		query.Features=NULL;
	}
	if(NULL!=NoteDuration){
		free (NoteDuration);
		NoteDuration=NULL;
		query.Weights=NULL;
	}
	if(NULL!=candiY.Features){
		free (candiY.Features);
		candiY.Features=NULL;
	}
	if(NULL!=candiY.Weights){
		free (candiY.Weights);
		candiY.Weights=NULL;
	}
	
	//if (total%500==0 || total >310)
	//{
	//	ofstream outTime("wav.result",ofstream::app);//LSHʱ��
	//	outTime<<" ȫ������RAʱ�䣺"<<totalRATime<<"EMDʱ�䣺"<<totalEMDTime<< "LSʱ�䣺"<<totalLSTime<<endl;
	//	cout<<" ȫ������RAʱ�䣺"<<totalRATime<<"EMDʱ�䣺"<<totalEMDTime<< "LSʱ�䣺"<<totalLSTime<<endl;
	//	outTime.close();
	//}

	stable_sort(Dis.begin(),Dis.end());
	string   songNameFive;
	ofstream outf("wav.result",ofstream::app);//ʶ���������ļ�
	ofstream outfCandidates("wavCandidates.result",ofstream::app);//��ѡ��������ļ�

	static int num1=0;
	static int num2=0;
	static int num3=0;
	static int num4=0;
	static int num5=0;
	static int num10=0;
	static int num20=0;
	static int num50=0;
	static int findall=0;
	static float thresholdC=10;
	static float dis1=0;
	static float dis2=0;
	static string tempSt;
	
	static int rightNum[1101]={0};
	static int wrongNum[1101]={0};
	int findtop5=0;
	bool wrongA=TRUE;
	static map <string,int> resultMap;
	set <string> SongFindAlready;
	string firstSong;
	int findNum=0;	//�����������Ŀ�Լ�ͳ�Ƶڼ����ҵ���
	float ThrCal[1000]={0.0};
	int numCal=0;
	static float ThrRes[1000]={0.0};

	for (int i=0,k=0;i!=songDis.size()&&k<5 && i!= Dis.size() && k<500;i++)
	{
		if (songDis.count(Dis[i]))
		{
			songNameFive=songDis.find(Dis[i])->second;
		
			if (SongFindAlready.count(songNameFive))
			{
				;
			}
			else
			{
				k++;
			
				SongFindAlready.insert(songNameFive);
				int posSong=songNameFive.rfind("pv");
				songNameFive.erase(posSong-1,3);
				songNameFive.erase(0,6);
		
				songFive.push_back(songNameFive);//������
			}
		}
	}
	while (songFive.size()<5)
	{
		songFive.push_back("NONE");
	}
	outf.close();
#endif	
	return 0;
}

//������Ϊ�������������Ϊ5000newpv355.txt �е�pv�ļ��������������ļ����ڳ�������Ŀ¼/5355P�ļ����У�
//�����param��������Ϣ
//songIDAndName������ID�͸���ӳ��
int indexRead(ParamInfo *param,	map <string ,string> &songIDAndName)
{
	string fileName("wavall355.txt");	

	
	/*
	//���߳��·ָ�����
	int ThreadNum=1;
	if (ThreadNum>=100)
	{
		ThreadNum=100;//���100���߳�
	}
	OneFileToMultiFile(fileName,ThreadNum);	//�����߳����ָ��ļ�
	*/

	string dataIndex="5000newpv355.txt";	//pv�б��ļ�������./5355P�е�pv�ļ�����������
	vector<vector<float>> LSHVector;	//��¼LSH�㣬ÿ����Ϊһ����������

	//LSH���ȡ����
	int StepFactor = 3;	//ѡ��LSH��ļ����������������ȡһ���㣩
	int LSHsize = 20;	//LSH������һ��LSH��Ĵ�С��20֡*3��2.4��
	int LSHshift = StepFactor*5;	//LSH����,15֡��0.6��
	int maxFrame = StepFactor*9*120*5;	//һά�����������֡��

	//NLSH���ȡ����
	int noteMaxFrame = 10;	//һ�����ߵ��֡�����������з֣�
	int NLSHsize = 10;	//NLSH������һ��LSH��Ĵ�С

	//����֡�����������Ĳ����б�
	strcpy(param[0].wavename,fileName.c_str());
	strcpy(param[1].wavename,fileName.c_str());
	param[0].RetainNum = 3;		//ÿ����������ĵ���
	param[1].RetainNum = 5;	
	param[0].stepFactor = StepFactor;	//ѡ��LSH��ļ����
	param[1].stepFactor = StepFactor;
	param[0].LSHFilterNum = 10;	//LSH�˲������ĵ���
	param[1].LSHFilterNum = 200;
	param[0].stepRatio = 1.5;	//��ѯ��LSH�㴰��
	param[1].stepRatio = 1.5;
	param[0].LSHsize = LSHsize;	//LSH���ά��
	param[1].LSHsize = NLSHsize;
	param[0].LSHshift = LSHshift;	//LSH�㴰��
	param[1].LSHshift = 1;
	param[0].noteMinFrame = 3;	//�������֡��
	param[1].noteMinFrame = 3;
	param[0].noteMaxFrame = 10;	//�����֡��
	param[1].noteMaxFrame = 10;
	param[0].maxFrame = maxFrame;	//���֡��
	param[1].maxFrame = maxFrame;

	//�洢LSH����м����
	PPointT *dataSet=NULL;	//�洢LSH�㣬���ڽ���IndexHumming���ݽṹ
	PPointT *dataSetNote=NULL;	//�洢NLSH�㣬���ڽ���IndexHumming���ݽṹ

	//����pv�ļ�������ά��(����������ʱ��)����ת��Ϊһά��������
	//���룺dataIndex: ���������б��ļ�
	//�����indexSongName: �ļ�·����5355P\\XXX.pv���Ͷ�Ӧpv�ļ���һά��������
	readIndexPitch8MinutesNewPv(dataIndex, param[0].indexSongName);	
	
	//��һά�������г�ȡLSH�㣬ȥ����ƥ�俪ͷ�Ĺ���
	//���룺indexSongName��pv�ļ����Ͷ�Ӧ��һά��������
	//StepFactor��ѡ��LSH��ļ�����������֡��ȡһ����
	//LSHsize��LSH������һ��LSH��Ĵ�С
	//LSHshift��LSH����
	//maxFrame��һά�����������֡��
	//�����LSHVector��LSH�㣬ÿ����Ϊһ���������У��洢����pv�ļ���LSH��
	//IndexLSH,	LSH��������¼��LSHVector����ţ���·�����ļ�������ʼλ��
	IndexPitchToLSHVector(param[0].indexSongName, StepFactor, LSHsize, LSHshift, maxFrame, 
		LSHVector, param[0].IndexLSH);
		
	//��LSH��д���ļ�
	//���룺LSHVector��LSH�㣬ÿ����Ϊһ����������
	//filename������ļ�·��
	LSHVectorToFile(LSHVector,"LSHVector.txt");
	//��LSH����д���ļ�
	//���룺IndexLSH��LSH��������¼��LSHVector����ţ���·�����ļ�������ʼλ��
	//filename������ļ�·������һ������ļ�����ͳ��ÿ�������ļ���������
	IndexLSHToFile(param[0].IndexLSH,"LSHIndex.txt","LSHCounter.txt");

	//��LSH��д���ļ�
	//���룺LSHVector��LSH�㣬ÿ����Ϊһ����������
	//filename������ļ�·��
	LSHVectorToFile(LSHVector,"LSHVector.txt");
	//��LSH����д���ļ�
	//���룺IndexLSH��LSH��������¼��LSHVector����ţ���·�����ļ�������ʼλ��
	//filename������ļ�·������һ������ļ�����ͳ��ÿ�������ļ���������
	IndexLSHToFile(param[0].IndexLSH,"LSHIndex.txt","LSHCounter.txt");

	//��LSH���������ж����ݼ�������dataSet�У�����ÿ��LSH��ʱ��¼��ź�LSH���ƽ����
	dataSet = readDataSetFromVector(LSHVector);

	//����MIT��r-NN��Դ�����������
	Int32T nPointsData = LSHVector.size();	//LSH�㼯��С
	IntT dimension = LSHsize;	//ÿ����ά������LSH����
	IntT nSampleQueries = 100;	//������������
	float thresholdR = 18;		//����
	MemVarT memoryUpperBound = 600000000;	//�ڴ�����

	if (nPointsData>0)
	{
		dimension = LSHVector[0].size();	//��¼��ʵά��
	}
	LSHVector.clear();	//������м�����������Ѷ���dataSet��

	//����MIT��r-NN��Դ�����ʼ��LSH���ݽṹ
	//���룺dataSet��LSH�㼯
	//dimension��ÿ��LSH��ά��
	//nPointsData��LSH�㼯��С
	//nSampleQueries��������������
	//thresholdR������
	//memoryUpperBound���ڴ�����
	//�����IndexHuming��RNN����
	LSHDataStruct(dataSet, dimension, nPointsData, nSampleQueries,
		thresholdR, memoryUpperBound, param[0].IndexHuming);
	
	//��һά�������г�ȡ����������LSH��
	//���룺indexSongName��pv�ļ����Ͷ�Ӧ��һά��������
	//noteMaxFrame��һ�����ߵ��֡�����������з֣�
	//NLSHsize��LSH������һ��LSH��Ĵ�С
	//maxFrame��һά�����������֡��
	//�����LSHVector��LSH�㣬ÿ����Ϊһ��10ά����������
	//IndexLSH,	NLSH��������¼��LSHVector����ţ���·�����ļ�������ʼλ�ã�����֡��
	IndexPitchToLSHVectorNote(param[0].indexSongName, noteMaxFrame, NLSHsize, maxFrame, 
		LSHVector, param[1].IndexLSHNote);
		
	//��LSH��д���ļ�
	//���룺LSHVector��LSH�㣬ÿ����Ϊһ����������
	//filename������ļ�·��
	LSHVectorToFile(LSHVector,"NLSHVector.txt");
	//��NLSH����д���ļ�
	//���룺IndexLSH��NLSH��������¼��LSHVector����ţ���·�����ļ�������ʼλ�ã�����֡��
	//filename������ļ�·������һ������ļ�����ͳ��ÿ�������ļ���������
	IndexLSHNoteToFile(param[1].IndexLSHNote,"NLSHIndex.txt","NLSHCounter.txt");

	//��LSH��д���ļ�
	//���룺LSHVector��LSH�㣬ÿ����Ϊһ����������
	//filename������ļ�·��
	LSHVectorToFile(LSHVector,"NLSHVector.txt");
	//��NLSH����д���ļ�
	//���룺IndexLSH��NLSH��������¼��LSHVector����ţ���·�����ļ�������ʼλ�ã�����֡��
	//filename������ļ�·������һ������ļ�����ͳ��ÿ�������ļ���������
	IndexLSHNoteToFile(param[1].IndexLSHNote,"NLSHIndex.txt","NLSHCounter.txt");

	//��NLSH���������ж����ݼ�������dataSetNote�У�����ÿ��LSH��ʱ��¼��ź�LSH���ƽ����
	dataSetNote=readDataSetFromVector(LSHVector);

	//����MIT��r-NN��Դ�����������
	nPointsData = LSHVector.size();	//NLSH�㼯��С
	dimension = NLSHsize;	//ÿ����ά������NLSH����
	nSampleQueries = 100;	//������������
	thresholdR = 7;	//����
	memoryUpperBound = 600000000;	//�ڴ�����

	if (nPointsData>0)
	{
		dimension = LSHVector[0].size();	//��¼��ʵά��
	}
	LSHVector.clear();	//������м�����������Ѷ���dataSet��
	samePoint.clear();

	//����MIT��r-NN��Դ�����ʼ��LSH���ݽṹ
	//���룺dataSetNote��NLSH�㼯
	//dimension��ÿ��LSH��ά��
	//nPointsData��LSH�㼯��С
	//nSampleQueries��������������
	//thresholdR������
	//memoryUpperBound���ڴ�����
	//�����IndexHuming
	LSHDataStruct(dataSetNote, dimension, nPointsData, nSampleQueries,
		thresholdR, memoryUpperBound, param[1].IndexHuming);

	string IDAndNameFile = "name.txt";
	songread(IDAndNameFile,songIDAndName);	//���ļ��������ID�͸���ӳ���,�ļ�ÿ��Ϊ��ID ����

	/*
	//��param��ʼ���������LSH����
	FAILIF(NULL == (IndexHumingLocal = (PRNearNeighborStructT)MALLOC(sizeof(RNearNeighborStructT))));
	PRNearNeighborStructTCopy(IndexHumingLocal,param[0].IndexHuming);
	FAILIF(NULL == (IndexHumingLocalNote = (PRNearNeighborStructT)MALLOC(sizeof(RNearNeighborStructT))));
	PRNearNeighborStructTCopy(IndexHumingLocalNote,param[1].IndexHuming);
	*/
	return 1;
}

void PRNearNeighborStructTCopy(PRNearNeighborStructT des,PRNearNeighborStructT src)	//RNN���ݽṹ����
{
	des->dimension=src->dimension;
	des->hashedBuckets=src->hashedBuckets;
	des->hfTuplesLength=src->hfTuplesLength;
	des->lshFunctions=src->lshFunctions;
	des->sizeMarkedPoints=src->sizeMarkedPoints;
	FAILIF(NULL == (des->markedPoints = (BooleanT*)MALLOC(des->sizeMarkedPoints * sizeof(BooleanT))));
	for(IntT i = 0; i < des->sizeMarkedPoints; i++)
	{
		des->markedPoints[i] = FALSE;
	}
	FAILIF(NULL == (des->markedPointsIndeces = (Int32T*)MALLOC(des->sizeMarkedPoints * sizeof(Int32T))));
	des->nHFTuples=src->nHFTuples;
	des->nPoints=src->nPoints;
	des->parameterK=src->parameterK;
	des->parameterL=src->parameterL;
	des->parameterR=src->parameterR;
	des->parameterR2=src->parameterR2;
	des->parameterT=src->parameterT;
	des->parameterW=src->parameterW;
	des->points=src->points;
	des->pointsArraySize=src->pointsArraySize;
	FAILIF(NULL == (des->pointULSHVectors = (Uns32T**)MALLOC(des->nHFTuples * sizeof(Uns32T*))));
	for(IntT i = 0; i < des->nHFTuples; i++)
	{
		FAILIF(NULL == (des->pointULSHVectors[i] = (Uns32T*)MALLOC(des->hfTuplesLength * sizeof(Uns32T))));
	}
	FAILIF(NULL == (des->precomputedHashesOfULSHs = (Uns32T**)MALLOC(des->nHFTuples * sizeof(Uns32T*))));
	for(IntT i = 0; i < des->nHFTuples; i++)
	{
		FAILIF(NULL == (des->precomputedHashesOfULSHs[i] = (Uns32T*)MALLOC(N_PRECOMPUTED_HASHES_NEEDED * sizeof(Uns32T))));
	}
	FAILIF(NULL == (des->reducedPoint = (float*)MALLOC(des->dimension * sizeof(float))));
	des->reportingResult=src->reportingResult;
	des->useUfunctions=src->useUfunctions;
}


#include<windows.h>
#include<tlhelp32.h>
#include<iostream>
using namespace std;

int CreateNewProcess(LPCSTR pszExeName,PROCESS_INFORMATION &piProcInfoGPS)
{
	STARTUPINFO siStartupInfo;
	SECURITY_ATTRIBUTES saProcess, saThread;
	ZeroMemory( &siStartupInfo, sizeof(siStartupInfo) );
	siStartupInfo.cb = sizeof(siStartupInfo);
	saProcess.nLength = sizeof(saProcess);
	saProcess.lpSecurityDescriptor = NULL;
	saProcess.bInheritHandle = true;
	saThread.nLength = sizeof(saThread);
	saThread.lpSecurityDescriptor = NULL;
	saThread.bInheritHandle = true;
	return ::CreateProcess( NULL, (LPTSTR)pszExeName, &saProcess,
		&saThread, false,
		CREATE_NO_WINDOW, NULL, NULL,
		&siStartupInfo, &piProcInfoGPS );
}


void main()
{
	ParamInfo param[2];		//������Ϣ
	map <string ,string> songIDAndName;		//����ID�͸���ӳ���
	vector<string> songFive;	//�洢���صļ������

	
	//������Ϊ�������������Ϊ5000newpv355.txt �е�pv�ļ��������������ļ�����./5355P�ļ����У�
	indexRead(param,songIDAndName);

	string fileName("query.txt");	//�ļ�ÿ����һ���߳�wav��·��������ѯ��
	ifstream pitchFile(fileName.c_str());

	string OutName("result.txt");	//��Ž�����ļ�
	ofstream resultFile(OutName.c_str());

	string pitchname;	//wav·��
	queryResultFile.open("query_result.txt");	//��DBN��ѯ����ļ���ÿ��һ����ѯ�����Ϊ��ѡ��źͶ�Ӧ���룬�������С��������

	//��ѯ��Ϣ�ļ�����Ϊ��������ļ�Ϊ׷�Ӵ򿪣���дǰ��ɾ��
	string cmd1 = "del "+query_vector_fname;
	string cmd2 = "del "+query_index_fname;
	string cmd3 = "del "+query_counter_fname;
	system(cmd1.c_str());
	system(cmd2.c_str());
	system(cmd3.c_str());

	double top1Accuracy = 0;	//top1��ȷ��
	double top5Accuracy = 0;	//top5��ȷ��
	double wavCount = 0;	//��ѯwav��

	while(getline(pitchFile,pitchname))	//��ȡÿ�е�wav·��
	{
		char filename[300];		//�߳�wav�ļ�������·����
		strcpy(filename,pitchname.c_str());

		//��ѯ������
		//���룺filename,��ѯ�ļ�·����param,������Ϣ
		//isWriteLSHQueryToFile,�Ƿ�LS���LSH��ѯ���ݼ�����д���ļ�
		//isUseDBNResult���Ƿ���DBN��ѯ���
		//�����songFive����ѯ���
		WavToSongFive(filename,param,songFive,true,false);

		//������
		resultFile<<"query:"<<pitchname<<endl;

		//��ȡ�߳�wav�ļ����ļ���
		int pos = pitchname.find_last_of('\\');
		string queryPureName = pitchname.substr(pos+1,pitchname.length()-pos-5);	//ȡ���һ��'\'��".wav"֮����ļ���

		for (int j=0; j<5 && j<=songFive.size(); j++)
		{
			resultFile<<songFive[j]<<endl;
			if(queryPureName == songFive[j])
			{
				top5Accuracy++;
				if(j==0) top1Accuracy++;
			}
		}
		resultFile<<endl;

		wavCount++;
	}
	top1Accuracy /= wavCount;
	top5Accuracy /= wavCount;
	resultFile << "Top1 ��ȷ�ʣ�" << top1Accuracy*100 << " %" << endl;
	resultFile << "Top5 ��ȷ�ʣ�" << top5Accuracy*100 << " %" << endl;
	
	queryResultFile.close();

	pitchFile.close();	//�رղ�ѯ�ļ���
	resultFile.close();	//�رս���ļ���
}