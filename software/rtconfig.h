/* RT-Thread config file */
#ifndef __RTTHREAD_CFG_H__
#define __RTTHREAD_CFG_H__

/* RT_NAME_MAX*/
#define RT_NAME_MAX	   8

/* RT_ALIGN_SIZE*/
#define RT_ALIGN_SIZE	4

/* PRIORITY_MAX */
#define RT_THREAD_PRIORITY_MAX	32

/* Tick per Second */
#define RT_TICK_PER_SECOND	1000

/* SECTION: RT_DEBUG */
/* Thread Debug */
#define RT_DEBUG
#define RT_USING_OVERFLOW_CHECK

/* Using Hook */
#define RT_USING_HOOK

#define IDLE_THREAD_STACK_SIZE     1024

/* Using Software Timer */
/* #define RT_USING_TIMER_SOFT */
#define RT_TIMER_THREAD_PRIO		4
#define RT_TIMER_THREAD_STACK_SIZE	512

/* SECTION: IPC */
/* Using Semaphore*/
#define RT_USING_SEMAPHORE

/* Using Mutex */
#define RT_USING_MUTEX

/* Using Event */
#define RT_USING_EVENT

/* Using MailBox */
#define RT_USING_MAILBOX

/* Using Message Queue */
#define RT_USING_MESSAGEQUEUE

/* SECTION: Memory Management */
/* Using Memory Pool Management*/
#define RT_USING_MEMPOOL

/* Using Dynamic Heap Management */
#define RT_USING_HEAP

/* Using Small MM */
#define RT_USING_SMALL_MEM

/* SECTION: Device System */
/* Using Device System */
#define RT_USING_DEVICE
#define RT_USING_DEVICE_IPC
/* Using serial framework */
#define RT_USING_SERIAL

#define RT_USING_UART1
//#define RT_USING_UART2
#define RT_USING_UART3

/* Using I2C framework */
#define RT_USING_I2C
//#define RT_I2C_DEBUG
#define RT_USING_I2C_BITOPS
//#define RT_I2C_BIT_DEBUG

/* Using GPIO pin framework */
#define RT_USING_PIN

/* Using Hardware Timer framework */
//#define RT_USING_HWTIMER

/* SECTION: Console options */
#define RT_USING_CONSOLE
/* the buffer size of console*/
#define RT_CONSOLEBUF_SIZE	128

/* SECTION: finsh, a C-Express shell */
#define RT_USING_FINSH
/* Using symbol table */
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
/* Using msh style */
#define FINSH_USING_MSH
#define FINSH_USING_MSH_ONLY

#define RT_USING_COMPONENTS_INIT

/* SECTION: device filesystem */
/* Using Device file system */
/* #define RT_USING_DFS */
/* the max number of mounted filesystem */
#define DFS_FILESYSTEMS_MAX			2
/* the max number of opened files 		*/
#define DFS_FD_MAX					4

/* Using ELM FATFS */
//#define RT_USING_DFS_ELMFAT
#define RT_DFS_ELM_WORD_ACCESS
/* Reentrancy (thread safe) of the FatFs module.  */
#define RT_DFS_ELM_REENTRANT
/* Number of volumes (logical drives) to be used. */
#define RT_DFS_ELM_DRIVES			2
/* #define RT_DFS_ELM_USE_LFN			1 */
#define RT_DFS_ELM_MAX_LFN			255
/* Maximum sector size to be handled. */
#define RT_DFS_ELM_MAX_SECTOR_SIZE  512

/* Using ROM file system */
// #define RT_USING_DFS_ROMFS

/* SECTION: lwip, a lighwight TCP/IP protocol stack */
/* #define RT_USING_LWIP */
/* LwIP uses RT-Thread Memory Management */
#define RT_LWIP_USING_RT_MEM
/* Enable ICMP protocol*/
#define RT_LWIP_ICMP
/* Enable UDP protocol*/
#define RT_LWIP_UDP
/* Enable TCP protocol*/
#define RT_LWIP_TCP
/* Enable DNS */
#define RT_LWIP_DNS

