/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, gary. All rights reserved.
 * @file            bsp_shm.c
 * @brief           IPC功能
 * @author          Gary
 * @date            2025/01-05
 * @remark
 *****************************************************************************/
#include <error.h>
#include "bsp_shm.h"

// SYSMODE_SHMEM_T g_RTDBStruct_t;
static DEVPOINT_RTDB_T *g_RTDB = NULL;
static webSign_t *g_WebSign = NULL;
static cfg_dsp_msg_mem_t *g_DspSign = NULL;
/*********************************************************************
 * @brief      管理共享内存的注册和连接
 * @param[in]   key      共享内存的键值
 * @param[in]   size     共享内存的大小
 * @param[in]   ipcType  IPC 类型，决定是否创建共享内存
 * @return      成功时返回共享内存的指针，失败时返回 NULL
 *********************************************************************/
void *cfgsign_shmen_set(key_t key, int size, int ipcType)
{
	void *shm = NULL; // 分配的共享内存的原始首地址

	// 创建共享内存
	int shmid, shmflg;
	if (kEE_SHM_CREAT == ipcType)
	{
		shmflg = 0666 | IPC_CREAT;
	}
	else if (kEE_SHM_READ == ipcType)
	{
		shmflg = 0666;
	}

	shmid = shmget(key, size, shmflg);

	KITPTF(LOG_DB_EN, DEBUG_EN, "cfgsign_shmen_set :: shmid = %d  \n", shmid);
	KITLOG(LOG_DB_EN, DEBUG_EN, "cfgsign_shmen_set :: shmid = %d  \n", shmid);

	if (shmid == -1)
	{
		KITPTF(LOG_DB_EN, DEBUG_EN, "cfgsign_shmen_set :: shmat failed\n", NULL);
		// exit(EXIT_FAILURE);
		return NULL;
	}
	// 将共享内存连接到当前进程的地址空间
	shm = shmat(shmid, 0, 0);
	if ((shm == (void *)-1) || (shm == NULL))
	{
		KITPTF(LOG_DB_EN, DEBUG_EN, "cfgsign_shmen_set :: shmat failed\n", NULL);
		// exit(EXIT_FAILURE);
		return NULL;
	}
	return shm;
}

/*********************************************************************
 * @brief      内存注册管理
 * @param[in]   void: 无
 * @return      null
 *********************************************************************/
void *devcodeShmenSet(key_t key, int maxnum, int ipcType)
{
	void *shm = NULL; // 分配的共享内存的原始首地址
	int shmflg;
	if (kEE_SHM_CREAT == ipcType)
	{
		shmflg = 0666 | IPC_CREAT;
	}
	else if (kEE_SHM_READ == ipcType)
	{
		shmflg = 0666;
	}
	// 创建共享内存
	int shmid;
	shmid = shmget(key, maxnum, shmflg);

	if (shmid == -1)
	{
		KITPTF(LOG_DB_EN, DEBUG_EN, "devcodeShmenSet :: shmat failed\n", NULL);
		// exit(EXIT_FAILURE);
		return NULL;
	}
	// 将共享内存连接到当前进程的地址空间 关联
	shm = shmat(shmid, 0, 0);
	if ((shm == (void *)-1) || (shm == NULL))
	{
		KITPTF(LOG_DB_EN, DEBUG_EN, "devcodeShmenSet :: shmat failed\n", NULL);
		// exit(EXIT_FAILURE);
		return NULL;
	}
	return shm;
}

/*********************************************************************
 * @brief       初始化设置信号通知的共享内存
 * @param[in]   void: 无
 * @return      ture-成功  0-fail
 *********************************************************************/
int initWebSignShm(shm_creat_type_e type)
{
	// 为设置信号通知分配空间
	g_WebSign = (webSign_t *)cfgsign_shmen_set(IPC_WEB_SHEM_KEY, sizeof(webSign_t), type);
	if (g_WebSign == NULL)
	{
		KITPTF(LOG_DB_EN, ERROR_EN, "init_cfgsign_shm error !!!!!", NULL);
		KITLOG(LOG_DB_EN, ERROR_EN, "init_cfgsign_shm error !!!!!", NULL);
		return 1;
	}
	else
	{
		memset(g_WebSign, 0, sizeof(webSign_t));
		return 0;
	}
}


