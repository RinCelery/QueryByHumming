#include "index.h"
extern map<int,vector<int>> samePoint;
extern map<string,int> ListSong;


int readindex(string dataIndex,map<string , vector<vector<double>>> &indexSongName)
{
	ifstream indexFile(dataIndex.c_str());
	vector <double> pitchNum;
	vector<vector<double>> database;
	ifstream songPitch;
	string songName,songPitchNum;
	string::size_type pos=0;
	while (indexFile>>songName)
	{
		songPitch.open(songName.c_str());
		while (getline(songPitch,songPitchNum))
		{
			if (songPitchNum.find("value")!=string::npos)
			{
				pitchNum.clear();
				pos=songPitchNum.find("value");
				songPitchNum.erase(0,pos+8);
				pitchNum.push_back(atof(songPitchNum.c_str()));
				database.push_back(pitchNum);

			}
        }
		indexSongName.insert(make_pair(songName,database));
		database.clear();
		songPitch.close();
		songPitch.clear();
	}
	indexFile.close();
	return 0;
}

int IndexSignToQueryAndDataVectorHummingMatchLeastALLNote(vector<pair<short, short>>& posPair,vector<IntT> &IndexCandidatesStretch,vector<IntT> &CandidatesNumStretch,float stretchFactor,
													  map<unsigned long , pair<string,pair<short,short>>> &IndexLSH,int stepFactor,vector <float> queryPitch,
													  vector<float>  &queryX, vector< vector<float> > &dataY,
													  map<string , vector<float>> &indexSongName,vector <string> &SongNameMapToDataY,
													  int &CandidatesSizeInDWT,int MatchBeginPos,map <string , short > &SongMapPosition,
													  map <string , vector<pair<short, double>> >  &SongMapPositionAll,int offsetbegin, int offsetLength)//�����������ӷ��غ�ѡ�����Լ���Ӧ�ĸ�������
{  
	//ƥ��������Ŀ������ȷƥ�����Ŀ����
	int posBegin=100;//С��1/posBegin�ı���
	queryX.clear();
	StringToString(queryPitch,queryX,1);
	int sizePoints=CandidatesNumStretch.size();
	int sizeCandidates=0;
	int QueryLSHPos=0;
	int QueryLSHLength=0;
	float DataYLength=0;
	float ratioLength=0;
	int accuSizeQuery=0;
	int DifferPos=0;
	int sizeQuery=queryX.size();
	int queryLogicPos=0;
	pair<string, pair<short ,short>> SongNamePos;
	map<string , vector<float>> :: iterator SongTone;
	vector<float>  DataYOnePoint;
	vector <string> :: iterator SongIter;
	vector< vector<float> >  :: iterator SongPitchIter;
	map <string , vector<pair<short, double>> > :: iterator songPosIter;
	vector<pair<short, double>>  :: iterator PaiCIter;

	for (int i=0;i<sizePoints;i++)
	{
		int sizeCandidatesCurrent=sizeCandidates;
		int CurrentNum=CandidatesNumStretch[i];
		for (int j=sizeCandidatesCurrent;j<sizeCandidatesCurrent+CurrentNum;j++)
		{
			sizeCandidates++;

			if (IndexLSH.count(IndexCandidatesStretch[j]))
			{
				QueryLSHPos=posPair[i].first;
				QueryLSHLength=posPair[i].second;
				SongNamePos=IndexLSH.find(IndexCandidatesStretch[j])->second;
				DataYLength=SongNamePos.second.second;
				ratioLength=DataYLength/QueryLSHLength;
				queryLogicPos=DataYLength*QueryLSHPos/QueryLSHLength;
				DifferPos=SongNamePos.second.first-queryLogicPos-offsetbegin;
				accuSizeQuery=sizeQuery/ratioLength+offsetLength;
				if (indexSongName.count(SongNamePos.first) && ratioLength>0.5 && ratioLength<2)
				{
					DataYOnePoint.clear();
					SongTone=indexSongName.find(SongNamePos.first);
					int sizeSong=SongTone->second.size();
					if (DifferPos>=0  &&DifferPos+accuSizeQuery*3.0/4.0<sizeSong)//��֤���Ȳ�����ĩβ����,����queryһ��
					{
						for (int k=DifferPos;k<DifferPos+accuSizeQuery && k<sizeSong;k++)
						{
							DataYOnePoint.push_back(SongTone->second[k]);//���˵㸴�ƹ�ȥ
						}
						if (SongMapPositionAll.count(SongNamePos.first))
						{
							int booW=1;
							songPosIter=SongMapPositionAll.find(SongNamePos.first);
							for (int i=0;i<songPosIter->second.size();i++)
							{
								if (abs(songPosIter->second[i].first-DifferPos)<sizeQuery/posBegin && 
									abs(songPosIter->second[i].second-accuSizeQuery)<6)
								{
									booW=0;
								}
							}
							if (booW==1)
							{
								dataY.push_back(DataYOnePoint);
								SongNameMapToDataY.push_back(SongNamePos.first);
								SongMapPositionAll[SongNamePos.first].push_back(make_pair(DifferPos,accuSizeQuery));
								CandidatesSizeInDWT++;
							}
						}
						else
						{
							vector<pair<short, double>> temp;
							temp.push_back(make_pair(DifferPos,accuSizeQuery));
							SongMapPositionAll.insert(make_pair(SongNamePos.first,accuSizeQuery));
							dataY.push_back(DataYOnePoint);
							SongNameMapToDataY.push_back(SongNamePos.first);
							CandidatesSizeInDWT++;
						}
					}
					else if (DifferPos<0 && DifferPos >-(sizeQuery/3) &&  DifferPos+sizeQuery<sizeSong)
					{
						DifferPos=0;
						for (int k=0;k<DifferPos+accuSizeQuery && k<sizeSong;k++)
						{
							DataYOnePoint.push_back(SongTone->second[k]);//���˵㸴�ƹ�ȥ
						}
						if (SongMapPositionAll.count(SongNamePos.first))
						{
							int booW=1;
							songPosIter=SongMapPositionAll.find(SongNamePos.first);
							for (int i=0;i<songPosIter->second.size();i++)
							{
								if (abs(songPosIter->second[i].first-DifferPos)<sizeQuery/posBegin && 
									abs(songPosIter->second[i].second-accuSizeQuery)<4)
								{
									booW=0;
								}
							}
							if (booW==1)//������б��е�λ�ÿ�ǰ
							{
								dataY.push_back(DataYOnePoint);
								SongNameMapToDataY.push_back(SongNamePos.first);
								SongMapPositionAll[SongNamePos.first].push_back(make_pair(DifferPos,accuSizeQuery));
								CandidatesSizeInDWT++;
							}
						}
						else
						{
							vector<pair<short, double>> temp;
							temp.push_back(make_pair(DifferPos,accuSizeQuery));
							SongMapPositionAll.insert(make_pair(SongNamePos.first,temp));
							dataY.push_back(DataYOnePoint);
							SongNameMapToDataY.push_back(SongNamePos.first);
							CandidatesSizeInDWT++;
						}
					}
					else
						CandidatesNumStretch[i]--;//�˾����д洢ʵ�ʾ�ȷƥ�����Ŀ
				}
				else
					CandidatesNumStretch[i]--;
			}
			else
				CandidatesNumStretch[i]--;
		}
	}
	return 0;
}


