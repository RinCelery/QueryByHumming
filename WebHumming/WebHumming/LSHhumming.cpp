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
		printf("指针地址：%d ",dataSetPoints[i]);
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
//		printf("指针地址：%d ",dataSetPoints[i]);
//	}
//	return dataSetPoints;
//}


int main123()
{
	char *filenameData="data.txt";
	char *filenameQuery="query.txt";
	float thresholdR=5; 
	float successProbability=0.9;
	Int32T nPointsData=10;//数据点数目
	Int32T nPointsQuery=4;//询问点数目
	IntT dimension=24; 
	PPointT *dataSet=NULL; 
	IntT nSampleQueries=4; //测试询问点数目
	PPointT *sampleQueries=NULL;
	MemVarT memoryUpperBound=1000;
	PRNearNeighborStructT IndexHuming;
	dataSet=readDataSetFromFileHumming(filenameData,nPointsData ,dimension);
	sampleQueries=readDataSetFromFileHumming(filenameQuery,nPointsQuery ,dimension);
	IndexHuming=initSelfTunedRNearNeighborWithDataSet(thresholdR, successProbability, nPointsData, dimension, dataSet, 
		nSampleQueries, sampleQueries,  memoryUpperBound);//用数据直接初始化
	printf("\n初始化成功");



	RNNParametersT HummingParameter=computeOptimalParameters(thresholdR,successProbability,
		nPointsData,dimension,dataSet,nSampleQueries, sampleQueries,memoryUpperBound);//构造最优参数
	PRNearNeighborStructT IndexHumingTowSteps=initLSH_WithDataSet(HummingParameter,nPointsData,dataSet);//用最优参数构造LSH索引



	IntT resultSize=3;
	PPointT *result=(PPointT*)MALLOC(resultSize * sizeof(PPointT));
	IntT sizeR=getRNearNeighbors(IndexHuming,sampleQueries[0],result,resultSize);
	IntT sizeR2=getRNearNeighbors(IndexHumingTowSteps,sampleQueries[0],result,resultSize);
	printf("\n查询完毕");
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
//	Int32T nPointsData=10;//数据点数目
//	Int32T nPointsQuery=4;//询问点数目
//	IntT dimension=24; 
	//PPointT *dataSet=NULL; 
//	IntT nSampleQueries=4; //测试询问点数目
//	MemVarT memoryUpperBound=1000;


	PRNearNeighborStructT IndexHuming;
//	dataSet=readDataSetFromFileHumming(filenameData,nPointsData ,dimension);
//	sampleQueries=readDataSetFromFileHumming(filenameQuery,nPointsQuery ,dimension);


	IndexHuming=initSelfTunedRNearNeighborWithDataSet(thresholdR, successProbability, nPointsData, dimension, dataSet, 
		nSampleQueries, dataSet,  memoryUpperBound);//用数据直接初始化
	printf("\n初始化成功");



//	RNNParametersT HummingParameter=computeOptimalParameters(thresholdR,successProbability,
//		nPointsData,dimension,dataSet,nSampleQueries, sampleQueries,memoryUpperBound);//构造最优参数
//	PRNearNeighborStructT IndexHumingTowSteps=initLSH_WithDataSet(HummingParameter,nPointsData,dataSet);//用最优参数构造LSH索引



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
	printf("\n查询完毕");
	return IndexMaxSize;

}