/* the number of simulatenously active TCP connections*/
#define RT_LWIP_TCP_PCB_NUM	5

/* ip address of target*/
#define RT_LWIP_IPADDR0	192
#define RT_LWIP_IPADDR1	168
#define RT_LWIP_IPADDR2	1
#define RT_LWIP_IPADDR3	201

/* gateway address of target*/
#define RT_LWIP_GWADDR0	192
#define RT_LWIP_GWADDR1	168
#define RT_LWIP_GWADDR2	1
#define RT_LWIP_GWADDR3	1

/* mask address of target*/
#define RT_LWIP_MSKADDR0	255
#define RT_LWIP_MSKADDR1	255
#define RT_LWIP_MSKADDR2	255
#define RT_LWIP_MSKADDR3	0

/* tcp thread options */
#define RT_LWIP_TCPTHREAD_PRIORITY		12
#define RT_LWIP_TCPTHREAD_MBOX_SIZE		4
#define RT_LWIP_TCPTHREAD_STACKSIZE		1024

/* ethernet if thread options */
#define RT_LWIP_ETHTHREAD_PRIORITY		15
#define RT_LWIP_ETHTHREAD_MBOX_SIZE		4
#define RT_LWIP_ETHTHREAD_STACKSIZE		512

/* TCP sender buffer space */
#define RT_LWIP_TCP_SND_BUF	8192
/* TCP receive window. */
#define RT_LWIP_TCP_WND		8192

/* RT_GDB_STUB */
//#define RT_USING_GDB

/* SECTION: Graphic UI Engine */
#define RT_USING_GUIENGINE
// <section name="RT_USING_RTGUI" description="RT-Thread/GUI" default="true" >
#define RT_USING_RTGUI
// <integer name="RTGUI_NAME_MAX" description="the name size of RT-Thread/GUI widget/objects" default="12" />
#define RTGUI_NAME_MAX	12
// <bool name="RTGUI_USING_SMALL_SIZE" description="use small size in RT-Thread/GUI" default="true" />
#define RTGUI_USING_SMALL_SIZE
// <bool name="RTGUI_USING_FONT16" description="support 16 weight font" default="true" />
#define RTGUI_USING_FONT16
// <bool name="RTGUI_USING_FONT12" description="support 12 weight font" default="true" />
// #define RTGUI_USING_FONT12
// <bool name="RTGUI_USING_FONTHZ" description="support Chinese font" default="true" />
//#define RTGUI_USING_FONTHZ
// <integer name="RTGUI_DEFAULT_FONT_SIZE" description="default font size in RT-Thread/GUI" default="16" />
#define RTGUI_DEFAULT_FONT_SIZE	16
// <bool name="RTGUI_USING_DFS_FILERW" description="use RT-Thread/DFS as file interface" default="true" />
#define RTGUI_USING_DFS_FILERW
// <bool name="RTGUI_USING_HZ_BMP" description="use Chinese font bitmap engine" default="true" />
//#define RTGUI_USING_HZ_BMP
// <bool name="RTGUI_IMAGE_XPM" description="Using xpm image in RTGUI" default="true" />
#define RTGUI_IMAGE_XPM
// <bool name="RTGUI_IMAGE_JPEG" description="Using jpeg image in RTGUI" default="true" />
#define RTGUI_IMAGE_JPEG
// <bool name="RTGUI_IMAGE_PNG" description="Using PNG image in RTGUI" default="true" />
#define RTGUI_IMAGE_PNG
// <bool name="RTGUI_IMAGE_BMP" description="Using PNG image in RTGUI" default="true" />
#define RTGUI_IMAGE_BMP
// <bool name="RTGUI_USING_HZ_FILE" description="use font file as Chinese font" default="false" />
//#define RTGUI_USING_HZ_FILE
// <bool name="RTGUI_USING_MOUSE_CURSOR" description="use mouse cursor" default="false" />
//#define RTGUI_USING_MOUSE_CURSOR
// </section>

//#define RT_USING_GY53
//#define RT_USING_FILTER
#define RT_USING_DMP
#define RT_USING_PID
#define RT_USING_ANOP

#define RT_USING_COLORTRACE

#endif
