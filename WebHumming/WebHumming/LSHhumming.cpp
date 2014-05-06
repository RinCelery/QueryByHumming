#include "LSHhumming.h"
//#include <map>
//using namespace std;
//
//extern map<int,vector<int>> samePoint;


char sBufferH[600000];
PPointT * readDataSetFromFileHumming(char *filename,IntT nPoints ,IntT pointsDimension)
{
	FILE *f = fopen(filename, "rt");
	FAILIF(f == NULL);
	PPointT *dataSetPoints=NULL;

	//fscanf(f, "%d %d ", &nPoints, &pointsDimension);
	//FSCANF_DOUBLE(f, &thresholdR);
	//FSCANF_DOUBLE(f, &successProbability);
	//fscanf(f, "\n");
	FAILIF(NULL == (dataSetPoints = (PPointT*)MALLOC(nPoints * sizeof(PPointT))));
	for(IntT i = 0; i < nPoints; i++){
		dataSetPoints[i] = readPointHumming(f,pointsDimension);
		dataSetPoints[i]->index = i;
		printf("ָ���ַ��%d ",dataSetPoints[i]);
	}
	return dataSetPoints;
}


PPointT readPointHumming(FILE *fileHandle,IntT pointsDimension){
	PPointT p;
	float sqrLength = 0;
	FAILIF(NULL == (p = (PPointT)MALLOC(sizeof(PointT))));
	FAILIF(NULL == (p->coordinates = (float*)MALLOC(pointsDimension * sizeof(float))));
	for(IntT d = 0; d < pointsDimension; d++){
		FSCANF_REAL(fileHandle, &(p->coordinates[d]));
		sqrLength += SQR(p->coordinates[d]);
	}
	fscanf(fileHandle, "%[^\n]", sBufferH);
	p->index = -1;
	p->sqrLength = sqrLength;
	for(IntT d = 0; d < pointsDimension; d++){
		printf("%f ",p->coordinates[d]);
	}
	printf("\n");
	return p;
}



//PPointT * readDataSetFromVector(vector<vector<double>> &LSHVector,IntT nPoints ,IntT pointsDimension)
//{
//	char *filename;
//	FILE *f = fopen(filename, "rt");
//	FAILIF(f == NULL);
//	PPointT *dataSetPoints=NULL;
//
//	//fscanf(f, "%d %d ", &nPoints, &pointsDimension);
//	//FSCANF_DOUBLE(f, &thresholdR);
//	//FSCANF_DOUBLE(f, &successProbability);
//	//fscanf(f, "\n");
//	FAILIF(NULL == (dataSetPoints = (PPointT*)MALLOC(nPoints * sizeof(PPointT))));
//	for(IntT i = 0; i < nPoints; i++){
//		dataSetPoints[i] = readPointHumming(f,pointsDimension);
//		dataSetPoints[i]->index = i;
//		printf("ָ���ַ��%d ",dataSetPoints[i]);
//	}
//	return dataSetPoints;
//}


int main123()
{
	char *filenameData="data.txt";
	char *filenameQuery="query.txt";
	float thresholdR=5; 
	float successProbability=0.9;
	Int32T nPointsData=10;//���ݵ���Ŀ
	Int32T nPointsQuery=4;//ѯ�ʵ���Ŀ
	IntT dimension=24; 
	PPointT *dataSet=NULL; 
	IntT nSampleQueries=4; //����ѯ�ʵ���Ŀ
	PPointT *sampleQueries=NULL;
	MemVarT memoryUpperBound=1000;
	PRNearNeighborStructT IndexHuming;
	dataSet=readDataSetFromFileHumming(filenameData,nPointsData ,dimension);
	sampleQueries=readDataSetFromFileHumming(filenameQuery,nPointsQuery ,dimension);
	IndexHuming=initSelfTunedRNearNeighborWithDataSet(thresholdR, successProbability, nPointsData, dimension, dataSet, 
		nSampleQueries, sampleQueries,  memoryUpperBound);//������ֱ�ӳ�ʼ��
	printf("\n��ʼ���ɹ�");



	RNNParametersT HummingParameter=computeOptimalParameters(thresholdR,successProbability,
		nPointsData,dimension,dataSet,nSampleQueries, sampleQueries,memoryUpperBound);//�������Ų���
	PRNearNeighborStructT IndexHumingTowSteps=initLSH_WithDataSet(HummingParameter,nPointsData,dataSet);//�����Ų�������LSH����



	IntT resultSize=3;
	PPointT *result=(PPointT*)MALLOC(resultSize * sizeof(PPointT));
	IntT sizeR=getRNearNeighbors(IndexHuming,sampleQueries[0],result,resultSize);
	IntT sizeR2=getRNearNeighbors(IndexHumingTowSteps,sampleQueries[0],result,resultSize);
	printf("\n��ѯ���");
	return 0;

}



