#ifndef _COLOR_TRACE_H_
#define _COLOR_TRACE_H_

#define IMG_X 0                 //图片x坐标
#define IMG_Y 0                 //图片y坐标
#define IMG_W 480               //图片宽度
#define IMG_H 320               //图片高度

#define ALLOW_FAIL_PER 3        //容错率
#define ITERATER_NUM   8        //迭代次数

typedef struct                  //判定为目标的条件
{
	unsigned char H_MIN;        //目标最小色度
	unsigned char H_MAX;        //目标最大色度
	
	unsigned char S_MIN;        //目标最小饱和度
	unsigned char S_MAX;        //目标最大饱和度
	
	unsigned char L_MIN;        //目标最小亮度
	unsigned char L_MAX;        //目标最大亮度
	
	unsigned int WIDTH_MIN;     //目标最小宽度
	unsigned int HEIGHT_MIN;    //目标最小高度
	
	unsigned int WIDTH_MAX;		//目标最大宽度
	unsigned int HEIGHT_MAX;	//目标最大高度
}TARGET_CONDITION;

typedef struct				    //结果
{
	unsigned int x;			    //目标x坐标
	unsigned int y;			    //目标y坐标
	unsigned int w;			    //目标宽度
	unsigned int h;			    //目标高度
}RESULT;

extern RESULT result;
extern TARGET_CONDITION condition;

int Trace(const TARGET_CONDITION* condition, RESULT* result_final);

#endif  /* _COLOR_TRACE_H_ */
