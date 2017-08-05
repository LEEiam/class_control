#include "board.h"
#include "ov2640.h"
#include "ov2640cfg.h"
#include "sccb.h"

#define OV2640_RESET PDout(6) //RST�͵�ƽ��Ч
#define OV2640_PWDN  PEout(5) //PWDN�ߵ�ƽ��Ч

static void OV2640_Reset(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE, ENABLE);//ʹ��GPIOD,Eʱ��
    //OV_RESET OV_PWDN ����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //���ģʽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;//25MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;//GPIOD6�������
    GPIO_Init(GPIOD, &GPIO_InitStructure);//��ʼ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;//GPIOE5�������
    GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��
    
	OV2640_RESET = 0;//��λOV2640
	OV2640_PWDN =  0;//POWER ON
	rt_thread_delay(3);//���ϵ絽������λ����Ҫ��ʱ3ms
	OV2640_RESET = 1;//������λ
	rt_thread_delay(3);//�Ӹ�λ������SCCB��ʼ������Ҫ��ʱ3ms
}

//OV2640��ʼ��
void OV2640_Init(void)
{
    u8 i;
    u16 OV2640_ID;//��¼������ID���ƷID
    
    OV2640_Reset();//Ӳ����λ
	SCCB_Init();//SCCB�˿�����
	SCCB_WReg(OV2640_RA_DLMT,OV2640_SENSOR);//����SENSOR�Ĵ���
	//��Product_ID
	OV2640_ID  = SCCB_RReg(OV2640_SENSOR_PIDH);//����ƷID��8λ
	OV2640_ID<<=8;//����8λ
	OV2640_ID |= SCCB_RReg(OV2640_SENSOR_PIDL);//����ƷID��8λ
	rt_kprintf("Product_ID=0x%x\r\n",OV2640_ID);//���ڴ�ӡProduct_ID
	//��Manufacturer_ID
	OV2640_ID  = SCCB_RReg(OV2640_SENSOR_MIDH);//��������ID��8λ
	OV2640_ID<<=8;//����8λ
	OV2640_ID |= SCCB_RReg(OV2640_SENSOR_MIDL);//��������ID��8λ
	rt_kprintf("Manufacturer_ID=0x%x\r\n",OV2640_ID);//���ڴ�ӡManufacturer_ID
	
	//��ʼ�� OV2640,����SVGA�ֱ���(800*600)
	for(i=0;i<sizeof(OV2640_SVGA)/2;i++)
	{
		SCCB_WReg(OV2640_SVGA[i][0],OV2640_SVGA[i][1]);
	}
}

//OV2640�л�ΪJPEGģʽ
void OV2640_JPEG_Mode(void) 
{
	u8 i=0;
	//����:YUV422��ʽ
	for(i=0;i<(sizeof(OV2640_YUV422)/2);i++)
	{
		SCCB_WReg(OV2640_YUV422[i][0],OV2640_YUV422[i][1]); 
	} 
	
	//����:���JPEG����
	for(i=0;i<(sizeof(OV2640_JPEG)/2);i++)
	{
		SCCB_WReg(OV2640_JPEG[i][0],OV2640_JPEG[i][1]);  
	}  
}

//OV2640�л�ΪRGB565ģʽ
void OV2640_RGB565_Mode(void) 
{
	u8 i=0;
	//����:RGB565���
	for(i=0;i<(sizeof(OV2640_RGB565)/2);i++)
	{
		SCCB_WReg(OV2640_RGB565[i][0],OV2640_RGB565[i][1]); 
	} 
} 

//OV2640�Զ��ع�ȼ�����
//level:0~4
void OV2640_Auto_Exposure(u8 level)
{  
	u8 i;
	u8 *p=(u8*)OV2640_AUTOEXPOSURE_LEVEL[level];
	for(i=0;i<4;i++)
	{ 
		SCCB_WReg(p[i*2],p[i*2+1]); 
	} 
}  

