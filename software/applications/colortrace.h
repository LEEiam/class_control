#ifndef _COLOR_TRACE_H_
#define _COLOR_TRACE_H_

#define IMG_X 0                 //ͼƬx����
#define IMG_Y 0                 //ͼƬy����
#define IMG_W 480               //ͼƬ���
#define IMG_H 320               //ͼƬ�߶�

#define ALLOW_FAIL_PER 3        //�ݴ���
#define ITERATER_NUM   8        //��������

typedef struct                  //�ж�ΪĿ�������
{
	unsigned char H_MIN;        //Ŀ����Сɫ��
	unsigned char H_MAX;        //Ŀ�����ɫ��
	
	unsigned char S_MIN;        //Ŀ����С���Ͷ�
	unsigned char S_MAX;        //Ŀ����󱥺Ͷ�
	
	unsigned char L_MIN;        //Ŀ����С����
	unsigned char L_MAX;        //Ŀ���������
	
	unsigned int WIDTH_MIN;     //Ŀ����С���
	unsigned int HEIGHT_MIN;    //Ŀ����С�߶�
	
	unsigned int WIDTH_MAX;		//Ŀ�������
	unsigned int HEIGHT_MAX;	//Ŀ�����߶�
}TARGET_CONDITION;

typedef struct				    //���
{
	unsigned int x;			    //Ŀ��x����
	unsigned int y;			    //Ŀ��y����
	unsigned int w;			    //Ŀ����
	unsigned int h;			    //Ŀ��߶�
}RESULT;

extern RESULT result;
extern TARGET_CONDITION condition;

int Trace(const TARGET_CONDITION* condition, RESULT* result_final);

#endif  /* _COLOR_TRACE_H_ */