/*********************************************************************
 * @brief       获取RTDB中的值
 * @param[in]   signType: 信号类型
 * @return      获取值
 *********************************************************************/
uint16_t readWebSignShm(cfg_web_sign_e signType)
{
	if (g_WebSign == NULL)
	{
		return 0;
	}

	uint16_t *item = (uint16_t *)g_WebSign;
	if (signType < kSign_End)
	{
		return item[signType];
	}
	else
	{
		return 0;
	}
}

/*********************************************************************
 * @brief       写入web与c写配置变化的通知组件
 * @param[in]   signType: 通知的地址
 * @param[in]   val: 对应的值
 * @return      none
 *********************************************************************/
void writeWebSignShm(cfg_web_sign_e signType, uint16_t val)
{
	if (g_WebSign == NULL)
	{
		return;
	}

	uint16_t *item = (uint16_t *)g_WebSign;
	if (signType < kSign_End)
	{
		item[signType] = val;
	}
	else
	{
		return;
	}
}

/*********************************************************************
 * @brief       初始化创建RTDB
 * @param[in]   client: 客户端信息针
 * @param[in]   arg: 所有测点属性的指针
 * @return      0-成功  1失败
 *********************************************************************/
int initRtdbShm(void *arg, shm_creat_type_e type)
{
	// CFG_DEVPOINT_PARA_T *devpoint_t = (CFG_DEVPOINT_PARA_T *)arg;
	int i = 0, j = 0, k = 0;
	key_t key = CON_KERNEL_KEY;
	int ssize = sizeof(DEVPOINT_RTDB_T);
	g_RTDB = (DEVPOINT_RTDB_T *)devcodeShmenSet(key, ssize, type);

	KITPTF(LOG_KIT_EN, DEBUG_EN, "创建总rtdb上传，key：%d", key);
	KITLOG(LOG_KIT_EN, DEBUG_EN, "创建总rtdb上传，key：%d", key);

	if (g_RTDB == NULL)
	{
		KITPTF(LOG_DB_EN, ERROR_EN, "devcodeShmenSet error !!!!!");
		KITLOG(LOG_DB_EN, ERROR_EN, "devcodeShmenSet error !!!!!");
		return SHM_ERROR;
	}

	memset(g_RTDB, 0, ssize);
	g_RTDB->num = kDev_Type_End;
	int csize = sizeof(devPointType_t) * kDev_Type_End;
	g_RTDB->sDCodePoint = (devPointType_t *)devcodeShmenSet(++key, csize, type);
	memset(g_RTDB->sDCodePoint, 0, csize);

	for (i = 0; i < kDev_Type_End; i++)
	{
		g_RTDB->sDCodePoint[i].id = i;

		g_RTDB->sDCodePoint[i].num = (gStDevTypeNum[i] == 0 ? 1 : gStDevTypeNum[i]);

		int dsize = sizeof(devPoint_t) * gStDevTypePointNum[i];
		g_RTDB->sDCodePoint[i].sDevPoint = (devPoint_t *)devcodeShmenSet(++key, dsize, type);

		KITPTF(LOG_KIT_EN, DEBUG_EN, "设备类型:%d ,长度：%d key:%d", i, dsize, key);
		KITLOG(LOG_KIT_EN, DEBUG_EN, "设备类型:%d ,长度：%d key:%d", i, dsize, key);
		memset(g_RTDB->sDCodePoint[i].sDevPoint, 0, dsize);

		for (j = 0; j < g_RTDB->sDCodePoint[i].num; j++)
		{
			KITPTF(LOG_KIT_EN, DEBUG_EN, "设备类型：%d,设备号：%d", i, j);
			KITLOG(LOG_KIT_EN, DEBUG_EN, "设备类型：%d,设备号：%d", i, j);

			g_RTDB->sDCodePoint[i].sDevPoint[j].id = j;
			// 后面补充
			// strcpy(g_RTDB->sDCodePoint[i].sDevPoint[j].name , devpoint_t->sDCodePoint[i].sDevPoint[j].name);
			// strcpy(g_RTDB->sDCodePoint[i].sDevPoint[j].enname, devpoint_t->sDCodePoint[i].sDevPoint[j].enname);
			g_RTDB->sDCodePoint[i].sDevPoint[j].num = gStDevTypePointNum[i];

			int psize = sizeof(point_t) * g_RTDB->sDCodePoint[i].sDevPoint[j].num;
			g_RTDB->sDCodePoint[i].sDevPoint[j].sPoint = (point_t *)devcodeShmenSet(++key, psize, type);

			KITPTF(LOG_KIT_EN, DEBUG_EN, "设备类型：%d,设备号：%d,长度：%d,key:%d", i, j, psize, key);
			KITLOG(LOG_KIT_EN, DEBUG_EN, "设备类型：%d,设备号：%d,长度：%d,key:%d", i, j, psize, key);

			memset(g_RTDB->sDCodePoint[i].sDevPoint[j].sPoint, 0, psize);

			//获取解析的数据
			point_t* src = get_pointArr(i, j);

			dev_info_t* dev_item = get_devPointMapArr(i, j);
			if(src == NULL || dev_item == NULL)
			{
				continue;
			}
			
			//目标申请的指针
			point_t* dest = g_RTDB->sDCodePoint[i].sDevPoint[j].sPoint;
			uint16_t pointVildNum = dev_item->pointNum >= gStDevTypePointNum[i] ? gStDevTypePointNum[i] : dev_item->pointNum;

			for (k = 0; k < pointVildNum; k++)
			{
				if(dev_item->pointArr[k].pointId < gStDevTypePointNum[i])
				{					
					dest[dev_item->pointArr[k].pointId] = src[k];
				}
			}
		}

		/*给web交互使用*/
		if (i == kDev_Type_Con && type == kEE_SHM_CREAT)
		{
			for (int idx = kDev_Type_Start; idx < kDev_Type_End; idx++)
			{
				{
					g_RTDB->sDCodePoint[i].sDevPoint[0].sPoint[kCon_DevNumStart + idx].value = gStDevTypeNum[idx];
				}
			}
		}
	}
	return SHM_OK;
}