int IndexSignToQueryAndDataVectorHummingMatchLeastALL(vector<IntT> &IndexCandidatesStretch,vector<IntT> &CandidatesNumStretch,float stretchFactor,
													  map<unsigned long , pair<string,short>> &IndexLSH,int stepFactor,vector <float> queryPitch,
													  vector<float>  &queryX, vector< vector<float> > &dataY,
													  map<string , vector<float>> &indexSongName,vector <string> &SongNameMapToDataY,
													  int &CandidatesSizeInDWT,int MatchBeginPos,map <string , short > &SongMapPosition,
													  map <string , vector<pair<short, double>> >  &SongMapPositionAll,int offsetbegin, int offsetLength)//�����������ӷ��غ�ѡ�����Լ���Ӧ�ĸ�������
{  
	//ƥ��������Ŀ������ȷƥ�����Ŀ����
	int posBegin=100;//С��1/posBegin�ı���
	queryX.clear();
	StringToString(queryPitch,queryX,1);
	int sizePoints=CandidatesNumStretch.size();
	int sizeCandidates=0;
	int QueryLSHPos=0;
	int DifferPos=0;
	int sizeQuery=queryX.size();

	sizeQuery=(sizeQuery*stretchFactor+offsetLength);

	pair<string, short> SongNamePos;
	map<string , vector<float>> :: iterator SongTone;
	vector<float>  DataYOnePoint;
	vector <string> :: iterator SongIter;
	vector< vector<float> >  :: iterator SongPitchIter;
	map <string , vector<pair<short, double>> > :: iterator songPosIter;
	vector<pair<short, double>>  :: iterator PaiCIter;

	for (int i=0;i<sizePoints;i++)
	{
		int sizeCandidatesCurrent=sizeCandidates;
		int CurrentNum=CandidatesNumStretch[i];
		for (int j=sizeCandidatesCurrent;j<sizeCandidatesCurrent+CurrentNum;j++)
		{
			sizeCandidates++;

			if (IndexLSH.count(IndexCandidatesStretch[j]))
			{
				QueryLSHPos=i*stepFactor;
				SongNamePos=IndexLSH.find(IndexCandidatesStretch[j])->second;
				DifferPos=SongNamePos.second-QueryLSHPos-offsetbegin;
				if (indexSongName.count(SongNamePos.first))
				{
					DataYOnePoint.clear();
					SongTone=indexSongName.find(SongNamePos.first);
					int sizeSong=SongTone->second.size();
					if (DifferPos>=0  &&DifferPos+sizeQuery*3.0/4.0<sizeSong)//��֤���Ȳ�����ĩβ����,����queryһ��
					{
						for (int k=DifferPos;k<DifferPos+sizeQuery && k<sizeSong;k++)
						{
							DataYOnePoint.push_back(SongTone->second[k]);//���˵㸴�ƹ�ȥ

						}
						if (SongMapPositionAll.count(SongNamePos.first))
						{
							int booW=1;
							songPosIter=SongMapPositionAll.find(SongNamePos.first);
							for (int i=0;i<songPosIter->second.size();i++)
							{
								if (abs(songPosIter->second[i].first-DifferPos)<sizeQuery/posBegin && 
									abs(songPosIter->second[i].second-stretchFactor)<0.1)
								{
									booW=0;
								}
							}
							if (booW==1)
							{
								dataY.push_back(DataYOnePoint);
								SongNameMapToDataY.push_back(SongNamePos.first);
								SongMapPositionAll[SongNamePos.first].push_back(make_pair(DifferPos,stretchFactor));
								CandidatesSizeInDWT++;

							}
						}
						else
						{
							vector<pair<short, double>> temp;
							temp.push_back(make_pair(DifferPos,stretchFactor));
							SongMapPositionAll.insert(make_pair(SongNamePos.first,temp));
							dataY.push_back(DataYOnePoint);
							SongNameMapToDataY.push_back(SongNamePos.first);
							CandidatesSizeInDWT++;

						}
					}
					else if (DifferPos<0 && DifferPos >-(sizeQuery/3) &&  DifferPos+sizeQuery<sizeSong)
					{
						DifferPos=0;
						for (int k=0;k<DifferPos+sizeQuery && k<sizeSong;k++)
						{
							DataYOnePoint.push_back(SongTone->second[k]);//���˵㸴�ƹ�ȥ

						}
						if (SongMapPositionAll.count(SongNamePos.first))
						{
							int booW=1;
							songPosIter=SongMapPositionAll.find(SongNamePos.first);
							for (int i=0;i<songPosIter->second.size();i++)
							{
								if (abs(songPosIter->second[i].first-DifferPos)<sizeQuery/posBegin && 
									abs(songPosIter->second[i].second-stretchFactor)<0.05)
								{
									booW=0;
								}
							}
							if (booW==1)//������б��е�λ�ÿ�ǰ
							{
								dataY.push_back(DataYOnePoint);
								SongNameMapToDataY.push_back(SongNamePos.first);
								SongMapPositionAll[SongNamePos.first].push_back(make_pair(DifferPos,stretchFactor));
								CandidatesSizeInDWT++;

							}
						}
						else
						{
							vector<pair<short, double>> temp;
							temp.push_back(make_pair(DifferPos,stretchFactor));
							SongMapPositionAll.insert(make_pair(SongNamePos.first,temp));
							dataY.push_back(DataYOnePoint);
							SongNameMapToDataY.push_back(SongNamePos.first);
							CandidatesSizeInDWT++;
						}
					}
					else
						CandidatesNumStretch[i]--;//�˾����д洢ʵ�ʾ�ȷƥ�����Ŀ
				}
				else
					CandidatesNumStretch[i]--;
			}
			else
				CandidatesNumStretch[i]--;
		}
	}
	return 0;

}


//����pv�ļ�������ά��(����������ʱ��)����ת��Ϊһά��������
//���룺dataIndex: ���������б��ļ�
//�����indexSongName: �ļ�·����5355P\\XXX.pv���Ͷ�Ӧpv�ļ���һά��������
int readIndexPitch8MinutesNewPv(string dataIndex, map<string , vector<float>> &indexSongName)
{
	int frames_per_sec = 25;	//ÿ���֡��
	int max_totalMidi = 50000;	//�������pv�ļ���
	double max_time_midi = 600;	//ÿ��pv������ʱ��(s)
	int max_mean_thd = frames_per_sec*max_time_midi;	//����ֵ����ʱ���ֵ�����֡��

	ifstream indexFile(dataIndex.c_str());	//����pv�б��ļ���

	string songName;	//pv�ļ���
	int totalMidi=0;	//pv�ļ�����

	while (indexFile>>songName)	//����һ��pv�ļ���
	{
		totalMidi++;	//����

		if (totalMidi == max_totalMidi)	//�����������pv�ļ���
		{
			break;
		}

		songName = "5355P\\" + songName;	//�����ļ�(*.pv)·��
		
		locale::global(locale(""));		//�趨ȫ�� locale Ϊ�������õ�locale,���std::ofstream.open()����·���ļ�������
		
		ifstream songPitch;
		songPitch.open(songName.c_str());	//�򿪵�ǰpv�ļ���

		locale::global(locale("C"));	//���½�ȫ��locale�趨ΪC locale

		double time_midi=0;	//��¼ÿ��pv��Ч����ʱ��
		string songPitchNum,songPitchBegin,songPitchDuration;	//���գ���������ʼʱ�䣬����ʱ�䣩���ַ���
		float pitchNum=0;	//��ǰ����
		float pitchPre=0;	//ǰһ������
		float duration=0;	//��������ʱ��
		int pitchAmount=0;	//��������֡��

		vector<float> database;	//�洢��pv�ļ���һά��������

		//��ȡһ��pv�ļ���������Ч������ʱ��������360s��6min
		//pv�ļ���ʽ������ ��ʼʱ��(s) ����ʱ��(s)
		while (time_midi < max_time_midi && 
			songPitch>>songPitchNum && songPitch>>songPitchBegin && songPitch>>songPitchDuration )
		{
			pitchNum = atof(songPitchNum.c_str());	//����
			duration = atof(songPitchDuration.c_str());	//����ʱ��
			pitchAmount = duration * frames_per_sec;	//��������������֡����25֡/s
			//cout<<pitchNum<<" "<<duration<<" "<<pitchAmount<<endl;

			if(duration<5 && pitchAmount>=1)	//��Ч��������С��5s������1֡����40ms
			{
				
				if (pitchNum != pitchPre && pitchAmount<3)	//��ǰһ��������ͬ�ҳ���С��3֡��������
				{
					;
				}
				else	//��ǰһ��������ͬ��������������ȴ���3֡
				{
					pitchPre = pitchNum;
					time_midi += duration;	//��¼��ʱ��
					for (int i=0;i<pitchAmount;i++)	//����ά����ת��Ϊһά�������У�����database
						database.push_back(pitchNum);
				}
			}
		}
		
		if (totalMidi%10==0)	//ÿ10�׸����һ���������е���
		{
			cout<<"pv�ļ�(��ţ��ļ���)��"<<totalMidi<<","<<songName<<endl;
			cout<<"���ļ�һά��������֡��:"<<database.size()<<endl;
		}

		NoZero(database);	//ȥ�������е���ֵ
		smooth(database);	//�������ֵ��ƽ��������Ϊ5������Ϊ1
		MinusMeanWithThd(database,max_mean_thd);		//����ֵ������������ǰmax_mean_thd֡�ľ�ֵ

		indexSongName.insert(make_pair(songName,database));	//��pv�ļ�����һά�������в����������

		database.clear();	//��յ�ǰpv�ļ���һά��������
		songPitch.close();	//�رյ�ǰpv�ļ�
		songPitch.clear();	//����ļ���
	}
	indexFile.close();	//�ر�pv�б��ļ�

	return 0;
}

int charToVector(char * wavename,vector <float> &queryPitch)
{
	int  n=win;
	unsigned long DataLength=LONGPOINT;	//��Ƶ�еĲ���������
	int *pitchnum;
	double pitchNum=0;
	float energy=0;
	int BeginFrame[]={0};
	DataLength=pitchana(wavename,&pitchnum,BeginFrame);
	int leng_i=(DataLength-FLENGTH)/FSHIFTW+1;
	for (int i=0;i<leng_i;i++)
	{
		pitchNum=*(pitchnum+i);
		queryPitch.push_back(pitchNum);
	}
	free(pitchnum);

	return *BeginFrame;
}

