/*------------------------------------------------------------------------*/
/* Sample code of OS dependent controls for FatFs                         */
/* (C)ChaN, 2017                                                          */
/*------------------------------------------------------------------------*/

//#include "bsp_includes.h"
#include "stdlib.h"
#include "stdint.h"
#include "my_malloc.h"
#include "ff.h"



#if FF_USE_LFN == 3 /* Dynamic memory allocation */

/*------------------------------------------------------------------------*/
/* Allocate a memory block                                                */
/*------------------------------------------------------------------------*/

void *ff_memalloc(  /* Returns pointer to the allocated memory block (null on not enough core) */
    UINT msize      /* Number of bytes to allocate */
)
{
    //return (void*)mallocApi(SRAMIN,msize);
    return mymalloc(SRAMIN,msize);   /* Allocate a new memory block with POSIX API */
}


/*------------------------------------------------------------------------*/
/* Free a memory block                                                    */
/*------------------------------------------------------------------------*/

void ff_memfree(
    void *mblock    /* Pointer to the memory block to free */
)
{
    //freeApi(SRAMIN,mblock);
    myfree(SRAMIN, mblock);   /* Free the memory block with POSIX API */
}

#endif

//
//User defined function to give a current time to fatfs module      */
//31-25: Year(0-127 org.1980), 24-21: Month(1-12), 20-16: Day(1-31) */
//15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) */
DWORD get_fattime(void)
{
    #include "time.h"
  
    uint32_t time_now = time(0);
    struct tm * time_now_tm = localtime(&time_now);
    time_now = (((time_now_tm->tm_year-1980) & 0x7F) << 25) + 
               ((time_now_tm->tm_mon & 0x7) << 21) + 
               ((time_now_tm->tm_mday & 0x1F) << 16) + 
               ((time_now_tm->tm_hour & 0x1F) << 11) + 
               ((time_now_tm->tm_mon & 0x3F) << 5) + 
               ((time_now_tm->tm_mon & 0x3F) >> 1);
    return time_now;
}



#if FF_FS_REENTRANT /* Mutal exclusion */

/*------------------------------------------------------------------------*/
/* Create a Synchronization Object*/
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to create a new
/  synchronization object for the volume, such as semaphore and mutex.
/  When a 0 is returned, the f_mount() function fails with FR_INT_ERR.
*/

//const osMutexDef_t Mutex[FF_VOLUMES]; /* CMSIS-RTOS */



int ff_cre_syncobj(  /* 1:Function succeeded, 0:Could not create the sync object */
    BYTE vol,           /* Corresponding volume (logical drive number) */
    FF_SYNC_t *sobj     /* Pointer to return the created sync object */
)
{
    /* Win32 */
    //*sobj = CreateMutex(NULL, FALSE, NULL);
    //return (int)(*sobj != INVALID_HANDLE_VALUE);

    /* uITRON */
    //  T_CSEM csem = {TA_TPRI,1,1};
    //  *sobj = acre_sem(&csem);
    //  return (int)(*sobj > 0);

    /* uC/OS-II */
      OS_ERR err;
      *sobj = OSMutexCreate(0, &err);
      return (int)(err == OS_NO_ERR);

    /* FreeRTOS */
    //  *sobj = xSemaphoreCreateMutex();
    //  return (int)(*sobj != NULL);

    /* CMSIS-RTOS */
    //  *sobj = osMutexCreate(Mutex + vol);
    //  return (int)(*sobj != NULL);
}


/*------------------------------------------------------------------------*/
/* Delete a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to delete a synchronization
/  object that created with ff_cre_syncobj() function. When a 0 is returned,
/  the f_mount() function fails with FR_INT_ERR.
*/

int ff_del_syncobj(  /* 1:Function succeeded, 0:Could not delete due to an error */
    FF_SYNC_t sobj      /* Sync object tied to the logical drive to be deleted */
)
{
    /* Win32 */
    //return (int)CloseHandle(sobj);

    /* uITRON */
    //  return (int)(del_sem(sobj) == E_OK);

    /* uC/OS-II */
      OS_ERR err;
      OSMutexDel(sobj, OS_DEL_ALWAYS, &err);
      return (int)(err == OS_NO_ERR);

    /* FreeRTOS */
    //  vSemaphoreDelete(sobj);
    //  return 1;

    /* CMSIS-RTOS */
    //  return (int)(osMutexDelete(sobj) == osOK);
}


/*------------------------------------------------------------------------*/
/* Request Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on entering file functions to lock the volume.
/  When a 0 is returned, the file function fails with FR_TIMEOUT.
*/

int ff_req_grant(  /* 1:Got a grant to access the volume, 0:Could not get a grant */
    FF_SYNC_t sobj  /* Sync object to wait */
)
{
    /* Win32 */
    //return (int)(WaitForSingleObject(sobj, FF_FS_TIMEOUT) == WAIT_OBJECT_0);

    /* uITRON */
    //  return (int)(wai_sem(sobj) == E_OK);

    /* uC/OS-II */
      OS_ERR err;
      OSMutexPend(sobj, FF_FS_TIMEOUT, &err);
      return (int)(err == OS_NO_ERR);

    /* FreeRTOS */
    //  return (int)(xSemaphoreTake(sobj, FF_FS_TIMEOUT) == pdTRUE);

    /* CMSIS-RTOS */
    //  return (int)(osMutexWait(sobj, FF_FS_TIMEOUT) == osOK);
}


/*------------------------------------------------------------------------*/
/* Release Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on leaving file functions to unlock the volume.
*/

void ff_rel_grant(
    FF_SYNC_t sobj  /* Sync object to be signaled */
)
{
    /* Win32 */
    //ReleaseMutex(sobj);

    /* uITRON */
    //  sig_sem(sobj);

    /* uC/OS-II */
      OSMutexPost(sobj);

    /* FreeRTOS */
    //  xSemaphoreGive(sobj);

    /* CMSIS-RTOS */
    //  osMutexRelease(sobj);
}

#endif

