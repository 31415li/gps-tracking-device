/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"

#ifdef RTE_Compiler_EventRecorder
#include "EventRecorder.h"
#endif

#include "rtx_os.h" // ARM::CMSIS:RTOS2:Keil RTX5
#include "app_cfg.h"
#include "bsp.h"
#include "App_Thread.h"
#include "Position_Thread.h"
#include "Sim808_Thread.h"
#include "App_Thread.h"
#include "system.h"
#include "flash.h"
#include "record.h"
#include "sim808_app.h"

static void AppThreadCreate(void);
static void AppObjCreate(void);

// Define objects that are statically allocated
__attribute__((section(".bss.os.thread.cb")))
osRtxThread_t app_start_thread_tcb;
// Reserve two areas for the stacks
// uint64_t makes sure the memory alignment is 8
uint64_t app_start_thread_stk[APP_START_THREAD_STK_SIZE];
// Define the attributes which are used for thread creation
const osThreadAttr_t app_start_thread_attr = {
    "start",
    osThreadJoinable,
    &app_start_thread_tcb,
    sizeof(app_start_thread_tcb),
    &app_start_thread_stk[0],
    sizeof(app_start_thread_stk),
    APP_START_THREAD_PRIO,
    0};
// Define ID object for thread
osThreadId_t app_start_thread_id;

// Define objects that are statically allocated
__attribute__((section(".bss.os.mutex.cb")))
osRtxMutex_t eeprom_mutex_tcb;
// Define ID object for mutex
osMutexId_t eeprom_mutex_id;
// Define the attributes which are used for mutex creation
const osMutexAttr_t eeprom_mutex_attr = {
    "eepromMutex",
    osMutexRobust,
    &eeprom_mutex_tcb,
    sizeof(eeprom_mutex_tcb)};

// Define objects that are statically allocated
__attribute__((section(".bss.os.mutex.cb")))
osRtxMutex_t spi_mutex_tcb;
// Define ID object for mutex
osMutexId_t spi_mutex_id;
// Define the attributes which are used for mutex creation
const osMutexAttr_t spi_mutex_attr = {
    "spiMutex",
    osMutexRobust,
    &spi_mutex_tcb,
    sizeof(spi_mutex_tcb)};

// Define objects that are statically allocated
__attribute__((section(".bss.os.mutex.cb")))
osRtxMutex_t record_mutex_tcb;
// Define ID object for mutex
osMutexId_t record_mutex_id;
// Define the attributes which are used for mutex creation
const osMutexAttr_t record_mutex_attr = {
    "recordMutex",
    osMutexRobust,
    &record_mutex_tcb,
    sizeof(record_mutex_tcb)};

/*----------------------------------------------------------------------------
 * Application main thread
 *---------------------------------------------------------------------------*/
void app_main(void *argument)
{
   uint32_t flag;

   AppObjCreate();
   AppThreadCreate();

   APP_TRACE_INFO(("Power ON MCU\r"));
   APP_TRACE_INFO(("SourceReset: %d\r", resetSource));

   APP_TRACE_INFO(("Parameter Load\r"));
   parameterLoad();

   recordLock();
   if (sFlashInit() == FLASH_DEVICE_OK)
   {
      osEventFlagsSet(app_eventflag_id, APP_SPI_FLASH_READY_FLAG);
      APP_TRACE_INFO(("SPI Flash OK\r"));
   }
   else
   {
      APP_TRACE_INFO(("SPI Flash Not Detected\r"));
   }
   recordUnLock();

   flag = osEventFlagsGet(app_eventflag_id);
   if (flag & APP_SPI_FLASH_READY_FLAG)
   {
      recordLock();
      if (recordInit() != RECORD_ERROR_EEPROM)
         osEventFlagsSet(app_eventflag_id, APP_FLASH_INIT_RECORD_FLAG);
      recordUnLock();
   }
   if (osEventFlagsGet(app_eventflag_id) & APP_FLASH_INIT_RECORD_FLAG)
      APP_TRACE_INFO(("Flash (Eeprom) Init 1\r"));
   else
      APP_TRACE_INFO(("Flash (Eeprom) Init 0\r"));

   APP_TRACE_INFO(("Start Program\r"));

   osThreadTerminate(app_start_thread_id);

   for (;;)
   {
      osDelay(1000);
   }
}

int main(void)
{
   uint32_t param = NULL;

#ifdef FAULT_REPORT_MODE_EN
/*
   SCB->SHCSR  |= SCB_SHCSR_USGFAULTENA_Msk
               |  SCB_SHCSR_BUSFAULTENA_Msk
               |  SCB_SHCSR_MEMFAULTENA_Msk; // enable Usage-, Bus-, and MMU Fault
               */
#endif

   // System Initialization
   SystemCoreClockUpdate();
   bspInit();

#ifdef RTE_Compiler_EventRecorder
   // Initialize and start Event Recorder
   EventRecorderInitialize(EventRecordError, 1U);
#endif
   // ...

   osKernelInitialize(); // Initialize CMSIS-RTOS
   // Create an instance of the start thread with static resources (TCB and stack)
   app_start_thread_id = osThreadNew(app_main, &param, &app_start_thread_attr);
   osKernelStart(); // Start thread execution
   for (;;)
   {
   }
}

static void AppObjCreate(void)
{

   // Create eeprom mutex
   eeprom_mutex_id = osMutexNew(&eeprom_mutex_attr);

   // Create flash mutex
   spi_mutex_id = osMutexNew(&spi_mutex_attr);

   // Create record mutex
   record_mutex_id = osMutexNew(&record_mutex_attr);

#if SIM808_THREAD_EN
   Sim808_ObjCreate();
#endif

#if APP_THREAD_EN
   App_ObjCreate();
#endif

#if POSITION_THREAD_EN
   Position_ObjCreate();
#endif
}

static void AppThreadCreate(void)
{

#if SIM808_THREAD_EN
   Sim808_ThreadCreate();
#endif

#if APP_THREAD_EN
   App_ThreadCreate();
#endif

#if POSITION_THREAD_EN
   Position_ThreadCreate();
#endif
}