//�����ֵ
float MiddleFive(float a, float b, float c,float d,float e)
{
	vector <float> num;
	num.push_back(a);
	num.push_back(b);
	num.push_back(c);
	num.push_back(d);
	num.push_back(e);
	vector <float>::iterator ite_begin=num.begin();
	vector <float>::iterator ite_end=num.end();
	stable_sort(ite_begin,ite_end);
	ite_begin=num.begin();

	return *(ite_begin+2);
}

//�������ֵ��ƽ��������Ϊ5������Ϊ1
void smooth(vector <float> &queryPitch)
{
	vector <float> queryPitchDuplicate(queryPitch);
	int m=queryPitch.size();
	int i;
	for (i=2;i<m-2;i++)
	{
		queryPitch[i] = MiddleFive(queryPitchDuplicate[i-2],queryPitchDuplicate[i-1],
			queryPitchDuplicate[i],queryPitchDuplicate[i+1],queryPitchDuplicate[i+2]);
	}
}

void VectorSmoothToHalf(vector <float> &queryPitch)
{
	int m=queryPitch.size();
	int n=0;
	int i,j;
	vector <float> queryPitchHalf;
	for (i=0;i<m-1;i+=2)
	{
		queryPitchHalf.push_back(queryPitch[i]);
	}
	queryPitch.clear();
	n=queryPitchHalf.size();
	for (i=0;i<n;i++)
	{
		queryPitch.push_back(queryPitchHalf[i]);
	}

}

//��һά�������г�ȡ����������LSH��
//���룺indexSongName��pv�ļ����Ͷ�Ӧ��һά��������
//noteMaxFrame��һ�����ߵ��֡�����������з֣�
//NLSHsize��LSH������һ��LSH��Ĵ�С
//maxFrame��һά�����������֡��
//�����LSHVector��LSH�㣬ÿ����Ϊһ��10ά����������
//IndexLSH,	LSH��������¼��LSHVector����ţ�ÿ��LSH�����ʼλ�ã�����֡��
int IndexPitchToLSHVectorNote(map<string , vector<float>> &indexSongName, 
	int noteMaxFrame, int NLSHsize, int maxFrame, 
	vector<vector<float>> &LSHVector, map<unsigned long , pair<string,pair<short,short>>> &IndexLSH)
{
	map<string , vector<float>>::iterator indexIte;	//����pv�������еĵ�����
	unsigned long pointNum=0;	//��ȡLSH�����
	typedef map<unsigned long , pair<string,pair<short,short>>>::value_type IndexType;	//LSH������ʽ

	//��������pv�ļ�
	for (indexIte = indexSongName.begin(); indexIte!=indexSongName.end(); indexIte++)
	{
		map<string , vector<float>>::value_type SongTone;

		string SongName(indexIte->first);	//��ǰpv�ļ���
		int sizeTone=indexIte->second.size();	//��ǰһά�������е���

		//������ǰpv�ļ���һά��������
		for (int pos=0; pos<sizeTone && pos<maxFrame; )
		{
			float currentTone=1000;	//�浱ǰ����
			short frameNow = 0;		//��ǰLSH�����֡��
			int oneNoteDuration = 0;	//��һ������������֡��
			float TheFirstNote = indexIte->second[pos];	//��һ������
			
			vector <float> temp;	//��ǰLSH���õ�֡
			vector <float> LSHVectorDimention;	//�洢��ǰ��LSH��

			int CuNoteNum=0;	//��ǰ���߳���֡��

			//��posλ�ÿ�ʼ��ȡһ��LSH��
			for (int i=pos; i<sizeTone; i++)
			{
				frameNow++;		//��ǰLSH�����֡��
				temp.push_back(indexIte->second[i]);	//��ǰLSH���õ�֡

				if (TheFirstNote==indexIte->second[i])	//��¼��һ�����߳���֡��
					oneNoteDuration++;
				else	//��һ�����߽�������TheFirstNote��Ϊ���ֵ��ֹoneNoteDuration�����ۼ�
					TheFirstNote=1000;

				//��ǰLSH��ά�������ޣ��ҵ�ǰ���ߺ�ǰһ����ͬ����ǰLSH���ȡ���
				if (LSHVectorDimention.size()==NLSHsize && currentTone !=indexIte->second[i])
				{
					break;
				}

				if (currentTone !=indexIte->second[i])	//��ǰ���ߺ�ǰһ����ͬ
				{
					CuNoteNum=0;
					currentTone=indexIte->second[i];	//��¼��ǰ����
					LSHVectorDimention.push_back(indexIte->second[i]);	//��ǰ����¼��LSH��ĩβ
				}
				else	//��ǰһ��������ͬ
					CuNoteNum++;	//��ǰ���߳���֡������

				//��ǰLSH��ά�������ޣ��ҵ�ǰ���߳���֡�������֡����ǰLSH���ȡ���
				if (LSHVectorDimention.size()==NLSHsize && CuNoteNum>=noteMaxFrame)
				{
					break;
				}
				if (CuNoteNum>=noteMaxFrame)	//��ǰ���߳���֡�������֡���з�
				{
					CuNoteNum=0;	//����֡����������
					currentTone=indexIte->second[i];
					LSHVectorDimention.push_back(indexIte->second[i]);	//¼��LSH��ĩβ
				}
			}

			if (LSHVectorDimention.size()==NLSHsize)	//��ȡ��һ��������LSH��
			{
				pointNum++;	//LSH�����
				//����LSH��������ţ���ʼ����λ�ã�����֡��
				IndexLSH.insert(IndexType(pointNum,make_pair(SongName,make_pair(pos,frameNow))));
				float mean=MeanLSH(temp);	//��ǰLSH��������֡�ľ�ֵ
				MeanNoteLSH(LSHVectorDimention,mean);	//����ֵ
				LSHVector.push_back(LSHVectorDimention);	//����LSH�㼯
			}
			else	//��ǰpv�ļ���ȡ����1��������LSH�㣬����
				break;

			//LSH����Ϊ��һ�������ĳ���֡�����Ҳ������֡
			if (oneNoteDuration>=noteMaxFrame)
				pos+=noteMaxFrame;
			else
				pos+=oneNoteDuration;
		}
	}
	ofstream outf("wav.result",ofstream::app);
	outf<<"NLSH�ܵ�����"<<pointNum<<endl;
	cout<<"NLSH�ܵ�����"<<pointNum<<endl;
	outf.close();

	return 0;
}

//��һά�������г�ȡLSH��
//���룺indexSongName��pv�ļ����Ͷ�Ӧ��һά��������
//StepFactor��ѡ��LSH��ļ�����������֡��ȡһ����
//LSHsize��LSH������һ��LSH��Ĵ�С
//LSHshift��LSH����
//maxFrame��һά�����������֡��
//�����LSHVector��LSH�㣬ÿ����Ϊһ��20ά����������
//IndexLSH,	LSH��������¼��LSHVector����ţ���·�����ļ�������ʼ����λ��
int IndexPitchToLSHVector(map<string , vector<float>> &indexSongName, 
	int StepFactor, int LSHsize, int LSHshift, int maxFrame, 
	vector<vector<float>> &LSHVector, map<unsigned long , pair<string,short>> &IndexLSH)
{
	map<string , vector<float>>::iterator indexIte;	//����pv�������еĵ�����
	unsigned long pointNum=0;	//��ȡLSH�����
	typedef map<unsigned long , pair<string,short>>::value_type IndexType;	//LSH������ʽ	

	//��������pv�ļ�
	for (indexIte=indexSongName.begin(); indexIte!=indexSongName.end(); indexIte++)
	{
		string SongName(indexIte->first);	//��ǰ�ļ���
		int sizeTone=indexIte->second.size();	//һά�������е���

		//������ǰ�ļ���һά��������
		for (int pos=2; pos<sizeTone-StepFactor*(LSHsize+1) && pos<maxFrame; pos+=LSHshift)
		{
			pointNum++;
			double currentTone = indexIte->second[pos];	//��ǰ����

			IndexLSH.insert(IndexType(pointNum,make_pair(SongName,pos)));	//����LSH��������ţ���·�����ļ�������ʼ����λ��
			
			vector <float> LSHVectorDimention;	//�洢��ǰ��LSH��

			//��ȡһ��LSH��
			for (int i=pos; i<pos+StepFactor*LSHsize; i+=StepFactor)
			{
				vector <double > temp;

				//ȡStepFactor�������ֵ����LSH����
				for (int k=0;k<StepFactor;k++)
				{
					temp.push_back(indexIte->second[i+k]);
				}
				stable_sort(temp.begin(),temp.end());
				LSHVectorDimention.push_back(temp[StepFactor/2]);	
			}

			MinusMean(LSHVectorDimention);		//����ֵ
			LSHVector.push_back(LSHVectorDimention);	//����LSH�㼯
		}
	}
	ofstream outf("wav.result",ofstream::app);
	outf<<"LSH�ܵ�����"<<pointNum<<endl;
	cout<<"LSH�ܵ�����"<<pointNum<<endl;
	outf.close();
	return 0;
}