//��ƽ������
//0:�Զ�
//1:̫��sunny
//2,����cloudy
//3,�칫��office
//4,����home
void OV2640_Light_Mode(u8 mode)
{
	u8 regccval=0X5E;//Sunny 
	u8 regcdval=0X41;
	u8 regceval=0X54;
	switch(mode)
	{ 
		case 0://auto 
			SCCB_WReg(0XFF,0X00);	 
			SCCB_WReg(0XC7,0X10);//AWB ON 
			return;  	
		case 2://cloudy
			regccval=0X65;
			regcdval=0X41;
			regceval=0X4F;
			break;	
		case 3://office
			regccval=0X52;
			regcdval=0X41;
			regceval=0X66;
			break;	
		case 4://home
			regccval=0X42;
			regcdval=0X3F;
			regceval=0X71;
			break;	
	}
	SCCB_WReg(0XFF,0X00);	 
	SCCB_WReg(0XC7,0X40);	//AWB OFF 
	SCCB_WReg(0XCC,regccval); 
	SCCB_WReg(0XCD,regcdval); 
	SCCB_WReg(0XCE,regceval);  
}

//ɫ������
//0:-2
//1:-1
//2,0
//3,+1
//4,+2
void OV2640_Color_Saturation(u8 sat)
{ 
	u8 reg7dval=((sat+2)<<4)|0X08;
	SCCB_WReg(0XFF,0X00);		
	SCCB_WReg(0X7C,0X00);		
	SCCB_WReg(0X7D,0X02);				
	SCCB_WReg(0X7C,0X03);			
	SCCB_WReg(0X7D,reg7dval);			
	SCCB_WReg(0X7D,reg7dval); 		
}

//��������
//0:(0X00)-2
//1:(0X10)-1
//2,(0X20) 0
//3,(0X30)+1
//4,(0X40)+2
void OV2640_Brightness(u8 bright)
{
  SCCB_WReg(0xff, 0x00);
  SCCB_WReg(0x7c, 0x00);
  SCCB_WReg(0x7d, 0x04);
  SCCB_WReg(0x7c, 0x09);
  SCCB_WReg(0x7d, bright<<4); 
  SCCB_WReg(0x7d, 0x00); 
}

//�Աȶ�����
//0:-2
//1:-1
//2,0
//3,+1
//4,+2
void OV2640_Contrast(u8 contrast)
{
	u8 reg7d0val=0X20;//Ĭ��Ϊ��ͨģʽ
	u8 reg7d1val=0X20;
  	switch(contrast)
	{
		case 0://-2
			reg7d0val=0X18;	 	 
			reg7d1val=0X34;	 	 
			break;	
		case 1://-1
			reg7d0val=0X1C;	 	 
			reg7d1val=0X2A;	 	 
			break;	
		case 3://1
			reg7d0val=0X24;	 	 
			reg7d1val=0X16;	 	 
			break;	
		case 4://2
			reg7d0val=0X28;	 	 
			reg7d1val=0X0C;	 	 
			break;	
	}
	SCCB_WReg(0xff,0x00);
	SCCB_WReg(0x7c,0x00);
	SCCB_WReg(0x7d,0x04);
	SCCB_WReg(0x7c,0x07);
	SCCB_WReg(0x7d,0x20);
	SCCB_WReg(0x7d,reg7d0val);
	SCCB_WReg(0x7d,reg7d1val);
	SCCB_WReg(0x7d,0x06);
}

//��Ч����
//0:��ͨģʽ    
//1,��Ƭ
//2,�ڰ�   
//3,ƫ��ɫ
//4,ƫ��ɫ
//5,ƫ��ɫ
//6,����	    
void OV2640_Special_Effects(u8 eft)
{
	u8 reg7d0val=0X00;//Ĭ��Ϊ��ͨģʽ
	u8 reg7d1val=0X80;
	u8 reg7d2val=0X80; 
	switch(eft)
	{
		case 1://��Ƭ
			reg7d0val=0X40; 
			break;	
		case 2://�ڰ�
			reg7d0val=0X18; 
			break;	 
		case 3://ƫ��ɫ
			reg7d0val=0X18; 
			reg7d1val=0X40;
			reg7d2val=0XC0; 
			break;	
		case 4://ƫ��ɫ
			reg7d0val=0X18; 
			reg7d1val=0X40;
			reg7d2val=0X40; 
			break;	
		case 5://ƫ��ɫ
			reg7d0val=0X18; 
			reg7d1val=0XA0;
			reg7d2val=0X40; 
			break;	
		case 6://����
			reg7d0val=0X18; 
			reg7d1val=0X40;
			reg7d2val=0XA6; 
			break;	 
	}
	SCCB_WReg(0xff,0x00);
	SCCB_WReg(0x7c,0x00);
	SCCB_WReg(0x7d,reg7d0val);
	SCCB_WReg(0x7c,0x05);
	SCCB_WReg(0x7d,reg7d1val);
	SCCB_WReg(0x7d,reg7d2val); 
}