/*********************************************************************
 * @brief      获取某个具体点的数据接口
 * @param[in]   devType: 设备类型
 * @param[in]   devId:   设备ID号
 * @param[in]   pointId: 测点编号
 * @return      1-成功  0失败 128 无此设备
 *********************************************************************/
double getShmPointValue(uint16_t devType, uint16_t devId, uint16_t pointId)
{
	if (devType >= 0 && devType < kDev_Type_End && devId <= gStDevTypeNum[devType] && pointId < gStDevTypePointNum[devType])
	{
		return g_RTDB->sDCodePoint[devType].sDevPoint[devId].sPoint[pointId].value;
	}
	else
	{
		KITPTF(LOG_APP_EN, INFO_EN, "获取设备类型：%d，设备号：%d，测点号：%d，不存在", devType, devId, pointId);
		KITLOG(LOG_APP_EN, INFO_EN, "获取设备类型：%d，设备号：%d，测点号：%d，不存在", devType, devId, pointId);

#if DEBUG_SHM_ENABLE
		assert(devType > 0 && devType < kType_End && devId < gStDevTypeNum[devType] && pointId < gStDevTypePointNum[devType]);
#endif
		return 0;
	}
}

/*********************************************************************
 * @brief      获取某个设备的所有数据
 * @param[in]   devType: 设备类型
 * @param[in]   devId:   设备ID号
 * @return     浮点型Point_t数组指针
 *********************************************************************/