//��LSH���������ж����ݼ�������dataSet�У�����ÿ��LSH��ʱ��¼��ź�LSH���ƽ����
//���룺LSHVector��LSH�㼯
PPointT * readDataSetFromVector(vector<vector<float>> &LSHVector )
{
	IntT nPoints = LSHVector.size();	//LSH�㼯��С����pv�ļ���
	PPointT *dataSetPoints = NULL;		//���صĽ��

	FAILIF(NULL == (dataSetPoints = (PPointT*)MALLOC(nPoints * sizeof(PPointT))));	//���ٿռ�

	for(IntT i = 0; i < nPoints; i++){
		//dataSetPoints[i]->coordinatesΪһ��LSH��
		//dataSetPoints[i]->sqrlengthΪ�õ�ÿ�����ߵ�ƽ����
		dataSetPoints[i] = readPointVector(LSHVector[i]);	
		dataSetPoints[i]->index = i;
	}

	return dataSetPoints;
}

//����LSH�㵽PPoint�У�ͳ��ƽ����
PPointT readPointVector(vector<float> &VectorTone)
{
	PPointT p;
	float sqrLength = 0;	
	IntT pointsDimension = VectorTone.size();	//ά��

	FAILIF(NULL == (p = (PPointT)MALLOC(sizeof(PointT))));
	FAILIF(NULL == (p->coordinates = (float*)MALLOC(pointsDimension * sizeof(float))));

	for(IntT d = 0; d < pointsDimension; d++)
	{
		p->coordinates[d]=VectorTone[d];
		sqrLength += SQR(p->coordinates[d]);
	}
	p->index = -1;
	p->sqrLength = sqrLength;	//��¼ƽ����
	return p;
}


//��ȡһά�������е�NLSH��
//���룺queryPitch����ѯ��һά��������
//noteMinFrame��������̳���֡����������ȥ��
//noteMaxFrame�����������֡�����������з�
//NLSHsize,NLSH��ά��
//�����posPairvector����¼LSH�����ʼλ�úͳ�������
//LSHQueryVectorLinearStretching����¼LSH��
int QueryPitchToLSHVectorLinearStretchingShortToMoreNoteFirst(vector<pair<short, short>>& posPairvector, 
	vector <float> &queryPitch,vector <vector <vector<float>>> &LSHQueryVectorLinearStretching,
	int noteMinFrame,int noteMaxFrame,int NLSHsize)
{
	int sizeTone = queryPitch.size();	//һά�������г���
	set<pair<short, short>> posPairTemp;	//��¼ÿ��NLSH�����ʼλ�úͳ���ʱ��
	vector <vector <float> >LSHQueryVectorOneStretch;	//һ�������Ľ��
	long OneSongPoint=0;	//LSH�����
	static int numEntry = 0;	//����˺�������
	static long QueryPoint20Demention = 0;	//LSH���ۼƼ���

	numEntry++;	//����˺����Ĵ���

	//������ѯ��һά��������
	for (int pos=0;pos<sizeTone;)
	{
		float currentTone = 1000;	//��¼��ǰ����
		short frameNow = 0;		//��ǰNLSH�����֡��
		int oneNoteDuration = 0;	//��һ������������֡��
		float TheFirstNote = queryPitch[pos];	//��һ������

		vector <float> temp;	//��ǰNLSH���õ�֡
		vector <float> LSHVector20Dimention;	//��ǰLSH��

		int CuNoteNum=0;	//��ǰ���߳���֡��

		//��posλ�ÿ�ʼ��ȡһ��NLSH��
		for (int i=pos;i<sizeTone;i++)
		{
			frameNow++;	//��ǰLSH�����֡��
			temp.push_back(queryPitch[i]);

			if (TheFirstNote == queryPitch[i])	//��¼��һ����������ʱ��
				oneNoteDuration++;
			else
				TheFirstNote=1000;	//��һ�����߽�������TheFirstNote��Ϊ���ֵ��ֹoneNoteDuration�����ۼ�

			//��ǰLSH��ά�������ޣ��ҵ�ǰ���ߺ�ǰһ����ͬ����ǰLSH���ȡ���
			if (LSHVector20Dimention.size()==NLSHsize  && currentTone !=queryPitch[i])
			{
				break;
			} 

			if (currentTone !=queryPitch[i])	//��ǰ���ߺ�ǰһ����ͬ
			{
				if(CuNoteNum<=noteMinFrame)	//ǰһ�����߳���֡��С�ڵ������֡��
				{
					if (!LSHVector20Dimention.empty())	//�����Ȳ����ǰһ������ȥ��
					{
						LSHVector20Dimention.pop_back();
					}
				}
				else	//ǰһ�����߳���֡���������֡��
				{
					CuNoteNum=0;	
				}
				currentTone=queryPitch[i];		//��¼��ǰ����
				LSHVector20Dimention.push_back(queryPitch[i]);	//��ǰ����¼��LSH��ĩβ
			}
			else	//��ǰһ��������ͬ
			{
				CuNoteNum++;	//��ǰ���߳���֡������
			}

			//��ǰLSH��ά�������ޣ��ҵ�ǰ���߳���֡�������֡����ǰLSH���ȡ���
			if (LSHVector20Dimention.size()==NLSHsize && CuNoteNum>=noteMaxFrame)
			{
				break;
			}
			if (CuNoteNum >= noteMaxFrame)	//��ǰ���߳���֡�������֡���з�
			{
				CuNoteNum=0;
				currentTone=queryPitch[i];
				LSHVector20Dimention.push_back(queryPitch[i]);	//¼��LSH��ĩβ
			}
		}

		if (LSHVector20Dimention.size()==NLSHsize)	//��ȡ��һ��������LSH��
		{
			OneSongPoint++;	//LSH�����
			if (posPairTemp.count(make_pair(pos,frameNow)))	//��ǰLSH���Ѵ���
			{
				;
			} 
			else
			{
				posPairTemp.insert(make_pair(pos,frameNow));	//��¼��ǰLSH�����ʼλ�úͳ�������
				posPairvector.push_back(make_pair(pos,frameNow));	//��¼��ǰLSH�����ʼλ�úͳ�������
				float mean = MeanLSH(temp);	//��ǰLSH��������֡�ľ�ֵ
				MeanNoteLSH(LSHVector20Dimention,mean);	//����ֵ
				LSHQueryVectorOneStretch.push_back(LSHVector20Dimention);	//����LSH�㼯β
			}
		}
		else	//��ǰpv�ļ���ȡ����1��������LSH�㣬����
			break;

		//LSH����Ϊ��һ�������ĳ���֡�����Ҳ������֡
		if (oneNoteDuration>=noteMaxFrame)
		{
			pos+=noteMaxFrame;
		} 
		else
		{
			pos+=oneNoteDuration;
		}
	}

	LSHQueryVectorLinearStretching.push_back(LSHQueryVectorOneStretch);	//���뱾�γ�ȡ��LSH�㼯

	QueryPoint20Demention += OneSongPoint;	//��¼LSH����

	if (numEntry%100==0 || numEntry >=330)
	{
		ofstream outf("wav.result",ofstream::app);
		outf<<"��ǰ��NLSH������"<<OneSongPoint<<" ȫ������NLSH����:"<<QueryPoint20Demention<<endl;
		cout<<"��ǰ��NLSH������"<<OneSongPoint<<" ȫ������NLSH����:"<<QueryPoint20Demention<<endl;
		outf.close();
	}

	return 0;
}

