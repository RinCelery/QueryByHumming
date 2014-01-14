/**************************************************************************/
/*             Function for reading data from WAV_PCM file                    */
/*         Return value:                                                  */
/*              (short int*): Pointer to the data buffer,if success       */
/*              NULL : If fail                                            */
/**************************************************************************/
#include<stdlib.h>
#include<math.h>
#include<stdio.h>
#include "define.h"
struct wavehead
{
	char sign[4];         //"RIFF"��־
    long int flength;     //�ļ����� 
    char wavesign[4];     //"WAVE"��־
    char fmtsign[4];      //"fmt"��־
	long int unused;      // �����ֽڣ�������
    short formattype;     //��ʽ���10HΪPCM��ʽ����������)
    short  channelnum;    //ͨ������������Ϊ1��˫����Ϊ2
	long int  samplerate; //�����ʣ�ÿ��������������ʾÿ��ͨ���Ĳ����ٶ�
    long int transferrate;//������Ƶ���ݴ������ʣ���ֵΪͨ������ÿ������λ����ÿ��
                          //��������λ����8������������ô�ֵ���Թ��ƻ������Ĵ�С
    short int adjustnum;  //���ݿ�ĵ����������ֽ���ģ�����ֵΪͨ������ÿ����������λֵ��8��
	                      //���������Ҫһ�δ�������ֵ��С���ֽ����ݣ��Ա㽫��ֵ���ڻ������ĵ�����
	short int databitnum; //ÿ����������λ������ʾÿ�������и�������������λ��
	char datasign[4];     //���ݱ�Ƿ���data��
	long int datalength;  //�������ݵĳ���(�ֽڣ�
}wavhead;

void rm_dc(short sample[],long int dataszize);
//����ֱ������
//============================================================================
//��ȡwav�ļ�
void waveread(char filename[],int *FS,long int *size,short **wavdata)
{
	wavehead wavhead;
	FILE *fp;
	if((fp=fopen(filename,"rb"))==NULL)
	{
		printf("cannot read wave file\n");
		exit(0);
	}
	int headnum;
	headnum=sizeof(wavhead);
	//WAVE�ļ�ͷ00H~28H.
	fread(&wavhead,headnum,1,fp);
	short int bitnum;
	long int datasize;
	//�ֱ�������ʣ��������ݵĳ��ȣ�ÿ����������λ����
	*FS=wavhead.samplerate;
	bitnum=wavhead.databitnum;
	datasize=wavhead.datalength;
	*size=datasize/sizeof(unsigned char);
	//�õ�������������ĸ�����ÿ��������short int��ʾ��
    //printf("The total wave data length:%d\n",datasize/sizeof(short int));
	//printf("The sample rate is:");
	//printf("%d\n",*FS);
	//int tt;
	//float total_t=float(datasize)/((*FS)*2);
	unsigned char *data=new unsigned char[datasize/sizeof(unsigned char)];
	//WAVE�ļ���ÿ������ֵ������һ������i��
	//��ȡ�õ�������;
	//16λPCM int 32768 
	long int offset=sizeof(wavehead);
	fseek(fp,offset,0);
	//Ҫ��λ������long������
	//short int samplenew=0;
	//unsigned char samplenew;
	//long int size_i=0;
	//while(fread(&samplenew,1,1,fp)==1)
	//{
	//	*data++=samplenew;
	//	//size_i++;
	//}
	if(fread(data,datasize,1,fp)!=1)
	{
		if(feof(fp)) exit(0);
		printf("filereaderror\n");
	}
	short int *datashort=new short int[datasize/sizeof(unsigned char)];
	for (long int i=0;i<*size;i++)
	{
		*(datashort+i)=*(data+i);
	}
	rm_dc(datashort,datasize);
	delete []data;
	//����ֱ������
	*wavdata=datashort;
	fclose(fp);
}

void WavHeadRead(char filename[],int *FS,long int *size)
{
	wavehead wavhead;
	FILE *fp;
	//char filename[]="E:\\SpeechNoise\\F07B522.wav";
	//char filename[]="E:\\noise\\white.wav";
	//char filename[20];
	//printf("Please input the wave filename:");
	//scanf("%s",filename);
	if((fp=fopen(filename,"rb"))==NULL)
	{
		printf("cannot read wavehead file\n");
		exit(0);
	}
	int headnum;
	headnum=sizeof(wavhead);
	//WAVE�ļ�ͷ00H~28H.
	fread(&wavhead,headnum,1,fp);
	short int bitnum;
	long int datasize;
	//�ֱ�������ʣ��������ݵĳ��ȣ�ÿ����������λ����
	*FS=wavhead.samplerate;
	bitnum=wavhead.databitnum;
	datasize=wavhead.datalength;
	*size=datasize/sizeof(short int);
	//�õ�������������ĸ�����ÿ��������short int��ʾ��
	printf("The total wave data length:%d\n",datasize/sizeof(short int));
	//printf("The sample rate is:");
	//printf("%d\n",*FS);
	//int tt;
	//float total_t=float(datasize)/((*FS)*2);

	fclose(fp);
}

//����8λPCM������£�
//8λPCM unsigned int 225 0 
//��һ��dat.Data = (dat.Data-128)/128;  [-1,1)
/* remove DC by removing the average of the whole utterance */
void rm_dc(short sample[],long int datasize)
{
  long i,total=0;
  long no_samples=datasize/sizeof(unsigned char);
  for(i=0;i<no_samples;i++)
     total+=sample[i];
  total/=no_samples;
  for(i=0;i<no_samples;i++)
     sample[i]-=(short)total;
}


long int wavSamleRate(char filename[])
{
	wavehead wavhead;
	FILE *fp;
	if((fp=fopen(filename,"rb"))==NULL)
	{
		printf("cannot read wave file\n");
		exit(0);
	}
	int headnum;
	headnum=sizeof(wavhead);
	//WAVE�ļ�ͷ00H~28H.
	fread(&wavhead,headnum,1,fp);
	//short int bitnum;
	long int datasize;
	//�ֱ�������ʣ��������ݵĳ��ȣ�ÿ����������λ����
	datasize=wavhead.datalength;
	fclose(fp);
	return datasize/sizeof(unsigned char);

}