IntT LSHDataQueryToResult(PPointT *dataSet,PPointT *sampleQueries,IntT dimension,Int32T nPointsData,
						 Int32T nPointsQuery,IntT nSampleQueries,float thresholdR,MemVarT memoryUpperBound,
						 IntT *IndexArray)
{
	char *filenameData="data.txt";
	char *filenameQuery="query.txt";
//	float thresholdR=5; 
	float successProbability=0.9;
//	Int32T nPointsData=10;//���ݵ���Ŀ
//	Int32T nPointsQuery=4;//ѯ�ʵ���Ŀ
//	IntT dimension=24; 
	//PPointT *dataSet=NULL; 
//	IntT nSampleQueries=4; //����ѯ�ʵ���Ŀ
//	MemVarT memoryUpperBound=1000;


	PRNearNeighborStructT IndexHuming;
//	dataSet=readDataSetFromFileHumming(filenameData,nPointsData ,dimension);
//	sampleQueries=readDataSetFromFileHumming(filenameQuery,nPointsQuery ,dimension);


	IndexHuming=initSelfTunedRNearNeighborWithDataSet(thresholdR, successProbability, nPointsData, dimension, dataSet, 
		nSampleQueries, dataSet,  memoryUpperBound);//������ֱ�ӳ�ʼ��
	printf("\n��ʼ���ɹ�");



//	RNNParametersT HummingParameter=computeOptimalParameters(thresholdR,successProbability,
//		nPointsData,dimension,dataSet,nSampleQueries, sampleQueries,memoryUpperBound);//�������Ų���
//	PRNearNeighborStructT IndexHumingTowSteps=initLSH_WithDataSet(HummingParameter,nPointsData,dataSet);//�����Ų�������LSH����



	IntT resultSize=3;
	PPointT *result=NULL;
	int IndexArraySize=10000;
	IndexArray=(IntT *)MALLOC(IndexArraySize *sizeof(IntT));
	IntT IndexMaxSize=0;
	IntT IndexNum=0;
	IntT sizeR;
	for (int i=0;i<nPointsQuery;i++)
	{
		result=(PPointT*)MALLOC(resultSize * sizeof(PPointT));
		sizeR=getRNearNeighbors(IndexHuming,sampleQueries[i],result,resultSize);
		for (int j=0;j<sizeR;j++)
		{
			if (IndexNum<IndexArraySize)
			{
				IndexArray[IndexNum]=result[j]->index;
			}
		}
		IndexMaxSize+=sizeR;
		free(result);
	}
	printf("\n��ѯ���");
	return IndexMaxSize;

}

//����MIT��r-NN��Դ�����ʼ��LSH���ݽṹ
//���룺dataSet��LSH�㼯
//dimension��ÿ��LSH��ά��
//nPointsData��LSH�㼯��С
//nSampleQueries��������������
//thresholdR������
//memoryUpperBound���ڴ�����
//�����IndexHuming��RNN����
IntT LSHDataStruct(PPointT *dataSet,IntT dimension,Int32T nPointsData,IntT nSampleQueries,
	float thresholdR,MemVarT memoryUpperBound,PRNearNeighborStructT &IndexHuming)
{
	float successProbability=0.9;

	//����MIT��r-NN��Դ�����ʼ��LSH���ݽṹ
	IndexHuming = initSelfTunedRNearNeighborWithDataSet(thresholdR, successProbability, nPointsData, dimension, dataSet, 
		nSampleQueries, dataSet,  memoryUpperBound);	//������ֱ�ӳ�ʼ��
	printf("\n��ʼ���ɹ�\n");

	return 1;
}

IntT LSHStructToResult(PPointT *sampleQueries,Int32T nPointsQuery,IntT IndexArraySize, 
	   IntT * &IndexArray,PRNearNeighborStructT &IndexHuming,IntT * NumArray ,IntT dimension)
{
	IntT resultSize=3;
	PPointT *result=NULL;
	IntT IndexMaxSize=0;
	IntT IndexNum=0;
	IntT sizeR;
	for (int i=0;i<nPointsQuery;i++)
	{
		result=(PPointT*)MALLOC(resultSize * sizeof(PPointT));
		sizeR=getRNearNeighbors(IndexHuming,sampleQueries[i],result,resultSize);
		for (int j=0;j<sizeR;j++)
		{
			if (IndexNum<IndexArraySize)
			{
				IndexArray[IndexNum]=result[j]->index;
				IndexNum++;
			}
		}
		NumArray[i]=sizeR;
		IndexMaxSize+=sizeR;
		free(result);
	}
	return IndexMaxSize;
}