//������������ȡLSH��
//���룺queryPitch��һά��������
//FloorLevel����ʼ�������ӣ�UpperLimit�������������ޣ�StretchStep����������
//stepFactor����ȡ�����stepRatio����ȡ���ƣ�LSHsize����ȡ������recur���ϲ㺯����ѭ�����
//�����LSHQueryVectorLinearStretching���������������µ�LSH�㼯
int QueryPitchToLSHVectorLinearStretchingShortToMore(vector <float> &queryPitch,
	vector <vector <vector<float>>> &LSHQueryVectorLinearStretching,
	float FloorLevel, float UpperLimit,int stepFactor,float stepRatio,float StretchStep,int recur,int LSHsize)
{
	long OneSongPoint=0;	//��¼LSH����
	static int numEntry=0;	//��¼����˺����Ĵ���
	numEntry++;
	static long QueryPoint20Demention=0;	//��¼LSH�ܵ���

	//����������������
	for (; FloorLevel<UpperLimit+StretchStep; FloorLevel+=StretchStep)
	{
		if ((recur==1 || recur==2 )&& FloorLevel<=1.01 && FloorLevel>=0.99)
		{
			continue;
		}
		if (recur==2 && ((FloorLevel<=0.81 && FloorLevel>=0.79)||(FloorLevel<=1.21 && FloorLevel>=1.19)||(FloorLevel<=1.41 && FloorLevel>=1.39)))
		{
			continue;
		}
		vector <float> queryPitchStretch;	//����������Ĵ�

		//����������������������FloorLevel��������queryPitch���õ�queryPitchStretch
		StringToString(queryPitch,queryPitchStretch,FloorLevel);

		int sizeTone = queryPitchStretch.size();	//�����������ά��
		int StepFactorCurrent = int(stepFactor*stepRatio*FloorLevel);	//�����������֡��
		vector <vector <float> >LSHQueryVectorOneStretch;	//��������������ĳ�ȡ��LSH�㼯

		//�������������һά�������к��ȡLSH��
		for (int pos=0; pos<sizeTone-stepFactor*(LSHsize+1); pos+=StepFactorCurrent)
		{
			vector <float> LSHVector20Dimention;	//��¼���γ�ȡ��LSH��

			//��posλ�ÿ�ʼ��ȡһ��LSH��
			for (int i=pos; i<pos+stepFactor*LSHsize; i+=stepFactor)
			{
				vector<float> temp;

				//ȡStepFactor�������ֵ����LSH��
				for (int k=0;k<stepFactor;k++)
				{
					temp.push_back(queryPitchStretch[i+k]);
				}
				stable_sort(temp.begin(),temp.end());
				LSHVector20Dimention.push_back(temp[stepFactor/2]);
			}
			MinusMean(LSHVector20Dimention);	//����ֵ
			OneSongPoint++;		//LSH�����
			LSHQueryVectorOneStretch.push_back(LSHVector20Dimention);	//����LSH�㼯
		}
		LSHQueryVectorLinearStretching.push_back(LSHQueryVectorOneStretch);	//���γ�ȡ��LSH�㼯�����������
	}
	QueryPoint20Demention += OneSongPoint;	//�ۼƼ�¼LSH����
	
	if (numEntry%100==0 || numEntry >=330)
	{
		ofstream outf("wav.result",ofstream::app);
		outf<<"��ǰ�������������LSH������"<<OneSongPoint<<" ȫ����������������LSH����:"<<QueryPoint20Demention<<endl;
		cout<<"��ǰ�������������LSH������"<<OneSongPoint<<" ȫ����������������LSH����:"<<QueryPoint20Demention<<endl;
		outf.close();
	}
	
	return 0;
}


int IndexSignToQueryAndDataVectorHummingBeginQueryLengthFixedAndNoClearMatchLeast(vector <float> &DisCandidates,vector<IntT> &IndexCandidatesStretch,vector<IntT> &CandidatesNumStretch,float stretchFactor,
															  map<unsigned long , pair<string,short>> &IndexLSH,int stepFactor,vector <float> queryPitch,
															   vector<float>  &queryX, vector< vector<float>  >&dataY,
															  map<string , vector<float>> &indexSongName,vector <string> &SongNameMapToDataY,
															  int &CandidatesSizeInDWT,int MatchBeginPos,map <string , short > &SongMapPosition,
															  vector<float>  &CandidatesDataYDis)//�����������ӷ��غ�ѡ�����Լ���Ӧ�ĸ�������
{  
	//ƥ��������Ŀ������ȷƥ�����Ŀ����
	int posBegin=10;//�뿪ͷ�ȽϽ���С�ڳ���1/posBegin��ȫ������ƥ��
	queryX.clear();
	StringToString(queryPitch,queryX,1);
	int sizePoints=CandidatesNumStretch.size();
	int sizeCandidates=0;
	int QueryLSHPos=0;
	int DifferPos=0;
	int sizeQuery=queryX.size();
	sizeQuery=sizeQuery*stretchFactor;
	pair<string, short> SongNamePos;
	map<string , vector<float>> :: iterator SongTone;
	vector<float>  DataYOnePoint;
	vector <string> :: iterator SongIter;
	vector< vector<float> >  :: iterator SongPitchIter;
	vector<float>  :: iterator SongDisIter;

	for (int i=0;i<sizePoints;i++)
	{
		int sizeCandidatesCurrent=sizeCandidates;
		int CurrentNum=CandidatesNumStretch[i];
		for (int j=sizeCandidatesCurrent;j<sizeCandidatesCurrent+CurrentNum;j++)
		{
			sizeCandidates++;

			if (IndexLSH.count(IndexCandidatesStretch[j]))
			{
				QueryLSHPos=i*stepFactor;
				SongNamePos=IndexLSH.find(IndexCandidatesStretch[j])->second;
				DifferPos=SongNamePos.second-QueryLSHPos;
				if (indexSongName.count(SongNamePos.first))
				{
					DataYOnePoint.clear();
					SongTone=indexSongName.find(SongNamePos.first);
					int sizeSong=SongTone->second.size();
					if (DifferPos>=0 && DifferPos<sizeQuery/MatchBeginPos &&DifferPos+sizeQuery/MatchBeginPos<sizeSong)//��֤���Ȳ�����ĩβ����,����queryһ��
					{
						for (int k=DifferPos;k<DifferPos+sizeQuery && k<sizeSong;k++)
						{
							DataYOnePoint.push_back(SongTone->second[k]);//���˵㸴�ƹ�ȥ
						}
						if (SongMapPosition.count(SongNamePos.first))
						{
							short posInMap=SongMapPosition.find(SongNamePos.first)->second;
							if (posInMap>DifferPos)//������б��е�λ�ÿ�ǰ
							{
								if	(posInMap>(short)(sizeQuery/posBegin))//�����ǰ�Ƚ�Զ
								{
									SongIter=find(SongNameMapToDataY.begin(),SongNameMapToDataY.end(),SongNamePos.first);
									SongPitchIter=dataY.begin()+(SongIter-SongNameMapToDataY.begin());
									SongDisIter=CandidatesDataYDis.begin()+(SongIter-SongNameMapToDataY.begin());
									SongNameMapToDataY.erase(SongIter);
									CandidatesDataYDis.erase(SongDisIter);
									dataY.erase(SongPitchIter);
									dataY.push_back(DataYOnePoint);
									CandidatesDataYDis.push_back(DisCandidates[j]);
									SongNameMapToDataY.push_back(SongNamePos.first);
									SongMapPosition[SongNamePos.first]=DifferPos;
								}
								else
								{
									dataY.push_back(DataYOnePoint);
									CandidatesDataYDis.push_back(DisCandidates[j]);
									SongNameMapToDataY.push_back(SongNamePos.first);
									SongMapPosition[SongNamePos.first]=sizeQuery/posBegin;
									CandidatesSizeInDWT++;
								}
							}
						}
						else
						{
							SongMapPosition.insert(make_pair(SongNamePos.first,DifferPos));
							dataY.push_back(DataYOnePoint);
							CandidatesDataYDis.push_back(DisCandidates[j]);
							SongNameMapToDataY.push_back(SongNamePos.first);
							CandidatesSizeInDWT++;

						}
					}
					else if (DifferPos<0 && DifferPos >-(sizeQuery/MatchBeginPos) &&  DifferPos+sizeQuery<sizeSong)
					{
						for (int k=0;k<sizeQuery && k<sizeSong;k++)
						{
							DataYOnePoint.push_back(SongTone->second[k]);//���˵㸴�ƹ�ȥ

						}
						
						if (!SongMapPosition.count(SongNamePos.first))//����˸���û��
						{
							SongMapPosition.insert(make_pair(SongNamePos.first,abs(DifferPos)));
							dataY.push_back(DataYOnePoint);
							CandidatesDataYDis.push_back(DisCandidates[j]);
							SongNameMapToDataY.push_back(SongNamePos.first);
							CandidatesSizeInDWT++;
						}						
						else
						{
							short posInMap=SongMapPosition.find(SongNamePos.first)->second;
							if	(posInMap> abs(DifferPos) )//���б��еľ��뿪ͷ��
							{
								if	(posInMap>(short)(sizeQuery/posBegin))//�����ǰ�Ƚ�Զ
								{
									SongIter=find(SongNameMapToDataY.begin(),SongNameMapToDataY.end(),SongNamePos.first);
									SongDisIter=CandidatesDataYDis.begin()+(SongIter-SongNameMapToDataY.begin());
									SongPitchIter=dataY.begin()+(SongIter-SongNameMapToDataY.begin());
									SongNameMapToDataY.erase(SongIter);
									dataY.erase(SongPitchIter);
									CandidatesDataYDis.erase(SongDisIter);
									dataY.push_back(DataYOnePoint);
									CandidatesDataYDis.push_back(DisCandidates[j]);
									SongNameMapToDataY.push_back(SongNamePos.first);
									SongMapPosition[SongNamePos.first]=abs(DifferPos);
								}
								else
								{
									dataY.push_back(DataYOnePoint);
									CandidatesDataYDis.push_back(DisCandidates[j]);
									SongNameMapToDataY.push_back(SongNamePos.first);
									SongMapPosition[SongNamePos.first]=sizeQuery/posBegin;
									CandidatesSizeInDWT++;
								}
							}
						}
					}
					else
						CandidatesNumStretch[i]--;//�˾����д洢ʵ�ʾ�ȷƥ�����Ŀ
				}
				else
					CandidatesNumStretch[i]--;
			}
			else
				CandidatesNumStretch[i]--;
		}
	}
	return 0;

}


