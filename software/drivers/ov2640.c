#include "board.h"
#include "ov2640.h"
#include "ov2640cfg.h"
#include "sccb.h"

#define OV2640_RESET PDout(6) //RST低电平有效
#define OV2640_PWDN  PEout(5) //PWDN高电平有效

static void OV2640_Reset(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE, ENABLE);//使能GPIOD,E时钟
    //OV_RESET OV_PWDN 设置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;//25MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;//GPIOD6推挽输出
    GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;//GPIOE5推挽输出
    GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化
    
	OV2640_RESET = 0;//复位OV2640
	OV2640_PWDN =  0;//POWER ON
	rt_thread_delay(3);//从上电到结束复位至少要延时3ms
	OV2640_RESET = 1;//结束复位
	rt_thread_delay(3);//从复位结束到SCCB初始化至少要延时3ms
}

//OV2640初始化
void OV2640_Init(void)
{
    u8 i;
    u16 OV2640_ID;//记录制造商ID或产品ID
    
    OV2640_Reset();//硬件复位
	SCCB_Init();//SCCB端口配置
	SCCB_WReg(OV2640_RA_DLMT,OV2640_SENSOR);//操作SENSOR寄存器
	//读Product_ID
	OV2640_ID  = SCCB_RReg(OV2640_SENSOR_PIDH);//读产品ID高8位
	OV2640_ID<<=8;//左移8位
	OV2640_ID |= SCCB_RReg(OV2640_SENSOR_PIDL);//读产品ID低8位
	rt_kprintf("Product_ID=0x%x\r\n",OV2640_ID);//串口打印Product_ID
	//读Manufacturer_ID
	OV2640_ID  = SCCB_RReg(OV2640_SENSOR_MIDH);//读制造商ID高8位
	OV2640_ID<<=8;//左移8位
	OV2640_ID |= SCCB_RReg(OV2640_SENSOR_MIDL);//读制造商ID低8位
	rt_kprintf("Manufacturer_ID=0x%x\r\n",OV2640_ID);//串口打印Manufacturer_ID
	
	//初始化 OV2640,采用SVGA分辨率(800*600)
	for(i=0;i<sizeof(OV2640_SVGA)/2;i++)
	{
		SCCB_WReg(OV2640_SVGA[i][0],OV2640_SVGA[i][1]);
	}
}

//OV2640切换为JPEG模式
void OV2640_JPEG_Mode(void) 
{
	u8 i=0;
	//设置:YUV422格式
	for(i=0;i<(sizeof(OV2640_YUV422)/2);i++)
	{
		SCCB_WReg(OV2640_YUV422[i][0],OV2640_YUV422[i][1]); 
	} 
	
	//设置:输出JPEG数据
	for(i=0;i<(sizeof(OV2640_JPEG)/2);i++)
	{
		SCCB_WReg(OV2640_JPEG[i][0],OV2640_JPEG[i][1]);  
	}  
}

//OV2640切换为RGB565模式
void OV2640_RGB565_Mode(void) 
{
	u8 i=0;
	//设置:RGB565输出
	for(i=0;i<(sizeof(OV2640_RGB565)/2);i++)
	{
		SCCB_WReg(OV2640_RGB565[i][0],OV2640_RGB565[i][1]); 
	} 
} 

//OV2640自动曝光等级设置
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

//白平衡设置
//0:自动
//1:太阳sunny
//2,阴天cloudy
//3,办公室office
//4,家里home
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

//色度设置
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

//亮度设置
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