//��������
//sw:0,�رղ���
//   1,��������(ע��OV2640�Ĳ����ǵ�����ͼ�������)
void OV2640_Color_Bar(u8 sw)
{
	u8 reg;
	SCCB_WReg(0XFF,0X01);
	reg=SCCB_RReg(0X12);
	reg&=~(1<<1);
	if(sw)reg|=1<<1; 
	SCCB_WReg(0X12,reg);
}

//����ͼ��������� 
//sx,sy,��ʼ��ַ
//width,height:���(��Ӧ:horizontal)�͸߶�(��Ӧ:vertical)
void OV2640_Window_Set(u16 sx,u16 sy,u16 width,u16 height)
{
	u16 endx;
	u16 endy;
	u8 temp; 
	endx=sx+width/2;	//V*2
 	endy=sy+height/2;
	
	SCCB_WReg(0XFF,0X01);			
	temp=SCCB_RReg(0X03);				//��ȡVref֮ǰ��ֵ
	temp&=0XF0;
	temp|=((endy&0X03)<<2)|(sy&0X03);
	SCCB_WReg(0X03,temp);				//����Vref��start��end�����2λ
	SCCB_WReg(0X19,sy>>2);			//����Vref��start��8λ
	SCCB_WReg(0X1A,endy>>2);			//����Vref��end�ĸ�8λ
	
	temp=SCCB_RReg(0X32);				//��ȡHref֮ǰ��ֵ
	temp&=0XC0;
	temp|=((endx&0X07)<<3)|(sx&0X07);
	SCCB_WReg(0X32,temp);				//����Href��start��end�����3λ
	SCCB_WReg(0X17,sx>>3);			//����Href��start��8λ
	SCCB_WReg(0X18,endx>>3);			//����Href��end�ĸ�8λ
}

//����ͼ�������С
//OV2640���ͼ��Ĵ�С(�ֱ���),��ȫ�ɸĺ���ȷ��
//width,height:���(��Ӧ:horizontal)�͸߶�(��Ӧ:vertical),width��height������4�ı���
//����ֵ:0,���óɹ�
//    ����,����ʧ��
u8 OV2640_OutSize_Set(u16 width,u16 height)
{
	u16 outh;
	u16 outw;
	u8 temp; 
	if(width%4)return 1;
	if(height%4)return 2;
	outw=width/4;
	outh=height/4; 
	SCCB_WReg(0XFF,0X00);	
	SCCB_WReg(0XE0,0X04);			
	SCCB_WReg(0X5A,outw&0XFF);		//����OUTW�ĵͰ�λ
	SCCB_WReg(0X5B,outh&0XFF);		//����OUTH�ĵͰ�λ
	temp=(outw>>8)&0X03;
	temp|=(outh>>6)&0X04;
	SCCB_WReg(0X5C,temp);				//����OUTH/OUTW�ĸ�λ 
	SCCB_WReg(0XE0,0X00);	
	return 0;
}