int IndexSignToQueryAndDataVectorHummingMatchLeastALL(vector<IntT> &IndexCandidatesStretch,vector<IntT> &CandidatesNumStretch,float stretchFactor,
																				  map<unsigned long , pair<string,short>> &IndexLSH,int stepFactor,vector <vector <double>> queryPitch,
																				  vector< vector<double> > &queryX,vector< vector< vector<double> > >&dataY,
																				  map<string , vector<vector<double>>> &indexSongName,vector <string> &SongNameMapToDataY,
																				  int &CandidatesSizeInDWT,int MatchBeginPos,map <string , short > &SongMapPosition)//�����������ӷ��غ�ѡ�����Լ���Ӧ�ĸ�������
{  
	//ƥ��������Ŀ������ȷƥ�����Ŀ����
	int posBegin=30;//�뿪ͷ�ȽϽ���С�ڳ���1/posBegin��ȫ������ƥ��
	queryX.clear();
	int sizePoints=CandidatesNumStretch.size();
	int sizeCandidates=0;
	int QueryLSHPos=0;
	int DifferPos=0;
	int sizeQuery=queryX.size();
	sizeQuery=sizeQuery*stretchFactor;
	pair<string, short> SongNamePos;
	map<string , vector<vector<double>>> :: iterator SongTone;
	vector< vector<double> > DataYOnePoint;
	vector <string> :: iterator SongIter;
	vector< vector< vector<double> > > :: iterator SongPitchIter;

	for (int i=0;i<sizePoints;i++)
	{
		int sizeCandidatesCurrent=sizeCandidates;
		int CurrentNum=CandidatesNumStretch[i];
		for (int j=sizeCandidatesCurrent;j<sizeCandidatesCurrent+CurrentNum;j++)
		{
			sizeCandidates++;

			if (IndexLSH.count(IndexCandidatesStretch[j]))
			{
				QueryLSHPos=i*stepFactor;
				SongNamePos=IndexLSH.find(IndexCandidatesStretch[j])->second;
				DifferPos=SongNamePos.second-QueryLSHPos;
				if (indexSongName.count(SongNamePos.first))
				{
					DataYOnePoint.clear();
					SongTone=indexSongName.find(SongNamePos.first);
					int sizeSong=SongTone->second.size();
					if (DifferPos>=0  &&DifferPos+sizeQuery/2<sizeSong)//��֤���Ȳ�����ĩβ����,����queryһ��
					{
						for (int k=DifferPos;k<DifferPos+sizeQuery && k<sizeSong;k++)
						{
							DataYOnePoint.push_back(SongTone->second[k]);//���˵㸴�ƹ�ȥ

						}
						SongMapPosition.insert(make_pair(SongNamePos.first,DifferPos));
						dataY.push_back(DataYOnePoint);
						SongNameMapToDataY.push_back(SongNamePos.first);
						CandidatesSizeInDWT++;
					}
					else if (DifferPos<0 && DifferPos >-(sizeQuery/MatchBeginPos) &&  DifferPos+sizeQuery<sizeSong)
					{
						for (int k=0;k<sizeQuery && k<sizeSong;k++)
						{
							DataYOnePoint.push_back(SongTone->second[k]);//���˵㸴�ƹ�ȥ
						}
						SongMapPosition.insert(make_pair(SongNamePos.first,abs(DifferPos)));
						dataY.push_back(DataYOnePoint);
						SongNameMapToDataY.push_back(SongNamePos.first);
						CandidatesSizeInDWT++;
					}
					else
						CandidatesNumStretch[i]--;//�˾����д洢ʵ�ʾ�ȷƥ�����Ŀ
				}
				else
					CandidatesNumStretch[i]--;
			}
			else
				CandidatesNumStretch[i]--;
		}
	}
	return 0;
}