//对比度设置
//0:-2
//1:-1
//2,0
//3,+1
//4,+2
void OV2640_Contrast(u8 contrast)
{
	u8 reg7d0val=0X20;//默认为普通模式
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

//特效设置
//0:普通模式    
//1,负片
//2,黑白   
//3,偏红色
//4,偏绿色
//5,偏蓝色
//6,复古	    
void OV2640_Special_Effects(u8 eft)
{
	u8 reg7d0val=0X00;//默认为普通模式
	u8 reg7d1val=0X80;
	u8 reg7d2val=0X80; 
	switch(eft)
	{
		case 1://负片
			reg7d0val=0X40; 
			break;	
		case 2://黑白
			reg7d0val=0X18; 
			break;	 
		case 3://偏红色
			reg7d0val=0X18; 
			reg7d1val=0X40;
			reg7d2val=0XC0; 
			break;	
		case 4://偏绿色
			reg7d0val=0X18; 
			reg7d1val=0X40;
			reg7d2val=0X40; 
			break;	
		case 5://偏蓝色
			reg7d0val=0X18; 
			reg7d1val=0XA0;
			reg7d2val=0X40; 
			break;	
		case 6://复古
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

//彩条测试
//sw:0,关闭彩条
//   1,开启彩条(注意OV2640的彩条是叠加在图像上面的)
void OV2640_Color_Bar(u8 sw)
{
	u8 reg;
	SCCB_WReg(0XFF,0X01);
	reg=SCCB_RReg(0X12);
	reg&=~(1<<1);
	if(sw)reg|=1<<1; 
	SCCB_WReg(0X12,reg);
}

//设置图像输出窗口 
//sx,sy,起始地址
//width,height:宽度(对应:horizontal)和高度(对应:vertical)
void OV2640_Window_Set(u16 sx,u16 sy,u16 width,u16 height)
{
	u16 endx;
	u16 endy;
	u8 temp; 
	endx=sx+width/2;	//V*2
 	endy=sy+height/2;
	
	SCCB_WReg(0XFF,0X01);			
	temp=SCCB_RReg(0X03);				//读取Vref之前的值
	temp&=0XF0;
	temp|=((endy&0X03)<<2)|(sy&0X03);
	SCCB_WReg(0X03,temp);				//设置Vref的start和end的最低2位
	SCCB_WReg(0X19,sy>>2);			//设置Vref的start高8位
	SCCB_WReg(0X1A,endy>>2);			//设置Vref的end的高8位
	
	temp=SCCB_RReg(0X32);				//读取Href之前的值
	temp&=0XC0;
	temp|=((endx&0X07)<<3)|(sx&0X07);
	SCCB_WReg(0X32,temp);				//设置Href的start和end的最低3位
	SCCB_WReg(0X17,sx>>3);			//设置Href的start高8位
	SCCB_WReg(0X18,endx>>3);			//设置Href的end的高8位
}

//设置图像输出大小
//OV2640输出图像的大小(分辨率),完全由改函数确定
//width,height:宽度(对应:horizontal)和高度(对应:vertical),width和height必须是4的倍数
//返回值:0,设置成功
//    其他,设置失败
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
	SCCB_WReg(0X5A,outw&0XFF);		//设置OUTW的低八位
	SCCB_WReg(0X5B,outh&0XFF);		//设置OUTH的低八位
	temp=(outw>>8)&0X03;
	temp|=(outh>>6)&0X04;
	SCCB_WReg(0X5C,temp);				//设置OUTH/OUTW的高位 
	SCCB_WReg(0XE0,0X00);	
	return 0;
}

//设置图像开窗大小
//由:OV2640_ImageSize_Set确定传感器输出分辨率从大小.
//该函数则在这个范围上面进行开窗,用于OV2640_OutSize_Set的输出
//注意:本函数的宽度和高度,必须大于等于OV2640_OutSize_Set函数的宽度和高度
//     OV2640_OutSize_Set设置的宽度和高度,根据本函数设置的宽度和高度,由DSP
//     自动计算缩放比例,输出给外部设备.
//width,height:宽度(对应:horizontal)和高度(对应:vertical),width和height必须是4的倍数
//返回值:0,设置成功
//    其他,设置失败
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
	SCCB_WReg(0X51,hsize&0XFF);		//设置H_SIZE的低八位
	SCCB_WReg(0X52,vsize&0XFF);		//设置V_SIZE的低八位
	SCCB_WReg(0X53,offx&0XFF);		//设置offx的低八位
	SCCB_WReg(0X54,offy&0XFF);		//设置offy的低八位
	temp=(vsize>>1)&0X80;
	temp|=(offy>>4)&0X70;
	temp|=(hsize>>5)&0X08;
	temp|=(offx>>8)&0X07; 
	SCCB_WReg(0X55,temp);				//设置H_SIZE/V_SIZE/OFFX,OFFY的高位
	SCCB_WReg(0X57,(hsize>>2)&0X80);	//设置H_SIZE/V_SIZE/OFFX,OFFY的高位
	SCCB_WReg(0XE0,0X00);	
	return 0;
} 

//该函数设置图像尺寸大小,也就是所选格式的输出分辨率
//UXGA:1600*1200,SVGA:800*600,CIF:352*288
//width,height:图像宽度和图像高度
//返回值:0,设置成功
//    其他,设置失败
u8 OV2640_ImageSize_Set(u16 width,u16 height)
{ 
	u8 temp; 
	SCCB_WReg(0XFF,0X00);			
	SCCB_WReg(0XE0,0X04);			
	SCCB_WReg(0XC0,(width)>>3&0XFF);		//设置HSIZE的10:3位
	SCCB_WReg(0XC1,(height)>>3&0XFF);		//设置VSIZE的10:3位
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