//�õ�NLSH���������ÿ��LSH�����RNN��ѯ������RetainNum����ѡ
//���룺sampleQueries��NLSH�㼯��nPointsQuery��NLSH����
//IndexArraySize�������Һ�ѡ��
//IndexHuming��NLSH���RNN������RetainNum��NLSHÿ����������ĵ���
//dimension��NLSH��ά����LSHFilterNum��NLSH�˲������ĵ���
//�����
//IndexArray��NumArray��ÿ���㷵�ص���Ŀ
//IndexFilterArray��sizeFilter��IndexArrayDis
IntT LSHStructToResultOnePointRetainSeveral(PPointT *sampleQueries,Int32T nPointsQuery,IntT IndexArraySize, 
		IntT * &IndexArray,PRNearNeighborStructT &IndexHuming,IntT * NumArray,
		IntT RetainNum ,IntT dimension,IntT LSHFilterNum,
		IntT * &IndexFilterArray,IntT &sizeFilter,double * &IndexArrayDis)
{
	IntT IndexMaxSize = 0;
	IntT IndexNum = 0;
	IntT IndexLSHFilterNum = 0;
	IntT sizeR;

	IntT sizeRetain = LSHFilterNum > RetainNum ? LSHFilterNum: RetainNum;	//ʵ��ÿ���㱣��������RetainNum��5����LSHFilterNum��200�������ֵ


    IntT * RetainIndex = (IntT *)MALLOC(sizeRetain * sizeof(IntT));
	double * RetainDis = (double *)MALLOC(sizeRetain * sizeof(double));

	//��ÿ��LSH�����RNN��ѯ
	for (int i=0;i<nPointsQuery;i++)
	{
		IntT resultSize = 10;	//RNN��ѯ������ٵĿռ�
		PPointT *result = (PPointT*)MALLOC(resultSize * sizeof(PPointT));	//RNN��ѯ���

		//����MIT��RNN��Դ����õ�RNN���
		//���룺IndexHuming��RNN������sampleQueries[i]����ѯ��LSH��
		//�����result����ѯ�����resultSize����ѯ������ٵĿռ䣨���������resultSizeʱ��resultSize*=2�����ռ䲻�㿪�������ռ䣩
		//����ֵ��sizeR����ѯ�����
		sizeR = getRNearNeighbors(IndexHuming,sampleQueries[i],result,resultSize);

		if (sizeR<=RetainNum)	//��ѯ���������Ҫ�����ĵ�����
		{
			if (sizeR>0)	//�в�ѯ���

			{//��ø���ѡ�����ѯ��ľ��뼰��Ӧ��ŵ�RetainDis��RetainIndex��

				AllResultToRetainMostNearResult(RetainIndex,sampleQueries[i], sizeR ,
					result,dimension,sizeR,RetainDis);
			}
			for (int j=0;j<sizeR;j++)
			{
				if (IndexNum<IndexArraySize)
				{
					IndexArray[IndexNum]=result[j]->index;//�õ������ѡ��Ӧ������
					IndexFilterArray[IndexLSHFilterNum]=result[j]->index;//�õ������ѡ��Ӧ������
					IndexArrayDis[IndexNum]=RetainDis[j];//�õ��������ֵ
					IndexNum++;
					IndexLSHFilterNum++;
				}
			}
			NumArray[i]=sizeR;//�õ�ÿ�����ѯ������ص���Ŀ
			IndexMaxSize+=sizeR;//�õ����е�Ľ����Ŀ
			free(result);
		}
		else//��ѯ���������Ҫ�����Ķ�
		{
			IntT sizeResort= LSHFilterNum < sizeR ? LSHFilterNum: sizeR;//ʵ��ÿ���㱣��������sizeR��LSHFilterNum����Сֵ
			AllResultToRetainMostNearResult(RetainIndex,sampleQueries[i], sizeResort ,
				result,dimension,sizeR,RetainDis);//��ø���ѡ�����ѯ��ľ��루��С��������󣩼���Ӧ��ŵ�RetainDis��RetainIndex��
			for (int j=0;j<RetainNum;j++)
			{//�õ�ǰretainNum�����뼰��Ž��
				if (IndexNum<IndexArraySize)
				{
					IndexArray[IndexNum]=RetainIndex[j];
					IndexArrayDis[IndexNum]=RetainDis[j];
					IndexNum++;
				}
			}
			if (RetainDis[RetainNum-1]==RetainDis[RetainNum])//�����һ����ѡ������һ����ѡ�����ͬ�����һ���Ա���
			{
				IndexArray[IndexNum]=RetainIndex[RetainNum];
				IndexArrayDis[IndexNum]=RetainDis[RetainNum];
				IndexNum++;
				NumArray[i]=RetainNum+1;//��Ҫ���صĽ������1
				IndexMaxSize+=(RetainNum+1);//�õ����е�Ľ����Ŀ��1
				if (sizeR>RetainNum+1 && RetainDis[RetainNum]==RetainDis[RetainNum+1])//�����������������һ����ѡ������ͬ���򶼱���
				{
					IndexArray[IndexNum]=RetainIndex[RetainNum+1];
					IndexArrayDis[IndexNum]=RetainDis[RetainNum+1];
					IndexNum++;
					NumArray[i]=RetainNum+2;//��Ҫ���صĽ������2
					IndexMaxSize+=1;//�õ����е�Ľ����Ŀ�ټ�1
				}
			}
			else//�����һ����ѡ������һ����ѡ�����ͬ���������е�
			{
				NumArray[i]=RetainNum;
				IndexMaxSize+=RetainNum;
			}
			
			if (sizeR<=LSHFilterNum)//����ѯ���С���˲������ĵ��������������Ϊ��ѯ������
			{
				for (int j=0;j<sizeR;j++)
				{
					if (IndexNum<IndexArraySize)
					{
						IndexFilterArray[IndexLSHFilterNum]=result[j]->index;
						IndexLSHFilterNum++;
					}
				}
			}
			else//����ѯ��������˲������ĵ��������������Ϊ������Ľ�����
			{
				for (int j=0;j<LSHFilterNum;j++)
				{
					if (IndexLSHFilterNum<IndexArraySize)
					{
						IndexFilterArray[IndexLSHFilterNum]=RetainIndex[j];
						IndexLSHFilterNum++;
					}
				}

			}
			free(result);
		}
	}
	free(RetainIndex);
	free(RetainDis);
	sizeFilter=IndexLSHFilterNum;//���в�ѯ���ĺ�ѡ�����Ŀ

	return IndexMaxSize;//�������е�Ĳ�ѯ���������Ŀ
}