int LSHresultRate(char *wavename,int sizeQuery,int stepFactor,float stretch,vector<vector<IntT>> &IndexCandidatesStretch,
			  map<unsigned long , pair<string,short>> &IndexLSH,map<unsigned long , pair<string,pair<short,short>>> &IndexLSHNote,
			  float StretchStep,vector<vector<float>> &IndexCandidatesDis)
{
	string   songNameFive;
	int SizePoints=IndexCandidatesStretch.size();
	int QueryLSHPos=0;
	float stretchCurrent=0;
	static int find1=0;
	static int findSmall=0;
	static int returnPointNum=0;
	static int allWav=0;
	static int LSHresultMatch[300]={0};
	allWav++;
	ofstream outf("LSH.result",ofstream::app);//ʶ���������ļ�
	if (allWav%500==0 || allWav>=330)
	{
		outf<<wavename<< endl;
		cout<<wavename<< endl;
	}

	bool BigRe=FALSE;
	for (int i=0;i<SizePoints;i++)
	{
		stretchCurrent=sizeQuery*(stretch+i*StretchStep);
		int sizeC=IndexCandidatesStretch[i].size();

		for (int j=0;j<sizeC;j++)
		{
			if (IndexLSH.count(IndexCandidatesStretch[i][j]))
			{
				if (i==SizePoints-1)
				{
					songNameFive=IndexLSHNote.find(IndexCandidatesStretch[i][j])->second.first;
				}
				else
					songNameFive=IndexLSH.find(IndexCandidatesStretch[i][j])->second.first;
				QueryLSHPos=IndexLSH.find(IndexCandidatesStretch[i][j])->second.second-i*stepFactor;
				songNameFive.erase(0,6);
				int posSong=songNameFive.rfind("pv");
				//songNameFive.erase(5,6);
				songNameFive.erase(posSong-1,3);
				string nameSong(wavename);

				string::size_type beginiter=0;
				string::size_type enditer=0;
				//beginiter=nameSong.rfind("\");
				enditer=nameSong.rfind(".wav");
				string nameSongResult;
				nameSongResult.assign(nameSong,enditer-4,4);
				if (nameSongResult== songNameFive && QueryLSHPos>-(stretchCurrent/10) 
					&& QueryLSHPos<stretchCurrent/10)
				{
					if	(!BigRe)
					{find1++;}
					findSmall++;
					if (allWav%500==0 || allWav>=4430)
					{
						cout<<"find: "<<find1<<"findSamll: "<<findSmall<<endl;
						outf<<"find: "<<find1<<"findSamll: "<<findSmall<<endl;
						cout<<"allwav: "<<allWav<<endl;
						outf<<"allwav: "<<allWav<<endl;
					}

					j=sizeC;
					i=SizePoints;
				}
				else if	(nameSongResult== songNameFive && QueryLSHPos>-(stretchCurrent/2) 
					&& QueryLSHPos<stretchCurrent/2 && !BigRe)
				{
					find1++;
					if (allWav%500==0 || allWav>=4430)
					{
						cout<<"find: "<<find1<<endl;
						outf<<"find: "<<find1<<endl;
						cout<<"allwav: "<<allWav<<endl;
						outf<<"allwav: "<<allWav<<endl;
					}
					BigRe=TRUE;
				}
			}
		}
	}

	int currentNum=0;
	ofstream outf2("wav.result",ofstream::app);//ʶ���������ļ�
	ofstream outfCandidates("wavCandidates.result",ofstream::app);

	static int num1=0;
	static int num2=0;
	static int num3=0;
	static int num4=0;
	static int num5=0;
	static int num10=0;
	static int num20=0;
	static int num50=0;
	static int num100=0;
	static int num200=0;
	static int num400=0;
	static int num800=0;
	static int num1200=0;
	static int num1600=0;
	static int num2000=0;
	static int num2400=0;
	static int num2800=0;
	static int num3200=0;
	static int num3600=0;
	static int num4000=0;
	static int findall=0;

	static int rightNum[49]={0};
	static int wrongNum[49]={0};
	map <string, int> songNum;
	map <float, string> songUl;
	for (int i=0;i<SizePoints;i++)
	{
		stretchCurrent=sizeQuery*(stretch+i*StretchStep);
		int sizeC=IndexCandidatesStretch[i].size();
		returnPointNum+=sizeC;
		for (int j=0;j<sizeC;j++)
		{
			if (IndexLSH.count(IndexCandidatesStretch[i][j]))
			{

				if (i==SizePoints-1)
				{
					songNameFive=IndexLSHNote.find(IndexCandidatesStretch[i][j])->second.first;
				}
				else
					songNameFive=IndexLSH.find(IndexCandidatesStretch[i][j])->second.first;

				QueryLSHPos=IndexLSH.find(IndexCandidatesStretch[i][j])->second.second-i*stepFactor;
				songNameFive.erase(0,6);
				int posSong=songNameFive.rfind("pv");
				songNameFive.erase(posSong-1,3);
				songUl.insert(make_pair(IndexCandidatesDis[i][j],songNameFive));
				if (songNum.count(songNameFive))
				{
					songNum[songNameFive]++;
				}
				else
					songNum.insert(make_pair(songNameFive,0));
				string nameSong(wavename);

				string::size_type beginiter=0;
				string::size_type enditer=0;
				enditer=nameSong.rfind(".wav");
				string nameSongResult;
				nameSongResult.assign(nameSong,enditer-4,4);
				if (nameSongResult== songNameFive )
				{
					currentNum++;
				}
			}
		}
	}
	if (currentNum<300)
	{
		LSHresultMatch[currentNum]++;
	}
	if (allWav%500==0 || allWav>=4430)
	{
		cout<<"returnAllPoint: "<<returnPointNum<<endl;
		outf<<"returnAllPoint: "<<returnPointNum<<endl;
		cout<<"RightPoints: ";
		outf<<"RightPoints: ";
		int allmatchSong=0;
		for (int i=0;i<300;i++)
		{
			if (i>0)
			{
				allmatchSong+=LSHresultMatch[i];
			}
			cout<<LSHresultMatch[i]<<"  ";
			outf<<LSHresultMatch[i]<<"  ";
		}
		cout<<"LSHFindAllRightSong: "<<allmatchSong<<endl;
		outf<<"LSHFindAllRightSong: "<<allmatchSong<<endl;
		cout<<endl;
		outf<<endl;
	}

	outf.close();
	map<float ,string> songDis;

	vector <float> Dis;
	set <int> TempDis;
	float fl=0.00001;
	int numfl=1;
	map <string, int> ::const_iterator iterdis=songNum.begin();
	for (;iterdis!=songNum.end();iterdis++)
	{
		numfl++;
		Dis.push_back(iterdis->second+fl*numfl);

		songDis.insert(make_pair(iterdis->second+fl*numfl,iterdis->first));
	}

	set <string> SongFindAlready;
	static int total=0;
	int findtop5=0;
	int findNum=0;
	int accurateFind=0;
	stable_sort(Dis.begin(),Dis.end());
	int totalDis=Dis.size();
	for (int i=0,k=0;i!=songDis.size()/*&&i!=1000 && k<500*/;i++)
	{
		songNameFive=songDis.find(Dis[totalDis-i-1])->second;
		
		accurateFind=SongFindAlready.size();
		string nameSong(wavename);

		string::size_type beginiter=0;
		string::size_type enditer=0;
		enditer=nameSong.rfind(".wav");
		string nameSongResult;
		nameSongResult.assign(nameSong,enditer-4,4);

		int top5R=atoi(nameSongResult.c_str());
		if (top5R<0 || top5R>48)
		{
			top5R=0;
		}
		if (SongFindAlready.count(songNameFive))
		{
			;
		}
		else
		{
			k++;
			if (k<5)
			{
				if (total%200==0 || total >=4410)
				{
					outf<<songNameFive<<endl;
					cout<<songNameFive<<endl;
					outf<<"the distance: "<<Dis[i]<<endl;
					cout<<"the distance: "<<Dis[i]<<endl;
				}
			}

			SongFindAlready.insert(songNameFive);
			if (nameSongResult== songNameFive)
			{

				
				findall++;
				if (accurateFind==0)
				{
					num1++,num2++,num3++,num4++,num5++,num10++,num20++,num50++;num100++;num200++;
					rightNum[top5R]++;
					wrongNum[top5R]--;
					findtop5=1;
				}
				if (accurateFind==1)
				{
					num2++,num3++,num4++,num5++,num10++,num20++,num50++;num100++;num200++;
					rightNum[top5R]++;
					wrongNum[top5R]--;
					findtop5=1;
				}
				if (accurateFind==2)
				{
					num3++,num4++,num5++,num10++,num20++,num50++;num100++;num200++;
					rightNum[top5R]++;
					wrongNum[top5R]--;
					findtop5=1;
				}
				if (accurateFind==3){
					num4++,num5++,num10++,num20++,num50++;num100++;num200++;
					rightNum[top5R]++;
					wrongNum[top5R]--;
					findtop5=1;
				}
				if (accurateFind==4)
				{
					num5++,num10++,num20++,num50++;num100++;num200++;
					rightNum[top5R]++;
					wrongNum[top5R]--;
					findtop5=1;
				}
				if (accurateFind<=9 && accurateFind>4)
				{
					num10++,num20++,num50++;num100++;num200++;
				}
				if (accurateFind<=19 && accurateFind>9)
				{
					num20++,num50++;num100++;num200++;
				}
				if (accurateFind<=50 && accurateFind>19)
				{
					num50++;num100++;num200++;
				}
				if (accurateFind<=100 && accurateFind>50)
				{
					num100++;num200++;
				}
				if (accurateFind<=200 && accurateFind>100)
				{
					num200++;
				}
				if (findNum<=40)
				{
					num400++,num800++,num1200++,num1600++,num2000++,num2400++,num2800++,num3200++,num3600++,num4000++;
				}
				if (findNum<=80 && findNum>40)
				{
                    num800++,num1200++,num1600++,num2000++,num2400++,num2800++,num3200++,num3600++,num4000++;

				}
				if (findNum<=120 && findNum>80)
				{
					num1200++,num1600++,num2000++,num2400++,num2800++,num3200++,num3600++,num4000++;
				}
				if (findNum<=160 && findNum>120)
				{
					num1600++,num2000++,num2400++,num2800++,num3200++,num3600++,num4000++;
				}
				if (findNum<=200 && findNum>160)
				{
					num2000++,num2400++,num2800++,num3200++,num3600++,num4000++;
				}
				if (findNum<=400 && findNum>200)
				{
					num2400++,num2800++,num3200++,num3600++,num4000++;
				}
				if (findNum<=800 && findNum>400)
				{
					num2800++,num3200++,num3600++,num4000++;
				}
				if (findNum<=1200 && findNum>800)
				{
					num3200++,num3600++,num4000++;
				}
				if (findNum<=1600 && findNum>1200)
				{
					num3600++,num4000++;
				}
				if (findNum<=2000 && findNum>1600)
				{
					num4000++;
				}
				if (total%10==0 || total>=330)
				{
					cout<<"׼ȷ�ʣ�find1: "<<num1<<"  find2: "<<num2<<"  find3: "<<num3<<"  find4: "<<num4<<"  find5: "<<num5<<"  find10: "<<num10<<"  find20: "<<num20<<"  find50: "<<num50<<"  find100: "<<num100<<"  find200: "<<num200<<"  findall: "<<findall<<endl;
					cout<<"�ٻ��ʣ�find40: "<<num400<<"  find80: "<<num800<<"  find120: "<<num1200<<"  find160: "<<num1600<<"  find200: "<<num2000<<"  find400: "<<num2400<<"  find800: "<<num2800<<"  find1200: "<<num3200<<"  find1600: "<<num3600<<"  find2000: "<<num4000<<endl;
					outf2<<"׼ȷ�ʣ�find1: "<<num1<<"  find2: "<<num2<<"  find3: "<<num3<<"  find4: "<<num4<<"  find5: "<<num5<<"  find10: "<<num10<<"  find20: "<<num20<<"  find50: "<<num50<<"  find100: "<<num100<<"  find200: "<<num200<<"  findall: "<<findall<<endl;
					outf2<<"�ٻ��ʣ�find40: "<<num400<<"  find80: "<<num800<<"  find120: "<<num1200<<"  find160: "<<num1600<<"  find200: "<<num2000<<"  find400: "<<num2400<<"  find800: "<<num2800<<"  find1200: "<<num3200<<"  find1600: "<<num3600<<"  find2000: "<<num4000<<endl;
				}
			}
		}
		findNum+=Dis[totalDis-i-1];
	}
	total++;
	if (total==1000)
	{
		ofstream out1000("1000.result",ofstream::app);
		out1000<<"find1: "<<num1<<"  find2: "<<num2<<"  find3: "<<num3<<"  find4: "<<num4<<"  find5: "<<num5<<"  find10: "<<num10<<"  find20: "<<num20<<"  find30: "<<num50<<"  findall: "<<findall<<endl;
		out1000.close();
	}
	if (findtop5!=1)
	{
		ofstream outfError("error.result",ofstream::app);
		outfError<<wavename<<endl;
		outfError.close();
	}

	int allsongC=0;
	if (total%500==0 || total>=4428)
	{
		outf<<"��ȷ��ʹ���㣺"<<"  ";
		cout<<"��ȷ��ʹ���㣺"<<"  ";
		for (int i=0;i<49;i++)
		{
			outf2<<i<<" r:"<<rightNum[i]<<" w: "<<wrongNum[i]<<"  ";
			cout<<i<<" r:"<<rightNum[i]<<" w: "<<wrongNum[i]<<"  ";
		}
		outf2<<"all: "<<allsongC<<endl;
		cout<<"all: "<<allsongC<<endl;
		outf2<<wavename<<"�ܵĺ�ѡ������"<<Dis.size()<<" ȫ��������ѡ������"/*<<CandidatesDTWAll*/<< endl;
		outfCandidates<<wavename<<"�ܵĺ�ѡ������"<<Dis.size()<< endl;
		cout<<wavename<<"�ܵĺ�ѡ������"<<Dis.size()<<" ȫ��������ѡ������"/*<<CandidatesDTWAll*/<< endl;
	}
	for (int i=0;i<49;i++)
	{
		allsongC+=rightNum[i];
		allsongC+=wrongNum[i];
	}
	outfCandidates.close();
	if (total%10==0 || total>=4420)
	{
		cout<<"the total: "<<total<<endl;
		outf2<<"the total: "<<total<<endl;
	}
	return 0;
}