//调用MIT的r-NN开源代码初始化LSH数据结构
//输入：dataSet：LSH点集
//dimension：每个LSH点维数
//nPointsData：LSH点集大小
//nSampleQueries：测试样本点数
//thresholdR：门限
//memoryUpperBound：内存上限
//输出：IndexHuming
IntT LSHDataStruct(PPointT *dataSet,IntT dimension,Int32T nPointsData,IntT nSampleQueries,
	float thresholdR,MemVarT memoryUpperBound,PRNearNeighborStructT &IndexHuming)
{
	float successProbability=0.9;

	//调用MIT的r-NN开源代码初始化LSH数据结构
	IndexHuming = initSelfTunedRNearNeighborWithDataSet(thresholdR, successProbability, nPointsData, dimension, dataSet, 
		nSampleQueries, dataSet,  memoryUpperBound);	//用数据直接初始化
	printf("\n初始化成功\n");

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

IntT LSHStructToResultOnePointRetainSeveral(PPointT *sampleQueries,Int32T nPointsQuery,IntT IndexArraySize, 
					   IntT * &IndexArray,PRNearNeighborStructT &IndexHuming,IntT * NumArray,
					   IntT RetainNum ,IntT dimension,IntT LSHFilterNum,
					   IntT * &IndexFilterArray,IntT &sizeFilter,double * &IndexArrayDis)
{
	IntT resultSize=10;
	PPointT *result=NULL;
	IntT IndexMaxSize=0;
	IntT IndexNum=0;
	IntT IndexLSHFilterNum=0;
	IntT sizeR;
	IntT sizeRetain= LSHFilterNum > RetainNum ? LSHFilterNum: RetainNum;
    IntT * RetainIndex=(IntT * )MALLOC(sizeRetain * sizeof(IntT));
	double * RetainDis=(double * )MALLOC(sizeRetain * sizeof(double));
	for (int i=0;i<nPointsQuery;i++)
	{
		result=(PPointT*)MALLOC(resultSize * sizeof(PPointT));
		sizeR=getRNearNeighbors(IndexHuming,sampleQueries[i],result,resultSize);
		if (sizeR<=RetainNum)
		{
			if (sizeR>0)
			{
				AllResultToRetainMostNearResult(RetainIndex,sampleQueries[i], sizeR ,
					result,dimension,sizeR,RetainDis);
			}
			for (int j=0;j<sizeR;j++)
			{
				if (IndexNum<IndexArraySize)
				{
					IndexArray[IndexNum]=result[j]->index;
					IndexFilterArray[IndexLSHFilterNum]=result[j]->index;
					IndexArrayDis[IndexNum]=RetainDis[j];
					IndexNum++;
					IndexLSHFilterNum++;
				}
			}
			NumArray[i]=sizeR;
			IndexMaxSize+=sizeR;
			free(result);
		}
		else
		{
			IntT sizeResort= LSHFilterNum < sizeR ? LSHFilterNum: sizeR;
			AllResultToRetainMostNearResult(RetainIndex,sampleQueries[i], sizeResort ,
				result,dimension,sizeR,RetainDis);
			for (int j=0;j<RetainNum;j++)
			{
				if (IndexNum<IndexArraySize)
				{
					IndexArray[IndexNum]=RetainIndex[j];
					IndexArrayDis[IndexNum]=RetainDis[j];
					IndexNum++;
				}
			}
			if (RetainDis[RetainNum-1]==RetainDis[RetainNum])
			{
				IndexArray[IndexNum]=RetainIndex[RetainNum];
				IndexArrayDis[IndexNum]=RetainDis[RetainNum];
				IndexNum++;
				NumArray[i]=RetainNum+1;
				IndexMaxSize+=(RetainNum+1);
				if (sizeR>RetainNum+1 && RetainDis[RetainNum]==RetainDis[RetainNum+1])
				{
					IndexArray[IndexNum]=RetainIndex[RetainNum+1];
					IndexArrayDis[IndexNum]=RetainDis[RetainNum+1];
					IndexNum++;
					NumArray[i]=RetainNum+2;
					IndexMaxSize+=1;
				}
			}
			else
			{
				NumArray[i]=RetainNum;
				IndexMaxSize+=RetainNum;
			}
			
			if (sizeR<=LSHFilterNum)
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
			else
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
	sizeFilter=IndexLSHFilterNum;

	return IndexMaxSize;
}


IntT AllResultToRetainMostNearResult(IntT * RetainIndex,PPointT sampleQueries,IntT RetainNum ,
									 PPointT *result,IntT dimension,IntT sizeResult,double * RetainDis)//返回最近的RetainNum个点
{
	float * ResultDis=(float * )MALLOC(sizeResult * sizeof(float));
	IntT * ResultLable=(IntT * )MALLOC(sizeResult * sizeof(IntT));
	for (int i=0;i<sizeResult;i++)
	{
		ResultDis[i]=dist(sampleQueries,result[i],dimension);
		//ResultDis[i]=result[i]->sqrLength;
		ResultLable[i]=result[i]->index;
	}
	sort(ResultDis, ResultLable,sizeResult,RetainNum);
	for (int i=0;i<RetainNum;i++)
	{
		RetainIndex[i]=ResultLable[i];
		RetainDis[i]=ResultDis[i];
	}
	free(ResultDis);
	free(ResultLable);

	return 0;
}

float dist(PPointT p1, PPointT p2, IntT dimension){ //返回两个点的距离
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


void sort(float *arr,IntT * ResultLable,int n ,int SortNum)//排序
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