point_t *getShmDevValue(uint16_t devType, uint16_t devId)
{
	if (devType > 0 && devType < kDev_Type_End && devId <= gStDevTypeNum[devType])
	{
		return g_RTDB->sDCodePoint[devType].sDevPoint[devId].sPoint;
	}
	else
	{
		KITPTF(LOG_APP_EN, INFO_EN, "获取设备类型：%d，设备号：%d，不存在", devType, devId);
		KITLOG(LOG_APP_EN, INFO_EN, "获取设备类型：%d，设备号：%d，不存在", devType, devId);

#if DEBUG_SHM_ENABLE
		assert(devType > 0 && devType < kType_End && devId < gStDevTypeNum[devType] && pointId < gStDevTypePointNum[devType]);
#endif
		return NULL;
	}
}

/*********************************************************************
 * @brief      向实时库写入数据
 * @param[in]   devType: 设备类型
 * @param[in]   devId:   设备ID号
 * @param[in]   pointId: 测点编号
 * @param[in]   value: 	 测点值
 * @return      NONE
 *********************************************************************/
void setShmPointValue(uint16_t devType, uint16_t devId, uint16_t pointId, double value)
{
	if (devType >= 0 && devType < kDev_Type_End && devId <= gStDevTypeNum[devType] && pointId < gStDevTypePointNum[devType])
	{
		g_RTDB->sDCodePoint[devType].sDevPoint[devId].sPoint[pointId].value = value;
	}
	else
	{
		KITPTF(LOG_APP_EN, INFO_EN, "写入设备类型：%d，设备号：%d，测点号：%d，不存在", devType, devId, pointId);
		KITLOG(LOG_APP_EN, INFO_EN, "写入设备类型：%d，设备号：%d，测点号：%d，不存在", devType, devId, pointId);

#if DEBUG_SHM_ENABLE
		assert(devType > 0 && devType < kType_End && devId < gStDevTypeNum[devType] && pointId < gStDevTypePointNum[devType]);
#endif
	}
}





//---------------------------------------------------------------DSP相关交互信号----------------------------------------------------------------------

/*********************************************************************
 * @brief       初始化设置信号通知的共享内存
 * @param[in]   void: 无
 * @return      ture-成功  0-fail
 *********************************************************************/
int initWebDSPMsgShm(shm_creat_type_e type)
{
	// 为设置信号通知分配空间
	g_DspSign = (cfg_dsp_msg_mem_t *)cfgsign_shmen_set(IPC_WEB_DSP_MSG_KEY, sizeof(cfg_dsp_msg_mem_t), type);
	if (g_WebSign == NULL)
	{
		KITPTF(LOG_DB_EN, ERROR_EN, "init_cfgsign_shm error !!!!!", NULL);
		KITLOG(LOG_DB_EN, ERROR_EN, "init_cfgsign_shm error !!!!!", NULL);
		return 1;
	}
	else
	{
		memset(g_WebSign, 0, sizeof(cfg_dsp_msg_mem_t));
		return 0;
	}
}

/*********************************************************************
 * @brief       web将当前修改配置项的id写入到共享内存中
 * @param[in]   cfgIndex: 配置项索引，从0开始依次加1
 * @param[in]   value: 配置项的值，为数据库的id
 * @return      配置项个数
 *********************************************************************/
uint8_t writeWebDspCfgArrayIdShm(uint8_t cfgIndex,uint16_t value)
{
	if(cfgIndex >= MAX_CONFIG_ITEMS || g_DspSign == NULL)
	{
		return 1; // 错误值
	}

	uint16_t *item = (uint16_t *)g_DspSign->config_ids;
	if (cfgIndex < MAX_CONFIG_ITEMS)
	{
		item[cfgIndex] = value;
		return 0; // 成功
	}
	else
	{
		return 1; // 错误值
	}
}

/*********************************************************************
 * @brief       web将当前修改配置项的个数写入到共享内存中
 * @param[in]   cfgCount: 配置项个数
 * @return      0-成功 1-失败
 *********************************************************************/