//��ø���ѡ��ľ�������󼰶�Ӧ��ŵ�RetainIndex��RetainDis��
IntT AllResultToRetainMostNearResult(IntT * RetainIndex,PPointT sampleQueries,IntT RetainNum ,
									 PPointT *result,IntT dimension,IntT sizeResult,double * RetainDis)//���������RetainNum����
{
	float * ResultDis=(float * )MALLOC(sizeResult * sizeof(float));
	IntT * ResultLable=(IntT * )MALLOC(sizeResult * sizeof(IntT));
	for (int i=0;i<sizeResult;i++)
	{
		ResultDis[i]=dist(sampleQueries,result[i],dimension);//���ز�ѯ�������ѡ���ľ���
		//ResultDis[i]=result[i]->sqrLength;
		ResultLable[i]=result[i]->index;//����ѡ��Ӧ���
	}
	sort(ResultDis, ResultLable,sizeResult,RetainNum);//������ѡ������С��������
	for (int i=0;i<RetainNum;i++)
	{
		RetainIndex[i]=ResultLable[i];
		RetainDis[i]=ResultDis[i];
	}
	free(ResultDis);
	free(ResultLable);

	return 0;
}

float dist(PPointT p1, PPointT p2, IntT dimension){ //����������ľ���
	float result = 0;

	for (int i = 0; i < dimension; i++){
		float temp = p1->coordinates[i] - p2->coordinates[i];
#ifdef USE_L1_DISTANCE
		result += ABS(temp);
#else
		result += SQR(temp);
#endif
	}
#ifdef USE_L1_DISTANCE
	return result;
#else
	return SQRT(result);
#endif
}


void sort(float *arr,IntT * ResultLable,int n ,int SortNum)//��С��������
{
	float temp;
	IntT tempLable;
	int i,j,k;
	for (i = 0; i < n - 1 && i<SortNum; i++)
	{
		k = i;
		for (j = i + 1; j < n; j++)
		{
			if (arr[k]>arr[j] )
			{
				k = j;
			}
		}

		if(k != i)
		{
			temp = arr[i];
			arr[i]= arr[k];
			arr[k] = temp;
			tempLable = ResultLable[i];
			ResultLable[i]= ResultLable[k];
			ResultLable[k] = tempLable;
		}
	}
}