//����ͼ�񿪴���С
//��:OV2640_ImageSize_Setȷ������������ֱ��ʴӴ�С.
//�ú������������Χ������п���,����OV2640_OutSize_Set�����
//ע��:�������Ŀ�Ⱥ͸߶�,������ڵ���OV2640_OutSize_Set�����Ŀ�Ⱥ͸߶�
//     OV2640_OutSize_Set���õĿ�Ⱥ͸߶�,���ݱ��������õĿ�Ⱥ͸߶�,��DSP
//     �Զ��������ű���,������ⲿ�豸.
//width,height:���(��Ӧ:horizontal)�͸߶�(��Ӧ:vertical),width��height������4�ı���
//����ֵ:0,���óɹ�
//    ����,����ʧ��
u8 OV2640_ImageWin_Set(u16 offx,u16 offy,u16 width,u16 height)
{
	u16 hsize;
	u16 vsize;
	u8 temp; 
	if(width%4)return 1;
	if(height%4)return 2;
	hsize=width/4;
	vsize=height/4;
	SCCB_WReg(0XFF,0X00);	
	SCCB_WReg(0XE0,0X04);					
	SCCB_WReg(0X51,hsize&0XFF);		//����H_SIZE�ĵͰ�λ
	SCCB_WReg(0X52,vsize&0XFF);		//����V_SIZE�ĵͰ�λ
	SCCB_WReg(0X53,offx&0XFF);		//����offx�ĵͰ�λ
	SCCB_WReg(0X54,offy&0XFF);		//����offy�ĵͰ�λ
	temp=(vsize>>1)&0X80;
	temp|=(offy>>4)&0X70;
	temp|=(hsize>>5)&0X08;
	temp|=(offx>>8)&0X07; 
	SCCB_WReg(0X55,temp);				//����H_SIZE/V_SIZE/OFFX,OFFY�ĸ�λ
	SCCB_WReg(0X57,(hsize>>2)&0X80);	//����H_SIZE/V_SIZE/OFFX,OFFY�ĸ�λ
	SCCB_WReg(0XE0,0X00);	
	return 0;
} 

//�ú�������ͼ��ߴ��С,Ҳ������ѡ��ʽ������ֱ���
//UXGA:1600*1200,SVGA:800*600,CIF:352*288
//width,height:ͼ���Ⱥ�ͼ��߶�
//����ֵ:0,���óɹ�
//    ����,����ʧ��
u8 OV2640_ImageSize_Set(u16 width,u16 height)
{ 
	u8 temp; 
	SCCB_WReg(0XFF,0X00);			
	SCCB_WReg(0XE0,0X04);			
	SCCB_WReg(0XC0,(width)>>3&0XFF);		//����HSIZE��10:3λ
	SCCB_WReg(0XC1,(height)>>3&0XFF);		//����VSIZE��10:3λ
	temp=(width&0X07)<<3;
	temp|=height&0X07;
	temp|=(width>>4)&0X80; 
	SCCB_WReg(0X8C,temp);	
	SCCB_WReg(0XE0,0X00);				 
	return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>

void cmd_ov2640(int argc, char *argv[])
{
    if (argc == 2)
    {
        if (rt_strcmp(argv[1], "jpeg") == 0)
            OV2640_JPEG_Mode();
        else if (rt_strcmp(argv[1], "rgb565") == 0)
            OV2640_RGB565_Mode();
    }
    else if (argc == 3)
    {
        if (rt_strcmp(argv[1], "expose") == 0)
            OV2640_Auto_Exposure(atoi(argv[2]));
        else if (rt_strcmp(argv[1], "colors") == 0)
            OV2640_Color_Saturation(atoi(argv[2]));
        else if (rt_strcmp(argv[1], "brightness") == 0)
            OV2640_Brightness(atoi(argv[2]));
        else if (rt_strcmp(argv[1], "contrast") == 0)
            OV2640_Contrast(atoi(argv[2]));
        else if (rt_strcmp(argv[1], "special") == 0)
            OV2640_Special_Effects(atoi(argv[2]));
        else if (rt_strcmp(argv[1], "light") == 0)
            OV2640_Light_Mode(atoi(argv[2]));
    }
    else
    {
        rt_kprintf("Usage: ov2640 jpeg\n");
        rt_kprintf("              rgb565\n");
        rt_kprintf("              expose [level: 0~4]\n");
        rt_kprintf("              colors [level: 0~4]\n");
        rt_kprintf("              brightness [level: 0~4]\n");
        rt_kprintf("              contrast [level: 0~4]\n");
        rt_kprintf("              special [effects: 0~6]\n");
        rt_kprintf("              light [mode: 0~4]\n");
    }
}
MSH_CMD_EXPORT_ALIAS(cmd_ov2640, ov2640, camera ov2640 config);
#endif