uint8_t writeWebDspCfgIdCountShm(uint16_t cfgCount)
{
	if(cfgCount >= MAX_CONFIG_ITEMS || g_DspSign == NULL)
	{
		return 1; // 错误值
	}

	g_DspSign->item_count = cfgCount;
	return 0; // 成功
}


/*********************************************************************
 * @brief       获取升级结果
 * @param[in]   dspIndex: DSP索引
 * @return      获取值
 *********************************************************************/
UpgradeError readWebUpgradeRetShm(uint8_t dspIndex)
{
	if (g_DspSign == NULL || dspIndex >= 2*MAX_DSP_NUM)
	{
		return 0;
	}

	UpgradeError *item = (UpgradeError *)g_DspSign->upgrade_ret;
	return item[dspIndex];
}
/*********************************************************************
 * @brief       获取配置返回结果
 * @param[in]   cfgId: 配置id
 * @return      获取值
 *********************************************************************/
int16_t readWebDspCfgRetShm(uint8_t cfgId)
{
	if(cfgId >= MAX_CONFIG_ITEMS || g_DspSign == NULL)
	{
		return 0;
	}
	uint16_t *item = (uint16_t *)g_DspSign->config_result;
	if (cfgId < MAX_CONFIG_ITEMS)
	{
		return item[cfgId];
	}
	else
	{
		return -1; // 错误值
	}
}
/*********************************************************************
 * @brief       获取自动编址结果
 * @param[in]   dspIndex: DSP索引
 * @return      获取值
 *********************************************************************/
UpgradeError readWebAutoAddrRetShm(uint8_t dspIndex)
{
	if (g_DspSign == NULL || dspIndex >= 2*MAX_DSP_NUM)
	{
		return 0;
	}

	UpgradeError *item = (UpgradeError *)g_DspSign->autoaddr_ret;
	return item[dspIndex];
}

/*********************************************************************
 * @brief       获取升级的Web配置的cpu选项
 * @return      cpu选项值
 *********************************************************************/
int16_t readWebCfgUpgradeCpuShm()
{
	if (g_DspSign == NULL)
	{
		return -1;
	}

	return g_DspSign->dsp_select_cpu;
}

/*********************************************************************
 * @brief       写入升级的Web配置的cpu选项
 * @details     该函数用于设置共享内存中的CPU选择值。
 * @param[in]   cpu_value: CPU选择值，0表示CPU0，1表示CPU1
 * @note        如果传入的值不在0或1之间，则不会修改共享内存中的值
 *********************************************************************/
void writeWebCfgUpgradeCpuShm(uint16_t cpu_value)
{
	if (g_DspSign == NULL)
	{
		return;
	}

	if (cpu_value < 2) // 0 or 1
	{
		g_DspSign->dsp_select_cpu = cpu_value;
	}
	else
	{
		KITPTF(LOG_APP_EN, INFO_EN, "writeWebCfgUpgradeCpuShm: cpu_value %d is invalid", cpu_value);
		KITLOG(LOG_APP_EN, INFO_EN, "writeWebCfgUpgradeCpuShm: cpu_value %d is invalid", cpu_value);
	}
}
/*********************************************************************
 * @brief       获取当前操作的DSP ID
 * @return      当前操作的DSP ID
 * @details     该函数用于获取当前正在操作的DSP ID。
 *              如果g_DspSign为NULL，则返回0。
 * *********************************************************************/
uint16_t readWebCfgCurDspIdShm()
{
	if (g_DspSign == NULL)
	{
		return 0;
	}

	return g_DspSign->cur_dsp_id;
}
/*********************************************************************
 * @brief       写入当前操作的DSP ID
 * @param[in]   dspId: 当前操作的DSP ID，从0开始
 * @details     该函数用于设置当前正在操作的DSP ID。
 *              如果g_DspSign为NULL或dspId超过最大DSP数量，则不进行任何操作。
 *********************************************************************/
void writeWebCfgCurDspIdShm(uint8_t dspId)
{
	if (g_DspSign == NULL || dspId >= MAX_DSP_NUM)
	{
		return; // 错误值
	}

	g_DspSign->cur_dsp_id = dspId;
}