int LSHresult(char *wavename,int sizeQuery,int stepFactor,float stretch,vector<vector<IntT>> &IndexCandidatesStretch,
			  map<unsigned long , pair<string,short>> &IndexLSH,float StretchStep)
{
	string   songNameFive;
	int SizePoints=IndexCandidatesStretch.size();
	int QueryLSHPos=0;
	float stretchCurrent=0;
	static int find1=0;
	static int findSmall=0;
	static int returnPointNum=0;
	static int allWav=0;
	static int LSHresultMatch[300]={0};
	allWav++;
	ofstream outf("LSH.result",ofstream::app);//ʶ���������ļ�
	if (allWav%500==0 || allWav>=4430)
	{
		outf<<wavename<< endl;
		cout<<wavename<< endl;
	}
	
	bool BigRe=FALSE;
	for (int i=0;i<SizePoints;i++)
	{
		stretchCurrent=sizeQuery*(stretch+i*StretchStep);
		int sizeC=IndexCandidatesStretch[i].size();

		for (int j=0;j<sizeC;j++)
		{
			if (IndexLSH.count(IndexCandidatesStretch[i][j]))
			{
				songNameFive=IndexLSH.find(IndexCandidatesStretch[i][j])->second.first;
				QueryLSHPos=IndexLSH.find(IndexCandidatesStretch[i][j])->second.second-i*stepFactor;
				songNameFive.erase(0,6);
				int posSong=songNameFive.rfind("pv");
				songNameFive.erase(posSong-1,3);
				string nameSong(wavename);

				string::size_type beginiter=0;
				string::size_type enditer=0;
				enditer=nameSong.rfind(".wav");
				string nameSongResult;
				nameSongResult.assign(nameSong,enditer-4,4);
				if (nameSongResult== songNameFive && QueryLSHPos>-(stretchCurrent/10) 
					&& QueryLSHPos<stretchCurrent/10)
				{
					if	(!BigRe)
					{find1++;}
					findSmall++;
					if (allWav%500==0 || allWav>=4430)
					{
						cout<<"find: "<<find1<<"findSamll: "<<findSmall<<endl;
						outf<<"find: "<<find1<<"findSamll: "<<findSmall<<endl;
						cout<<"allwav: "<<allWav<<endl;
						outf<<"allwav: "<<allWav<<endl;
					}
					j=sizeC;
					i=SizePoints;

				}
				else if	(nameSongResult== songNameFive && QueryLSHPos>-(stretchCurrent/2) 
					&& QueryLSHPos<stretchCurrent/2 && !BigRe)
				{
					find1++;
					if (allWav%500==0 || allWav>=4430)
					{
						cout<<"find: "<<find1<<endl;
						outf<<"find: "<<find1<<endl;
						cout<<"allwav: "<<allWav<<endl;
						outf<<"allwav: "<<allWav<<endl;
					}
					BigRe=TRUE;
				}
			}
		}
	}

	int currentNum=0;
	for (int i=0;i<SizePoints;i++)
	{
		stretchCurrent=sizeQuery*(stretch+i*StretchStep);
		int sizeC=IndexCandidatesStretch[i].size();
		returnPointNum+=sizeC;
		for (int j=0;j<sizeC;j++)
		{
			if (IndexLSH.count(IndexCandidatesStretch[i][j]))
			{
				songNameFive=IndexLSH.find(IndexCandidatesStretch[i][j])->second.first;
				QueryLSHPos=IndexLSH.find(IndexCandidatesStretch[i][j])->second.second-i*stepFactor;
				songNameFive.erase(0,6);
				int posSong=songNameFive.rfind("pv");
				songNameFive.erase(posSong-1,3);
				string nameSong(wavename);

				string::size_type beginiter=0;
				string::size_type enditer=0;
				enditer=nameSong.rfind(".wav");
				string nameSongResult;
				nameSongResult.assign(nameSong,enditer-4,4);
				if (nameSongResult== songNameFive )
				{
					currentNum++;
				}
			}
		}
	}
	if (currentNum<300)
	{
		LSHresultMatch[currentNum]++;
	}
	if (allWav%500==0 || allWav>=4430)
	{
		cout<<"returnAllPoint: "<<returnPointNum<<endl;
		outf<<"returnAllPoint: "<<returnPointNum<<endl;
		cout<<"RightPoints: ";
		outf<<"RightPoints: ";
		int allmatchSong=0;
		for (int i=0;i<300;i++)
		{
			if (i>0)
			{
				allmatchSong+=LSHresultMatch[i];
			}
			cout<<LSHresultMatch[i]<<"  ";
			outf<<LSHresultMatch[i]<<"  ";
		}
		cout<<"LSHFindAllRightSong: "<<allmatchSong<<endl;
		outf<<"LSHFindAllRightSong: "<<allmatchSong<<endl;
		cout<<endl;
		outf<<endl;
	}
	
	outf.close();
	return 0;
}

int LSHFilter(char *wavename,map<string ,int> &LSHFilterMap,vector<IntT> &CandidatesFilter,
			  map<unsigned long , pair<string,short>> &IndexLSH)
{
	string   songNameFive;
	int SizePoints=CandidatesFilter.size();
	static int LSHresultMatch[500]={0};
	static int songNum=0;
	songNum++;
	ofstream outf("LSH.result",ofstream::app);//ʶ���������ļ�
	int currentNum=0;
	for (int i=0;i<SizePoints;i++)
	{
		if (IndexLSH.count(CandidatesFilter[i]))
		{
			songNameFive=IndexLSH.find(CandidatesFilter[i])->second.first;
			songNameFive.erase(0,6);
			int posSong=songNameFive.rfind("pv");
			songNameFive.erase(posSong-1,3);
			string nameSong(wavename);

			string::size_type beginiter=0;
			string::size_type enditer=0;
			enditer=nameSong.rfind(".wav");
			string nameSongResult;
			nameSongResult.assign(nameSong,enditer-4,4);
			if (nameSongResult== songNameFive )
			{
				currentNum++;
			}
		}
	}
	if (currentNum<500)
	{
		LSHresultMatch[currentNum]++;
	}
	if (songNum%500==0 || songNum>=4430)
	{
		cout<<"LSHFilter: "<<endl;
		outf<<"LSHFilter: "<<endl;
		cout<<"RightPoints: ";
		outf<<"RightPoints: ";
		int allmatchSong=0;
		for (int i=0;i<500;i++)
		{
			if (i>0)
			{
				allmatchSong+=LSHresultMatch[i];
			}
			cout<<LSHresultMatch[i]<<"  ";
			outf<<LSHresultMatch[i]<<"  ";
		}
		cout<<"LSHFindAllRightSong  Filter: "<<allmatchSong<<endl;
		outf<<"LSHFindAllRightSong  Filter: "<<allmatchSong<<endl;
		cout<<endl;
		outf<<endl;
	}
	
	outf.close();
	for (int i=0;i<SizePoints;i++)
	{
		if (IndexLSH.count(CandidatesFilter[i]))
		{
			songNameFive=IndexLSH.find(CandidatesFilter[i])->second.first;
			if (LSHFilterMap.count(songNameFive))
			{
				LSHFilterMap[songNameFive]++;
			}
			else
			    LSHFilterMap[songNameFive]=1;
		}
	}
	return 0;
}