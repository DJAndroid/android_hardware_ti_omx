
/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
/* =============================================================================
 *             Texas Instruments OMAP (TM) Platform Software
 *  (c) Copyright Texas Instruments, Incorporated.  All Rights Reserved.
 *
 *  Use of this software is controlled by the terms and conditions found
 *  in the license agreement under which this software has been supplied.
 * =========================================================================== */
/**
 * @file OMX_Mp3Dec_Utils.c
 *
 * This file implements various utilitiy functions for various activities
 * like handling command from application, callback from LCML etc.
 *
 * @path  $(CSLPATH)\OMAPSW_MPU\linux\audio\src\openmax_il\mp3_dec\src
 *
 * @rev  1.0
 */
/* ----------------------------------------------------------------------------
 *!
 *! Revision History
 *! ===================================
 *! 21-sept-2006 bk: updated some review findings for alpha release
 *! 24-Aug-2006 bk: Khronos OpenMAX (TM) 1.0 Conformance tests some more
 *! 18-July-2006 bk: Khronos OpenMAX (TM) 1.0 Conformance tests validated for few cases
 *! This is newest file
 * =========================================================================== */
/* ------compilation control switches -------------------------*/
/****************************************************************
 *  INCLUDE FILES
 ****************************************************************/
/* ----- system and platform files ----------------------------*/



#ifdef UNDER_CE
#include <windows.h>
#include <oaf_osal.h>
#include <omx_core.h>
#include <stdlib.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <malloc.h>
#include <memory.h>
#include <fcntl.h>
#endif

#include <dbapi.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

/*------- Program Header Files -----------------------------------------------*/
#include "LCML_DspCodec.h"
#include "OMX_Mp3Dec_Utils.h"
#include "mp3decsocket_ti.h"
#include "decode_common_ti.h"
#include "usn.h"

/*#ifdef MP3D_RM_MANAGER*/
#ifdef RESOURCE_MANAGER_ENABLED
#include <ResourceManagerProxyAPI.h>
#endif

#ifdef UNDER_CE
#define HASHINGENABLE 1
HINSTANCE g_hLcmlDllHandle = NULL;
void sleep(DWORD Duration)
{
    Sleep(Duration);

}
#endif

#ifdef MP3DEC_MEMDEBUG
#define newmalloc(x) mymalloc(__LINE__,__FILE__,x)
#define newfree(z) myfree(z,__LINE__,__FILE__)
#else
#define newmalloc(x) malloc(x)
#define newfree(z) free(z)
#endif


/* ================================================================================= * */
/**
 * @fn MP3DEC_Fill_LCMLInitParams() fills the LCML initialization structure.
 *
 * @param pHandle This is component handle allocated by the OMX core. 
 *
 * @param plcml_Init This structure is filled and sent to LCML. 
 *
 * @pre          None
 *
 * @post         None
 *
 *  @return      OMX_ErrorNone = Successful Inirialization of the LCML struct.
 *               OMX_ErrorInsufficientResources = Not enough memory
 *
 *  @see         None
 */
/* ================================================================================ * */
OMX_ERRORTYPE MP3DEC_Fill_LCMLInitParams(OMX_HANDLETYPE pComponent,LCML_DSP *plcml_Init,OMX_U16 arr[])
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 nIpBuf,nIpBufSize,nOpBuf,nOpBufSize;
    OMX_U32 i;
    OMX_BUFFERHEADERTYPE *pTemp = NULL;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    MP3DEC_COMPONENT_PRIVATE *pComponentPrivate =(MP3DEC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;
    MP3D_LCML_BUFHEADERTYPE *pTemp_lcml;
    char *char_temp = NULL;
    OMX_U32 size_lcml;
    OMX_U8 *ptr;

    pComponentPrivate->nRuntimeInputBuffers = 0;
    pComponentPrivate->nRuntimeOutputBuffers = 0;
  
    MP3DEC_DPRINT("Entered MP3DEC_Fill_LCMLInitParams\n");
    MP3DEC_DPRINT(":::pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bPopulated = %d\n",
                  pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bPopulated);
    MP3DEC_DPRINT(":::pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bEnabled = %d\n",
                  pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bEnabled);
    MP3DEC_DPRINT(":::pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bPopulated = %d\n",
                  pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bPopulated);
    MP3DEC_DPRINT(":::pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bEnabled = %d\n",
                  pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bEnabled);

    pComponentPrivate->strmAttr = NULL;

    nIpBuf = pComponentPrivate->pInputBufferList->numBuffers;
    pComponentPrivate->nRuntimeInputBuffers = nIpBuf;
    nOpBuf = pComponentPrivate->pOutputBufferList->numBuffers;
    pComponentPrivate->nRuntimeOutputBuffers = nOpBuf;
    nIpBufSize = pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->nBufferSize;
    nOpBufSize = pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->nBufferSize;


    MP3DEC_DPRINT("Input Buffer Count = %ld\n",nIpBuf);
    MP3DEC_DPRINT("Input Buffer Size = %ld\n",nIpBufSize);
    MP3DEC_DPRINT("Output Buffer Count = %ld\n",nOpBuf);
    MP3DEC_DPRINT("Output Buffer Size = %ld\n",nOpBufSize);

    plcml_Init->In_BufInfo.nBuffers = nIpBuf;
    plcml_Init->In_BufInfo.nSize = nIpBufSize;
    plcml_Init->In_BufInfo.DataTrMethod = DMM_METHOD;
    plcml_Init->Out_BufInfo.nBuffers = nOpBuf;
    plcml_Init->Out_BufInfo.nSize = nOpBufSize;
    plcml_Init->Out_BufInfo.DataTrMethod = DMM_METHOD;


    plcml_Init->NodeInfo.nNumOfDLLs = 3;

    memset(plcml_Init->NodeInfo.AllUUIDs[0].DllName,0, sizeof(plcml_Init->NodeInfo.AllUUIDs[0].DllName));
    memset(plcml_Init->NodeInfo.AllUUIDs[1].DllName,0, sizeof(plcml_Init->NodeInfo.AllUUIDs[1].DllName));
    memset(plcml_Init->NodeInfo.AllUUIDs[2].DllName,0, sizeof(plcml_Init->NodeInfo.AllUUIDs[1].DllName));
    memset(plcml_Init->NodeInfo.AllUUIDs[0].DllName,0, sizeof(plcml_Init->DeviceInfo.AllUUIDs[1].DllName));

    plcml_Init->NodeInfo.AllUUIDs[0].uuid = &MP3DECSOCKET_TI_UUID;
    strcpy ((char*)plcml_Init->NodeInfo.AllUUIDs[0].DllName, MP3DEC_DLL_NAME);
    plcml_Init->NodeInfo.AllUUIDs[0].eDllType = DLL_NODEOBJECT;

    plcml_Init->NodeInfo.AllUUIDs[1].uuid = &MP3DECSOCKET_TI_UUID;
    strcpy ((char*)plcml_Init->NodeInfo.AllUUIDs[1].DllName, MP3DEC_DLL_NAME);
    plcml_Init->NodeInfo.AllUUIDs[1].eDllType = DLL_DEPENDENT;

    plcml_Init->NodeInfo.AllUUIDs[2].uuid = &USN_TI_UUID;
    strcpy ((char*)plcml_Init->NodeInfo.AllUUIDs[2].DllName, MP3DEC_USN_DLL_NAME);
    plcml_Init->NodeInfo.AllUUIDs[2].eDllType = DLL_DEPENDENT;

    plcml_Init->SegID = OMX_MP3DEC_DEFAULT_SEGMENT;
    plcml_Init->Timeout = OMX_MP3DEC_SN_TIMEOUT;
    plcml_Init->Alignment = 0;
    plcml_Init->Priority = OMX_MP3DEC_SN_PRIORITY;
    plcml_Init->ProfileID = -1;

    if(pComponentPrivate->dasfmode == 1) {
#ifndef DSP_RENDERING_ON
        MP3D_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,
                            "Flag DSP_RENDERING_ON Must Be Defined To Use Rendering");
#else
        LCML_STRMATTR *strmAttr;
        MP3D_OMX_MALLOC(strmAttr, LCML_STRMATTR);
        MP3DEC_DPRINT(": Malloc strmAttr = %p\n",strmAttr);
        pComponentPrivate->strmAttr = strmAttr;
        MP3DEC_DPRINT(":: MP3 DECODER IS RUNNING UNDER DASF MODE \n");

        strmAttr->uSegid = 0;
        strmAttr->uAlignment = 0;
        strmAttr->uTimeout = -1;

	 strmAttr->uBufsize = MP3D_OUTPUT_BUFFER_SIZE;
	
	MP3DEC_DPRINT("::strmAttr->uBufsize:%d\n",strmAttr->uBufsize);

        strmAttr->uNumBufs = 2;
        strmAttr->lMode = STRMMODE_PROCCOPY;
        plcml_Init->DeviceInfo.TypeofDevice = 1;
        plcml_Init->DeviceInfo.TypeofRender = 0;

        plcml_Init->DeviceInfo.AllUUIDs[0].uuid = &DCTN_TI_UUID;
        plcml_Init->DeviceInfo.DspStream = strmAttr;
#endif
    } else {
        plcml_Init->DeviceInfo.TypeofDevice = 0;
    }

    if (pComponentPrivate->dasfmode == 0){        
        MP3DEC_DPRINT(":: FILE MODE CREATE PHASE PARAMETERS\n");
        arr[0] = 2;            /* Number of Streams */
        arr[1] = 0;            /* ID of the Input Stream */
        arr[2] = 0;            /* Type of Input Stream DMM (0) / STRM (1) */
#ifndef UNDER_CE
        arr[3] = 4;            /* Number of buffers for Input Stream */
#else
        arr[3] = 1;            /* WinCE Number of buffers for Input Stream */
#endif
        arr[4] = 1;            /* ID of the Output Stream */
        arr[5] = 0;            /* Type of Output Stream  */
#ifndef UNDER_CE
        arr[6] = 4;            /* Number of buffers for Output Stream */
#else
        arr[6] = 1;            /* WinCE Number of buffers for Output Stream */
#endif

        if(pComponentPrivate->pcmParams->nBitPerSample == 24){
            MP3DEC_DPRINT(" PCM 24 bit output\n");
            arr[7] = 24;
        } else {
            MP3DEC_DPRINT(" PCM 16 bit output\n");
            arr[7] = 16;
        }
    
        if(pComponentPrivate->frameMode) {
            MP3DEC_DPRINT(" frame mode is on\n");
            arr[8] = 1;   /* frame mode is on */
        } else {
            arr[8] = 0;
            MP3DEC_DPRINT(" frame mode is off\n");
        }
        arr[9] = END_OF_CR_PHASE_ARGS;
    } else {
        MP3DEC_DPRINT(":: DASF MODE CREATE PHASE PARAMETERS\n");
        arr[0] = 2;        /* Number of Streams */
        arr[1] = 0;        /* ID of the Input Stream */
        arr[2] = 0;        /* Type of Input Stream DMM (0) / STRM (1) */
        arr[3] = 4;        /* Number of buffers for Input Stream */
        arr[4] = 1;        /* ID of the Output Stream */
        arr[5] = 2;        /* Type of Output Stream  */
        arr[6] = 2;        /* Number of buffers for Output Stream */
        arr[7] = 16;       /*Decoder Output PCM width is 24-bit or 16-bit */
        arr[8] = 0;        /* frame mode off */

        arr[9] = END_OF_CR_PHASE_ARGS;
    }

    plcml_Init->pCrPhArgs = arr;

    MP3DEC_DPRINT(":: bufAlloced = %d\n",pComponentPrivate->bufAlloced);
    size_lcml = nIpBuf * sizeof(MP3D_LCML_BUFHEADERTYPE);
    MP3D_OMX_MALLOC_SIZE(ptr,size_lcml,OMX_U8);
    pTemp_lcml = (MP3D_LCML_BUFHEADERTYPE *)ptr;

    pComponentPrivate->pLcmlBufHeader[MP3D_INPUT_PORT] = pTemp_lcml;

    for (i=0; i<nIpBuf; i++) {
        pTemp = pComponentPrivate->pInputBufferList->pBufHdr[i];
        pTemp->nSize = sizeof(OMX_BUFFERHEADERTYPE);

        pTemp->nAllocLen = nIpBufSize;
        pTemp->nFilledLen = nIpBufSize;
        pTemp->nVersion.s.nVersionMajor = MP3DEC_MAJOR_VER;
        pTemp->nVersion.s.nVersionMinor = MP3DEC_MINOR_VER;

        pTemp->pPlatformPrivate = pHandle->pComponentPrivate;
        pTemp->nTickCount = 0;

        pTemp_lcml->pBufHdr = pTemp;
        pTemp_lcml->eDir = OMX_DirInput;
        pTemp_lcml->pOtherParams[i] = NULL;
        MP3D_OMX_MALLOC_SIZE(pTemp_lcml->pIpParam,
                             (sizeof(MP3DEC_UAlgInBufParamStruct) + DSP_CACHE_ALIGNMENT),
                             MP3DEC_UAlgInBufParamStruct);
        char_temp = (char*)pTemp_lcml->pIpParam;
        char_temp += EXTRA_BYTES;
        pTemp_lcml->pIpParam = (MP3DEC_UAlgInBufParamStruct*)char_temp;
        pTemp_lcml->pIpParam->bLastBuffer = 0;

        pTemp->nFlags = NORMAL_BUFFER;
        ((MP3DEC_COMPONENT_PRIVATE *) pTemp->pPlatformPrivate)->pHandle = pHandle;

        MP3DEC_DPRINT("::Comp: InBuffHeader[%ld] = %p\n", i, pTemp);
        MP3DEC_DPRINT("::Comp:  >>>> InputBuffHeader[%ld]->pBuffer = %p\n", i, pTemp->pBuffer);
        MP3DEC_DPRINT("::Comp: Ip : pTemp_lcml[%ld] = %p\n", i, pTemp_lcml);

        pTemp_lcml++;
    }

    size_lcml = nOpBuf * sizeof(MP3D_LCML_BUFHEADERTYPE);
    MP3D_OMX_MALLOC_SIZE(pTemp_lcml,size_lcml,MP3D_LCML_BUFHEADERTYPE);
    pComponentPrivate->pLcmlBufHeader[MP3D_OUTPUT_PORT] = pTemp_lcml;

    for (i=0; i<nOpBuf; i++) {
        pTemp = pComponentPrivate->pOutputBufferList->pBufHdr[i];
        pTemp->nSize = sizeof(OMX_BUFFERHEADERTYPE);

        pTemp->nAllocLen = nOpBufSize;

        MP3DEC_DPRINT(":: nOpBufSize = %ld\n", nOpBufSize);

        pTemp->nVersion.s.nVersionMajor = MP3DEC_MAJOR_VER;
        pTemp->nVersion.s.nVersionMinor = MP3DEC_MINOR_VER;
        pTemp->pPlatformPrivate = pHandle->pComponentPrivate;
        pTemp->nTickCount = 0;

        pTemp_lcml->pBufHdr = pTemp;
        pTemp_lcml->eDir = OMX_DirOutput;
        pTemp_lcml->pOtherParams[i] = NULL;
        MP3D_OMX_MALLOC_SIZE(pTemp_lcml->pOpParam,
                             (sizeof(MP3DEC_UAlgOutBufParamStruct) + DSP_CACHE_ALIGNMENT),
                             MP3DEC_UAlgOutBufParamStruct);
        char_temp = (char*)pTemp_lcml->pOpParam;
        char_temp += EXTRA_BYTES;
        pTemp_lcml->pOpParam = (MP3DEC_UAlgOutBufParamStruct*)char_temp;
        pTemp_lcml->pOpParam->ulFrameCount = DONT_CARE;

        pTemp->nFlags = NORMAL_BUFFER;
        ((MP3DEC_COMPONENT_PRIVATE *)pTemp->pPlatformPrivate)->pHandle = pHandle;
        MP3DEC_DPRINT("::Comp:  >>>>>>>>>>>>> OutBuffHeader[%ld] = %p\n", i, pTemp);
        MP3DEC_DPRINT("::Comp:  >>>> OutBuffHeader[%ld]->pBuffer = %p\n", i, pTemp->pBuffer);
        MP3DEC_DPRINT("::Comp: Op : pTemp_lcml[%ld] = %p\n", i, pTemp_lcml);
        pTemp_lcml++;
    }
    pComponentPrivate->bPortDefsAllocated = 1;
    MP3D_OMX_MALLOC_SIZE(pComponentPrivate->pParams,(sizeof(USN_AudioCodecParams) + DSP_CACHE_ALIGNMENT),
                         USN_AudioCodecParams);
    char_temp = (char*) pComponentPrivate->pParams;
    char_temp +=EXTRA_BYTES;
    pComponentPrivate->pParams = (USN_AudioCodecParams *)char_temp;
    MP3D_OMX_MALLOC_SIZE(pComponentPrivate->ptAlgDynParams,(sizeof(MP3DEC_UALGParams) + DSP_CACHE_ALIGNMENT),
                         MP3DEC_UALGParams);
    char_temp = (char*) pComponentPrivate->ptAlgDynParams;
    char_temp += EXTRA_BYTES;
    pComponentPrivate->ptAlgDynParams = (MP3DEC_UALGParams *) char_temp;

#ifdef __PERF_INSTRUMENTATION__
    pComponentPrivate->nLcml_nCntIp = 0;
    pComponentPrivate->nLcml_nCntOpReceived = 0;
#endif  

    pComponentPrivate->bInitParamsInitialized = 1;

 EXIT:
    MP3DEC_DPRINT("Exiting MP3DEC_Fill_LCMLInitParams. error=%d\n", eError);

    return eError;
}


/* ================================================================================= * */
/**
 * @fn Mp3Dec_StartCompThread() starts the component thread. This is internal
 * function of the component.
 *
 * @param pHandle This is component handle allocated by the OMX core. 
 *
 * @pre          None
 *
 * @post         None
 *
 *  @return      OMX_ErrorNone = Successful Inirialization of the component\n
 *               OMX_ErrorInsufficientResources = Not enough memory
 *
 *  @see         None
 */
/* ================================================================================ * */
OMX_ERRORTYPE Mp3Dec_StartCompThread(OMX_HANDLETYPE pComponent)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    MP3DEC_COMPONENT_PRIVATE *pComponentPrivate =
        (MP3DEC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;
    int nRet = 0;

#ifdef UNDER_CE
    pthread_attr_t attr;
    memset(&attr, 0, sizeof(attr));
    attr.__inheritsched = PTHREAD_EXPLICIT_SCHED;
    attr.__schedparam.__sched_priority = OMX_AUDIO_DECODER_THREAD_PRIORITY;
#endif

    MP3DEC_DPRINT (":: Enetering  Mp3Dec_StartCompThread()\n");

    pComponentPrivate->lcml_nOpBuf = 0;
    pComponentPrivate->lcml_nIpBuf = 0;
    pComponentPrivate->app_nBuf = 0;
    pComponentPrivate->num_Op_Issued = 0;
    pComponentPrivate->num_Sent_Ip_Buff = 0;
    pComponentPrivate->num_Reclaimed_Op_Buff = 0;
    pComponentPrivate->bIsEOFSent = 0;

    nRet = pipe (pComponentPrivate->dataPipe);
    if (0 != nRet) {
        MP3D_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,
                            "Pipe Creation Failed");
    }

    nRet = pipe (pComponentPrivate->cmdPipe);
    if (0 != nRet) {
        MP3D_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,
                            "Pipe Creation Failed");
    }

    nRet = pipe (pComponentPrivate->cmdDataPipe);
    if (0 != nRet) {
        MP3D_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,
                            "Pipe Creation Failed");
    }


#ifdef UNDER_CE
    nRet = pthread_create (&(pComponentPrivate->ComponentThread), &attr,
                           MP3DEC_ComponentThread, pComponentPrivate);
#else
    nRet = pthread_create (&(pComponentPrivate->ComponentThread), NULL,
                           MP3DEC_ComponentThread, pComponentPrivate);
#endif                                       
    if ((0 != nRet) || (!pComponentPrivate->ComponentThread)) {
        MP3D_OMX_ERROR_EXIT(eError, OMX_ErrorInsufficientResources,
                            "Thread Creation Failed");
    }

    pComponentPrivate->bCompThreadStarted = 1;

    MP3DEC_DPRINT (":: Exiting from Mp3Dec_StartCompThread()\n");

 EXIT:
    return eError;
}


/* ================================================================================= * */
/**
 * @fn MP3DEC_FreeCompResources() function newfrees the component resources.
 *
 * @param pComponent This is the component handle.
 *
 * @pre          None
 *
 * @post         None
 *
 *  @return      OMX_ErrorNone = Successful Inirialization of the component\n
 *               OMX_ErrorHardware = Hardware error has occured.
 *
 *  @see         None
 */
/* ================================================================================ * */

OMX_ERRORTYPE MP3DEC_FreeCompResources(OMX_HANDLETYPE pComponent)
{
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    MP3DEC_COMPONENT_PRIVATE *pComponentPrivate = (MP3DEC_COMPONENT_PRIVATE *)
        pHandle->pComponentPrivate;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 nIpBuf=0, nOpBuf=0;
    int nRet=0;

    MP3DEC_DPRINT (":: Mp3Dec_FreeCompResources\n");

    MP3DEC_DPRINT(":::pComponentPrivate->bPortDefsAllocated = %ld\n",pComponentPrivate->bPortDefsAllocated);
    if (pComponentPrivate->bPortDefsAllocated) {
        nIpBuf = pComponentPrivate->pInputBufferList->numBuffers;
        nOpBuf = pComponentPrivate->pOutputBufferList->numBuffers;
    }
    MP3DEC_DPRINT(":: Closing pipess.....\n");

    nRet = close (pComponentPrivate->dataPipe[0]);
    if (0 != nRet && OMX_ErrorNone == eError) {
        eError = OMX_ErrorHardware;
    }

    nRet = close (pComponentPrivate->dataPipe[1]);
    if (0 != nRet && OMX_ErrorNone == eError) {
        eError = OMX_ErrorHardware;
    }

    nRet = close (pComponentPrivate->cmdPipe[0]);
    if (0 != nRet && OMX_ErrorNone == eError) {
        eError = OMX_ErrorHardware;
    }

    nRet = close (pComponentPrivate->cmdPipe[1]);
    if (0 != nRet && OMX_ErrorNone == eError) {
        eError = OMX_ErrorHardware;
    }

    nRet = close (pComponentPrivate->cmdDataPipe[0]);
    if (0 != nRet && OMX_ErrorNone == eError) {
        eError = OMX_ErrorHardware;
    }

    nRet = close (pComponentPrivate->cmdDataPipe[1]);


    if (0 != nRet && OMX_ErrorNone == eError) {
        eError = OMX_ErrorHardware;
    }

    if (pComponentPrivate->bPortDefsAllocated) {

        MP3D_OMX_FREE(pComponentPrivate->pPortDef[MP3D_INPUT_PORT]);
        MP3D_OMX_FREE(pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]);
        MP3D_OMX_FREE(pComponentPrivate->mp3Params);
        MP3D_OMX_FREE (pComponentPrivate->pcmParams);
        MP3D_OMX_FREE(pComponentPrivate->pCompPort[MP3D_INPUT_PORT]->pPortFormat);
        MP3D_OMX_FREE (pComponentPrivate->pCompPort[MP3D_OUTPUT_PORT]->pPortFormat);
        MP3D_OMX_FREE (pComponentPrivate->pCompPort[MP3D_INPUT_PORT]);
        MP3D_OMX_FREE (pComponentPrivate->pCompPort[MP3D_OUTPUT_PORT]);
        MP3D_OMX_FREE (pComponentPrivate->sPortParam);
        MP3D_OMX_FREE (pComponentPrivate->pPriorityMgmt);
        MP3D_OMX_FREE(pComponentPrivate->pInputBufferList);
        MP3D_OMX_FREE(pComponentPrivate->pOutputBufferList);
    }

    pComponentPrivate->bPortDefsAllocated = 0;

#ifndef UNDER_CE
    MP3DEC_DPRINT("\n\n FreeCompResources: Destroying mutexes.\n\n");
    pthread_mutex_destroy(&pComponentPrivate->InLoaded_mutex);
    pthread_cond_destroy(&pComponentPrivate->InLoaded_threshold);
    
    pthread_mutex_destroy(&pComponentPrivate->InIdle_mutex);
    pthread_cond_destroy(&pComponentPrivate->InIdle_threshold);
    
    pthread_mutex_destroy(&pComponentPrivate->AlloBuf_mutex);
    pthread_cond_destroy(&pComponentPrivate->AlloBuf_threshold);
#else
    OMX_DestroyEvent(&(pComponentPrivate->InLoaded_event));
    OMX_DestroyEvent(&(pComponentPrivate->InIdle_event));
    OMX_DestroyEvent(&(pComponentPrivate->AlloBuf_event));
#endif

    return eError;
}


/* ================================================================================= * */
/**
 * @fn MP3DEC_HandleCommand() function handles the command sent by the application.
 * All the state transitions, except from nothing to loaded state, of the
 * component are done by this function. 
 *
 * @param pComponentPrivate  This is component's private date structure.
 *
 * @pre          None
 *
 * @post         None
 *
 *  @return      OMX_ErrorNone = Successful processing.
 *               OMX_ErrorInsufficientResources = Not enough memory
 *               OMX_ErrorHardware = Hardware error has occured lile LCML failed
 *               to do any said operartion.
 *
 *  @see         None
 */
/* ================================================================================ * */

OMX_U32 MP3DEC_HandleCommand (MP3DEC_COMPONENT_PRIVATE *pComponentPrivate)
{
    OMX_U32 i,ret = 0;
    OMX_U16 arr[24];
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    char *pArgs = "damedesuStr";
    OMX_COMPONENTTYPE *pHandle =(OMX_COMPONENTTYPE *) pComponentPrivate->pHandle;
    OMX_COMMANDTYPE command;
    OMX_STATETYPE commandedState;
    OMX_U32 commandData;
    OMX_HANDLETYPE pLcmlHandle = pComponentPrivate->pLcmlHandle;

#ifdef RESOURCE_MANAGER_ENABLED
    OMX_ERRORTYPE rm_error = OMX_ErrorNone;
#endif

    MP3DEC_DPRINT (":: >>> Entering HandleCommand Function\n");

    ret = read(pComponentPrivate->cmdPipe[0], &command, sizeof (command));
    if(ret == -1){
        MP3D_OMX_ERROR_EXIT(eError, 
                            OMX_ErrorHardware,
                            "Error while reading the command pipe");
        pComponentPrivate->cbInfo.EventHandler (pHandle, 
                                                pHandle->pApplicationPrivate,
                                                OMX_EventError, 
                                                eError,
                                                OMX_TI_ErrorSevere,
                                                NULL);
    }
    ret = read(pComponentPrivate->cmdDataPipe[0], &commandData, sizeof (commandData));
    if(ret == -1){
        MP3D_OMX_ERROR_EXIT(eError, OMX_ErrorHardware,
                            "Error while reading the commandData pipe");
        pComponentPrivate->cbInfo.EventHandler (pHandle, 
                                                pHandle->pApplicationPrivate,
                                                OMX_EventError, 
                                                eError,
                                                OMX_TI_ErrorSevere,
                                                NULL);
    }
    MP3DEC_DPRINT("---------------------------------------------\n");
    MP3DEC_DPRINT(":: command = %d\n",command);
    MP3DEC_DPRINT(":: commandData = %ld\n",commandData);
    MP3DEC_DPRINT("---------------------------------------------\n");
#ifdef __PERF_INSTRUMENTATION__
    PERF_ReceivedCommand(pComponentPrivate->pPERFcomp,
                         command,
                         commandData,
                         PERF_ModuleLLMM);
#endif  
    if (command == OMX_CommandStateSet){
        commandedState = (OMX_STATETYPE)commandData;
        if (pComponentPrivate->curState == commandedState) {
            pComponentPrivate->cbInfo.EventHandler (pHandle, 
                                                    pHandle->pApplicationPrivate,
                                                    OMX_EventError, 
                                                    OMX_ErrorSameState,
                                                    OMX_TI_ErrorMinor,
                                                    NULL);

            MP3DEC_EPRINT(":: Error: Same State Given by Application\n");
        } else {

            switch(commandedState) {

            case OMX_StateIdle:
                MP3DEC_DPRINT(": HandleCommand: Cmd Idle \n");
                if (pComponentPrivate->curState == OMX_StateLoaded || pComponentPrivate->curState == OMX_StateWaitForResources) {
                    LCML_CALLBACKTYPE cb;
                    LCML_DSP *pLcmlDsp;
                    char *p = "damedesuStr";
#ifdef __PERF_INSTRUMENTATION__
                    PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryStart | PERF_BoundarySetup);
#endif
                    int inputPortFlag=0,outputPortFlag=0;

                    if (pComponentPrivate->dasfmode == 1) {
                        pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bEnabled= FALSE;
                        pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bPopulated= FALSE;
                        if(pComponentPrivate->streamID == 0) { 
                            MP3DEC_EPRINT("**************************************\n"); 
                            MP3DEC_EPRINT(":: Error = OMX_ErrorInsufficientResources\n"); 
                            MP3DEC_EPRINT("**************************************\n"); 
                            pComponentPrivate->curState = OMX_StateInvalid; 
                            eError = OMX_ErrorInsufficientResources; 
                            pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                                   pHandle->pApplicationPrivate, 
                                                                   OMX_EventError, 
                                                                   eError,
                                                                   OMX_TI_ErrorMajor, 
                                                                   "AM: No Stream ID Available");                 
                            goto EXIT; 
                        } 
                    }


                    MP3DEC_DPRINT("In while loop: IP : %p OP: %p\n",pComponentPrivate->pPortDef[MP3D_INPUT_PORT], 
                                  pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]);
                    MP3DEC_DPRINT("pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bPopulated = %d\n",
                                  pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bPopulated);
                    MP3DEC_DPRINT("pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bPopulated = %d\n",
                                  pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bPopulated);
                    MP3DEC_DPRINT("pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bEnabled = %d\n",
                                  pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bEnabled);
                    MP3DEC_DPRINT("pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bEnabled = %d\n",
                                  pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bEnabled);
                     

                    if (pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bPopulated && 
                        pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bEnabled)  {
                        inputPortFlag = 1;
                    }

                    if (!pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bPopulated && 
                        !pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bEnabled) {
                        inputPortFlag = 1;
                    }

                    if (pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bPopulated && 
                        pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bEnabled) {
                        outputPortFlag = 1;
                    }

                    if (!pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bPopulated && 
                        !pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bEnabled) {
                        outputPortFlag = 1;
                    }

                    if(!(inputPortFlag && outputPortFlag)) {
                        pComponentPrivate->InLoaded_readytoidle = 1;
#ifndef UNDER_CE        
                        pthread_mutex_lock(&pComponentPrivate->InLoaded_mutex); 
                        pthread_cond_wait(&pComponentPrivate->InLoaded_threshold, &pComponentPrivate->InLoaded_mutex);
                        pthread_mutex_unlock(&pComponentPrivate->InLoaded_mutex);
#else
                        OMX_WaitForEvent(&(pComponentPrivate->InLoaded_event));
#endif
                    }

                    pLcmlHandle = (OMX_HANDLETYPE) MP3DEC_GetLCMLHandle(pComponentPrivate);
                    if (pLcmlHandle == NULL) {
                        MP3DEC_EPRINT(":: LCML Handle is NULL........exiting..\n");
                        pComponentPrivate->curState = OMX_StateInvalid;
                        eError = OMX_ErrorHardware;
                        pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                               pHandle->pApplicationPrivate,
                                                               OMX_EventError,
                                                               OMX_ErrorHardware,
                                                               OMX_TI_ErrorSevere,
                                                               "Lcml Handle NULL");
                        goto EXIT;
                    }

                    pLcmlDsp = (((LCML_DSP_INTERFACE*)pLcmlHandle)->dspCodec);
                    eError = MP3DEC_Fill_LCMLInitParams(pHandle, pLcmlDsp,arr);
                    if(eError != OMX_ErrorNone) {
                        MP3DEC_EPRINT(":: Error returned from Fill_LCMLInitParams()\n");
                        pComponentPrivate->curState = OMX_StateInvalid;
                        pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                               pHandle->pApplicationPrivate,
                                                               OMX_EventError, 
                                                               eError,
                                                               OMX_TI_ErrorSevere, 
                                                               NULL);
                        goto EXIT;
                    }
                    pComponentPrivate->pLcmlHandle = (LCML_DSP_INTERFACE *)pLcmlHandle;
                    cb.LCML_Callback = (void *) MP3DEC_LCML_Callback;

#ifndef UNDER_CE
                    eError = LCML_InitMMCodecEx(((LCML_DSP_INTERFACE *)pLcmlHandle)->pCodecinterfacehandle,
                                                p,&pLcmlHandle,(void *)p,&cb, (OMX_STRING)pComponentPrivate->sDeviceString);
                    if (eError != OMX_ErrorNone){
                        MP3DEC_EPRINT("%d :: Error : InitMMCodec failed...>>>>>> \n",__LINE__);
                        goto EXIT;
                    }
#else
                    eError = LCML_InitMMCodec(((LCML_DSP_INTERFACE *)pLcmlHandle)->pCodecinterfacehandle,
                                              p,&pLcmlHandle,(void *)p,&cb);
                    if (eError != OMX_ErrorNone){
                        MP3DEC_EPRINT("%d :: Error : InitMMCodec failed...>>>>>> \n",__LINE__);
                        goto EXIT;
                    }

#endif
#ifdef HASHINGENABLE
                    /* Enable the Hashing Code */
                    eError = LCML_SetHashingState(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle, OMX_TRUE);
                    if (eError != OMX_ErrorNone) {
                        MP3DEC_EPRINT("Failed to set Mapping State\n");
                        goto EXIT;
                    }
#endif

#ifdef RESOURCE_MANAGER_ENABLED
                    /* Need check the resource with RM */
                    pComponentPrivate->rmproxyCallback.RMPROXY_Callback = 
                        (void *) MP3_ResourceManagerCallback;
                    if (pComponentPrivate->curState != OMX_StateWaitForResources){
                        rm_error = RMProxy_NewSendCommand(pHandle, 
                                                          RMProxy_RequestResource, 
                                                          OMX_MP3_Decoder_COMPONENT,
                                                          MP3_CPU,
                                                          3456,
                                                          &(pComponentPrivate->rmproxyCallback));
                        if(rm_error == OMX_ErrorNone) {
                            /* resource is available */
#ifdef __PERF_INSTRUMENTATION__
                            PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundarySetup);
#endif   
                            pComponentPrivate->curState = OMX_StateIdle;
                            rm_error = RMProxy_NewSendCommand(pHandle,
                                                              RMProxy_StateSet,
                                                              OMX_MP3_Decoder_COMPONENT,
                                                              OMX_StateIdle,
                                                              3456,
                                                              NULL);
                            pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                                   pHandle->pApplicationPrivate,
                                                                   OMX_EventCmdComplete, 
                                                                   OMX_CommandStateSet,
                                                                   pComponentPrivate->curState,
                                                                   NULL);
                        }
                        else if(rm_error == OMX_ErrorInsufficientResources) {
                            /* resource is not available, need set state to 
                               OMX_StateWaitForResources */
                            pComponentPrivate->curState = OMX_StateWaitForResources;
                            pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                                   pHandle->pApplicationPrivate,
                                                                   OMX_EventCmdComplete,
                                                                   OMX_CommandStateSet,
                                                                   pComponentPrivate->curState,
                                                                   NULL);
                        }
                    }else{
                        rm_error = RMProxy_NewSendCommand(pHandle,
                                                          RMProxy_StateSet,
                                                          OMX_MP3_Decoder_COMPONENT,
                                                          OMX_StateIdle,
                                                          3456,
                                                          NULL);
                       
                        pComponentPrivate->curState = OMX_StateIdle;
                        pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                               pHandle->pApplicationPrivate,
                                                               OMX_EventCmdComplete, 
                                                               OMX_CommandStateSet,
                                                               pComponentPrivate->curState,
                                                               NULL);
                    }

#else
#ifdef __PERF_INSTRUMENTATION__
                    PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundarySetup);
#endif
                    pComponentPrivate->curState = OMX_StateIdle;
                    pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                           pHandle->pApplicationPrivate,
                                                           OMX_EventCmdComplete, 
                                                           OMX_CommandStateSet,
                                                           pComponentPrivate->curState,
                                                           NULL);
#endif

                    MP3DEC_DPRINT(":: Control Came Here\n");
                    MP3DEC_STATEPRINT("****************** Component State Set to Idle\n\n");
                    MP3DEC_DPRINT("MP3DEC: State has been Set to Idle\n");
                } else if (pComponentPrivate->curState == OMX_StateExecuting){
#ifdef __PERF_INSTRUMENTATION__
                    PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundarySteadyState);
#endif          
                    pComponentPrivate->bDspStoppedWhileExecuting = OMX_TRUE;                    
                    MP3DEC_DPRINT(":: In HandleCommand: Stopping the codec\n");
                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               MMCodecControlStop,(void *)pArgs);
                    if(eError != OMX_ErrorNone) {
                        MP3DEC_EPRINT(": Error Occurred in Codec Stop..\n");
                        pComponentPrivate->curState = OMX_StateInvalid;
                        pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                               pHandle->pApplicationPrivate,
                                                               OMX_EventError, 
                                                               eError,
                                                               OMX_TI_ErrorSevere, 
                                                               NULL);
                        goto EXIT;
                    }
#ifdef HASHINGENABLE
                    /*Hashing Change*/
                    pLcmlHandle = (LCML_DSP_INTERFACE*)pComponentPrivate->pLcmlHandle;
                    eError = LCML_FlushHashes(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle);
                    if (eError != OMX_ErrorNone) {
                        MP3DEC_EPRINT("Error occurred in Codec mapping flush!\n");
                        break;
                    }
#endif
                } else if(pComponentPrivate->curState == OMX_StatePause) {
                    char *pArgs = "damedesuStr";
                    MP3DEC_DPRINT(":: Comp: Stop Command Received\n");
#ifdef HASHINGENABLE
                    /*Hashing Change*/
                    pLcmlHandle = (LCML_DSP_INTERFACE*)pComponentPrivate->pLcmlHandle;
                    eError = LCML_FlushHashes(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle);
                    if (eError != OMX_ErrorNone) {
                        MP3DEC_EPRINT("Error occurred in Codec mapping flush!\n");
                        break;
                    }
#endif
#ifdef __PERF_INSTRUMENTATION__
                    PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundarySteadyState);
#endif              
                    MP3DEC_DPRINT(": MP3DECUTILS::About to call LCML_ControlCodec\n");
                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               MMCodecControlStop,(void *)pArgs);
                    if(eError != OMX_ErrorNone) {
                        MP3DEC_EPRINT(": Error Occurred in Codec Stop..\n");
                        pComponentPrivate->curState = OMX_StateInvalid;
                        pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                               pHandle->pApplicationPrivate,
                                                               OMX_EventError, 
                                                               eError,
                                                               OMX_TI_ErrorSevere, 
                                                               NULL);
                        goto EXIT;
                    }
                    MP3DEC_STATEPRINT("****************** Component State Set to Idle\n\n");
                    pComponentPrivate->curState = OMX_StateIdle;
#ifdef RESOURCE_MANAGER_ENABLED
                    rm_error = RMProxy_NewSendCommand(pHandle, 
                                                      RMProxy_StateSet, 
                                                      OMX_MP3_Decoder_COMPONENT, 
                                                      OMX_StateIdle, 
                                                      3456, 
                                                      NULL);
#endif                  
                    MP3DEC_DPRINT (":: The component is stopped\n");
                    pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                           pHandle->pApplicationPrivate,
                                                           OMX_EventCmdComplete, 
                                                           OMX_CommandStateSet, 
                                                           pComponentPrivate->curState, 
                                                           NULL);

                } else {
                    MP3DEC_DPRINT(": Comp: Sending ErrorNotification: Invalid State\n");
                    pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                           pHandle->pApplicationPrivate,
                                                           OMX_EventError,
                                                           OMX_ErrorIncorrectStateTransition, 
                                                           OMX_TI_ErrorMinor,
                                                           "Invalid State Error");
                }
                break;

            case OMX_StateExecuting:
                MP3DEC_DPRINT(": HandleCommand: Cmd Executing \n");
                if (pComponentPrivate->curState == OMX_StateIdle) {
                    char *pArgs = "damedesuStr";
                    OMX_U32 pValues[4];
                    OMX_U32 pValues1[4];

                    pComponentPrivate->SendAfterEOS = 0;  

                    if(pComponentPrivate->dasfmode == 1) {
                        pComponentPrivate->pParams->unAudioFormat = (unsigned short)pComponentPrivate->mp3Params->nChannels;
                        if (pComponentPrivate->pParams->unAudioFormat == MP3D_STEREO_STREAM) {
                            pComponentPrivate->pParams->unAudioFormat = MP3D_STEREO_NONINTERLEAVED_STREAM;
                        }

                        pComponentPrivate->pParams->ulSamplingFreq = pComponentPrivate->mp3Params->nSampleRate;
                        pComponentPrivate->pParams->unUUID = pComponentPrivate->streamID;

                        MP3DEC_DPRINT("::pParams->unAudioFormat   = %ld\n",pComponentPrivate->mp3Params->nChannels);
                        MP3DEC_DPRINT("::pParams->ulSamplingFreq  = %ld\n",pComponentPrivate->mp3Params->nSampleRate);

                        pValues[0] = USN_STRMCMD_SETCODECPARAMS;
                        pValues[1] = (OMX_U32)pComponentPrivate->pParams;
                        pValues[2] = sizeof(USN_AudioCodecParams);
                        eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                   EMMCodecControlStrmCtrl,(void *)pValues);
                        if(eError != OMX_ErrorNone) {
                            MP3DEC_EPRINT(": Error Occurred in Codec StreamControl..\n");
                            pComponentPrivate->curState = OMX_StateInvalid;
                            pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                                   pHandle->pApplicationPrivate,
                                                                   OMX_EventError, 
                                                                   eError,
                                                                   OMX_TI_ErrorSevere, 
                                                                   NULL);
                            goto EXIT;
                        }
                    }
                    if(pComponentPrivate->dasfmode == 0 && 
                       pComponentPrivate->pcmParams->bInterleaved) {
                        pComponentPrivate->ptAlgDynParams->lOutputFormat  = IAUDIO_INTERLEAVED;
                    } else {
                        pComponentPrivate->ptAlgDynParams->lOutputFormat  = IAUDIO_BLOCK;
                    }
                    
                    pComponentPrivate->ptAlgDynParams->lMonoToStereoCopy = 0;
                    pComponentPrivate->ptAlgDynParams->lStereoToMonoCopy = 0;
                    pComponentPrivate->ptAlgDynParams->size = sizeof(MP3DEC_UALGParams);
                
                    pValues1[0] = IUALG_CMD_SETSTATUS;
                    pValues1[1] = (OMX_U32) pComponentPrivate->ptAlgDynParams;
                    pValues1[2] = sizeof(MP3DEC_UALGParams);

                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               EMMCodecControlAlgCtrl,(void *)pValues1);
                    if(eError != OMX_ErrorNone) {
                        MP3DEC_EPRINT("Error Occurred in Codec Start..\n");
                        pComponentPrivate->curState = OMX_StateInvalid;
                        pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                               pHandle->pApplicationPrivate,
                                                               OMX_EventError, 
                                                               eError,
                                                               OMX_TI_ErrorSevere, 
                                                               NULL);
                        goto EXIT;
                    }
                    pComponentPrivate->bDspStoppedWhileExecuting = OMX_FALSE;
                    MP3DEC_DPRINT(":: Algcontrol has been sent to DSP\n");
                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               EMMCodecControlStart,(void *)pArgs);
                    if(eError != OMX_ErrorNone) {
                        MP3DEC_EPRINT("%d: Error Occurred in Codec Start..\n", __LINE__);
                        goto EXIT;
                    }
                    MP3DEC_DPRINT(": Codec Has Been Started \n");
                } else if (pComponentPrivate->curState == OMX_StatePause) {
                    char *pArgs = "damedesuStr";
                    MP3DEC_DPRINT(": Comp: Resume Command Came from App\n");
                    MP3DEC_DPRINT(": MP3DECUTILS::About to call LCML_ControlCodec\n");
                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               EMMCodecControlStart,(void *)pArgs);

                    if (eError != OMX_ErrorNone) {
                        MP3DEC_EPRINT ("Error While Resuming the codec\n");
                        pComponentPrivate->curState = OMX_StateInvalid;
                        pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                               pHandle->pApplicationPrivate,
                                                               OMX_EventError, 
                                                               eError,
                                                               OMX_TI_ErrorSevere, 
                                                               NULL);
                        goto EXIT;
                    }

                    for (i=0; i < pComponentPrivate->nNumInputBufPending; i++) {
                        if (pComponentPrivate->pInputBufHdrPending[i] != NULL) {
                            MP3D_LCML_BUFHEADERTYPE *pLcmlHdr;
                            MP3DEC_GetCorresponding_LCMLHeader(pComponentPrivate, 
                                                               pComponentPrivate->pInputBufHdrPending[i]->pBuffer, OMX_DirInput, &pLcmlHdr);
                            MP3DEC_SetPending(pComponentPrivate,pComponentPrivate->pInputBufHdrPending[i],OMX_DirInput,__LINE__);
                            eError = LCML_QueueBuffer(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                      EMMCodecInputBuffer,
                                                      pComponentPrivate->pInputBufHdrPending[i]->pBuffer,
                                                      pComponentPrivate->pInputBufHdrPending[i]->nAllocLen,
                                                      pComponentPrivate->pInputBufHdrPending[i]->nFilledLen,
                                                      (OMX_U8 *) pLcmlHdr->pIpParam,
                                                      sizeof(MP3DEC_UAlgInBufParamStruct),
                                                      NULL);
                        }
                    }
                    pComponentPrivate->nNumInputBufPending = 0;

                    for (i=0; i < pComponentPrivate->nNumOutputBufPending; i++) {
                        if (pComponentPrivate->pOutputBufHdrPending[i]) {
                            MP3D_LCML_BUFHEADERTYPE *pLcmlHdr;
                            MP3DEC_GetCorresponding_LCMLHeader(pComponentPrivate, 
                                                               pComponentPrivate->pOutputBufHdrPending[i]->pBuffer, 
                                                               OMX_DirOutput, 
                                                               &pLcmlHdr);
                            MP3DEC_SetPending(pComponentPrivate,
                                              pComponentPrivate->pOutputBufHdrPending[i],
                                              OMX_DirOutput,
                                              __LINE__);
                            eError = LCML_QueueBuffer(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                                      EMMCodecOuputBuffer,
                                                      pComponentPrivate->pOutputBufHdrPending[i]->pBuffer,
                                                      pComponentPrivate->pOutputBufHdrPending[i]->nAllocLen,
                                                      0,
                                                      (OMX_U8 *) pLcmlHdr->pOpParam,
                                                      sizeof(MP3DEC_UAlgOutBufParamStruct),
                                                      NULL);
                        }
                    }
                    pComponentPrivate->nNumOutputBufPending = 0;
                }else {
                    pComponentPrivate->cbInfo.EventHandler (pHandle, 
                                                            pHandle->pApplicationPrivate,
                                                            OMX_EventError, 
                                                            OMX_ErrorIncorrectStateTransition, 
                                                            OMX_TI_ErrorMinor,
                                                            "Invalid State");
                    MP3DEC_EPRINT(":: Error: Invalid State Given by Application\n");
                    goto EXIT;
                }

                MP3DEC_STATEPRINT("****************** Component State Set to Executing\n\n");
#ifdef RESOURCE_MANAGER_ENABLED         
                /*rm_error = RMProxy_SendCommand(pHandle, RMProxy_StateSet, OMX_MP3_Decoder_COMPONENT, OMX_StateExecuting, NULL);*/
                rm_error = RMProxy_NewSendCommand(pHandle, RMProxy_StateSet, OMX_MP3_Decoder_COMPONENT, OMX_StateExecuting, 3456, NULL);
#endif          
                pComponentPrivate->curState = OMX_StateExecuting;
#ifdef __PERF_INSTRUMENTATION__
                PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryStart | PERF_BoundarySteadyState);
#endif          
                pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                       pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete,
                                                       OMX_CommandStateSet, 
                                                       pComponentPrivate->curState, 
                                                       NULL);

                break;

            case OMX_StateLoaded:
                MP3DEC_DPRINT(": HandleCommand: Cmd Loaded\n");

                if (pComponentPrivate->curState == OMX_StateWaitForResources ){
                    MP3DEC_STATEPRINT("****************** Component State Set to Loaded\n\n");
#ifdef __PERF_INSTRUMENTATION__
                    PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryStart | PERF_BoundaryCleanup); 
#endif              
                    pComponentPrivate->curState = OMX_StateLoaded;
#ifdef __PERF_INSTRUMENTATION__
                    PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundaryCleanup);
#endif              
                    pComponentPrivate->cbInfo.EventHandler (pHandle, 
                                                            pHandle->pApplicationPrivate,
                                                            OMX_EventCmdComplete, 
                                                            OMX_CommandStateSet,
                                                            pComponentPrivate->curState,
                                                            NULL);
                    MP3DEC_DPRINT(":: Transitioning from WaitFor to Loaded\n");
                    break;
                }

                if (pComponentPrivate->curState != OMX_StateIdle) {
                    pComponentPrivate->cbInfo.EventHandler (pHandle, 
                                                            pHandle->pApplicationPrivate,
                                                            OMX_EventError, 
                                                            OMX_ErrorIncorrectStateTransition, 
                                                            OMX_TI_ErrorMinor,
                                                            "Invalid State");
                    MP3DEC_EPRINT(":: Error: Invalid State Given by \
                       Application\n");
                    goto EXIT;
                }
#ifdef __PERF_INSTRUMENTATION__
                PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryStart | PERF_BoundaryCleanup);
#endif          

                MP3DEC_DPRINT("Current State = %d\n",pComponentPrivate->curState);
                MP3DEC_DPRINT("pComponentPrivate->pInputBufferList->numBuffers = %ld\n",pComponentPrivate->pInputBufferList->numBuffers);
                MP3DEC_DPRINT("pComponentPrivate->pOutputBufferList->numBuffers = %ld\n",pComponentPrivate->pOutputBufferList->numBuffers);

                if (pComponentPrivate->pInputBufferList->numBuffers || pComponentPrivate->pOutputBufferList->numBuffers) {
                    pComponentPrivate->InIdle_goingtoloaded = 1;
#ifndef UNDER_CE
                    pthread_mutex_lock(&pComponentPrivate->InIdle_mutex); 
                    pthread_cond_wait(&pComponentPrivate->InIdle_threshold, &pComponentPrivate->InIdle_mutex);
                    pthread_mutex_unlock(&pComponentPrivate->InIdle_mutex);
#else
                    OMX_WaitForEvent(&(pComponentPrivate->InIdle_event));
#endif
                    /* Send StateChangeNotification to application */
                    pComponentPrivate->bLoadedCommandPending = OMX_FALSE;
                }
                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                           EMMCodecControlDestroy,(void *)pArgs);
#ifdef UNDER_CE
                FreeLibrary(g_hLcmlDllHandle);
                g_hLcmlDllHandle = NULL;
#endif
#ifdef __PERF_INSTRUMENTATION__
                PERF_SendingCommand(pComponentPrivate->pPERF, -1, 0, PERF_ModuleComponent);
#endif        
                eError = EXIT_COMPONENT_THRD;
                pComponentPrivate->bInitParamsInitialized = 0;
                break;

            case OMX_StatePause:
                MP3DEC_DPRINT(": HandleCommand: Cmd Pause: Cur State = %d\n",
                              pComponentPrivate->curState);

                if ((pComponentPrivate->curState != OMX_StateExecuting) &&
                    (pComponentPrivate->curState != OMX_StateIdle)) {
                    pComponentPrivate->cbInfo.EventHandler (pHandle, 
                                                            pHandle->pApplicationPrivate,
                                                            OMX_EventError, 
                                                            OMX_ErrorIncorrectStateTransition, 
                                                            OMX_TI_ErrorMinor,
                                                            "Invalid State");
                    MP3DEC_EPRINT(":: Error: Invalid State Given by \
                       Application\n");
                    goto EXIT;
                }
#ifdef __PERF_INSTRUMENTATION__
                PERF_Boundary(pComponentPrivate->pPERFcomp,PERF_BoundaryComplete | PERF_BoundarySteadyState);
#endif
                MP3DEC_DPRINT(": MP3DECUTILS::About to call LCML_ControlCodec\n");
                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                           EMMCodecControlPause,(void *)pArgs);
                if (eError != OMX_ErrorNone) {
                    MP3DEC_EPRINT(": Error: in Pausing the codec\n");
                    pComponentPrivate->curState = OMX_StateInvalid;
                    pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                           pHandle->pApplicationPrivate,
                                                           OMX_EventError, 
                                                           eError,
                                                           OMX_TI_ErrorSevere, 
                                                           NULL);
                    goto EXIT;
                }
                MP3DEC_STATEPRINT("****************** Component State Set to Pause\n\n");
#if 0
                pComponentPrivate->curState = OMX_StatePause;
                pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                       pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete, 
                                                       OMX_CommandStateSet,
                                                       pComponentPrivate->curState, 
                                                       NULL);
#endif
                break;

            case OMX_StateWaitForResources:
                MP3DEC_DPRINT(": HandleCommand: Cmd : OMX_StateWaitForResources\n");
                if (pComponentPrivate->curState == OMX_StateLoaded) {
#ifdef RESOURCE_MANAGER_ENABLED         
                    rm_error = RMProxy_NewSendCommand(pHandle, 
                                                      RMProxy_StateSet, 
                                                      OMX_MP3_Decoder_COMPONENT, 
                                                      OMX_StateWaitForResources, 
                                                      3456, 
                                                      NULL);
#endif  

                    pComponentPrivate->curState = OMX_StateWaitForResources;
                    MP3DEC_DPRINT(": Transitioning from Loaded to OMX_StateWaitForResources\n");
                    pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                           pHandle->pApplicationPrivate,
                                                           OMX_EventCmdComplete,
                                                           OMX_CommandStateSet,
                                                           pComponentPrivate->curState, 
                                                           NULL);
                } else {
                    pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                           pHandle->pApplicationPrivate,
                                                           OMX_EventError, 
                                                           OMX_ErrorIncorrectStateTransition,
                                                           OMX_TI_ErrorMinor, 
                                                           NULL);
                }
                break;

            case OMX_StateInvalid:
                MP3DEC_DPRINT(": HandleCommand: Cmd OMX_StateInvalid:\n");
                if (pComponentPrivate->curState != OMX_StateWaitForResources && 
                    pComponentPrivate->curState != OMX_StateLoaded &&
                    pComponentPrivate->curState != OMX_StateInvalid ) {

                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               EMMCodecControlDestroy, (void *)pArgs);
                }

                pComponentPrivate->curState = OMX_StateInvalid;
                pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                       pHandle->pApplicationPrivate,
                                                       OMX_EventError, 
                                                       OMX_ErrorInvalidState,
                                                       OMX_TI_ErrorSevere,
                                                       NULL);
                MP3DEC_CleanupInitParams(pHandle);

                break;

            case OMX_StateMax:
                MP3DEC_DPRINT(": HandleCommand: Cmd OMX_StateMax::\n");
                break;
            } /* End of Switch */
        }
    }
    else if (command == OMX_CommandMarkBuffer) {
        MP3DEC_DPRINT("command OMX_CommandMarkBuffer received\n");
        if(!pComponentPrivate->pMarkBuf){
            MP3DEC_DPRINT("command OMX_CommandMarkBuffer received \n");
            pComponentPrivate->pMarkBuf = (OMX_MARKTYPE *)(commandData);
        }
    } else if (command == OMX_CommandPortDisable) {
        if (!pComponentPrivate->bDisableCommandPending) {
            if(commandData == 0x0){
                /* disable port */
                for (i=0; i < pComponentPrivate->pInputBufferList->numBuffers; i++) {
                    MP3DEC_DPRINT("pComponentPrivate->pInputBufferList->bBufferPending[%ld] = %ld\n",i,
                                  pComponentPrivate->pInputBufferList->bBufferPending[i]);
                    if (MP3DEC_IsPending(pComponentPrivate,pComponentPrivate->pInputBufferList->pBufHdr[i],OMX_DirInput)) {
                        /* Real solution is flush buffers from DSP.  Until we have the ability to do that 
                           we just call EmptyBufferDone() on any pending buffers */
                        MP3DEC_DPRINT("Forcing EmptyBufferDone\n");
#ifdef __PERF_INSTRUMENTATION__
                        PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                                          PREF(pComponentPrivate->pInputBufferList->pBufHdr[i], pBuffer),
                                          0,
                                          PERF_ModuleHLMM);
#endif                  
                        pComponentPrivate->cbInfo.EmptyBufferDone (pComponentPrivate->pHandle,
                                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                                   pComponentPrivate->pInputBufferList->pBufHdr[i]);
                        pComponentPrivate->nEmptyBufferDoneCount++;
                    }
                }

                pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bEnabled = OMX_FALSE;
            }
            if(commandData == -1){
                /* disable port */
                pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bEnabled = OMX_FALSE;
            }
            if(commandData == 0x1 || commandData == -1){
                char *pArgs = "damedesuStr";
                pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bEnabled = OMX_FALSE;
                if (pComponentPrivate->curState == OMX_StateExecuting) {
                    pComponentPrivate->bNoIdleOnStop = OMX_TRUE;
                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               MMCodecControlStop,(void *)pArgs);
                }
            }
        }
        MP3DEC_DPRINT("commandData = %ld\n",commandData);
        MP3DEC_DPRINT("pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bPopulated = %d\n",
                      pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bPopulated);
        MP3DEC_DPRINT("pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bPopulated = %d\n",
                      pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bPopulated);
        if(commandData == 0x0) {
            if(!pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bPopulated){
                /* return cmdcomplete event if input unpopulated */ 
                pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                       pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete, 
                                                       OMX_CommandPortDisable,
                                                       MP3D_INPUT_PORT, 
                                                       NULL);
                pComponentPrivate->bDisableCommandPending = 0;
            }
            else{
                pComponentPrivate->bDisableCommandPending = 1;
                pComponentPrivate->bDisableCommandParam = commandData;
            }
        }

        if(commandData == 0x1) {
            if (!pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bPopulated){
                /* return cmdcomplete event if output unpopulated */ 
                pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                       pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete, 
                                                       OMX_CommandPortDisable,
                                                       MP3D_OUTPUT_PORT, 
                                                       NULL);
                pComponentPrivate->bDisableCommandPending = 0;
            }
            else {
                pComponentPrivate->bDisableCommandPending = 1;
                pComponentPrivate->bDisableCommandParam = commandData;
            }
        }

        if(commandData == -1) {
            if (!pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bPopulated && 
                !pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bPopulated){

                /* return cmdcomplete event if inout & output unpopulated */ 
                pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                       pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete, 
                                                       OMX_CommandPortDisable,
                                                       MP3D_INPUT_PORT, 
                                                       NULL);

                pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                       pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete, 
                                                       OMX_CommandPortDisable,
                                                       MP3D_OUTPUT_PORT, 
                                                       NULL);
                pComponentPrivate->bDisableCommandPending = 0;
            }
            else {
                pComponentPrivate->bDisableCommandPending = 1;
                pComponentPrivate->bDisableCommandParam = commandData;
            }
        }
    }
    else if (command == OMX_CommandPortEnable) {
        if (!pComponentPrivate->bEnableCommandPending) {
            if(commandData == 0x0 || commandData == -1){
                /* enable in port */
                MP3DEC_DPRINT("setting input port to enabled\n");
                pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bEnabled = OMX_TRUE;
                MP3DEC_DPRINT("WAKE UP!! HandleCommand: En utils setting output port to enabled. \n");
                if(pComponentPrivate->AlloBuf_waitingsignal){
                    pComponentPrivate->AlloBuf_waitingsignal = 0;
                }
                MP3DEC_DPRINT("pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bEnabled = %d\n",
                              pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bEnabled);
            }
            if(commandData == 0x1 || commandData == -1){
                char *pArgs = "damedesuStr";
                /* enable out port */
                MP3DEC_DPRINT("pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bEnabled = %d\n",
                              pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bEnabled);
                if(pComponentPrivate->AlloBuf_waitingsignal){
                    pComponentPrivate->AlloBuf_waitingsignal = 0;
#ifndef UNDER_CE             
                    pthread_mutex_lock(&pComponentPrivate->AlloBuf_mutex); 
                    pthread_cond_signal(&pComponentPrivate->AlloBuf_threshold);
                    pthread_mutex_unlock(&pComponentPrivate->AlloBuf_mutex);    
#else
                    OMX_SignalEvent(&(pComponentPrivate->AlloBuf_event));
#endif            
                }
                if(pComponentPrivate->curState == OMX_StateExecuting){
                    pComponentPrivate->bDspStoppedWhileExecuting = OMX_FALSE;
                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               EMMCodecControlStart,(void *)pArgs);
                }
                pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bEnabled = OMX_TRUE;
                MP3DEC_DPRINT("setting output port to enabled\n");
            }
        }
        if(commandData == 0x0){
            if (pComponentPrivate->curState == OMX_StateLoaded || 
                pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bPopulated) {
                pComponentPrivate->cbInfo.EventHandler(pHandle,
                                                       pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete,
                                                       OMX_CommandPortEnable,
                                                       MP3D_INPUT_PORT,
                                                       NULL);
                pComponentPrivate->bEnableCommandPending = 0;
            }
            else {
                pComponentPrivate->bEnableCommandPending = 1;
                pComponentPrivate->bEnableCommandParam = commandData;
            }
        }
        else if(commandData == 0x1) {
            if (pComponentPrivate->curState == OMX_StateLoaded || 
                pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bPopulated){
                pComponentPrivate->cbInfo.EventHandler( pHandle, 
                                                        pHandle->pApplicationPrivate,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandPortEnable,
                                                        MP3D_OUTPUT_PORT, 
                                                        NULL);
                pComponentPrivate->bEnableCommandPending = 0;
            }
            else {
                pComponentPrivate->bEnableCommandPending = 1;
                pComponentPrivate->bEnableCommandParam = commandData;
            }
        }
        else if(commandData == -1) {
            if (pComponentPrivate->curState == OMX_StateLoaded || 
                (pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->bPopulated
                 && pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->bPopulated)){
                pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                       pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete, 
                                                       OMX_CommandPortEnable,
                                                       MP3D_INPUT_PORT, 
                                                       NULL);
                pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                       pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete, 
                                                       OMX_CommandPortEnable,
                                                       MP3D_OUTPUT_PORT, 
                                                       NULL);
                pComponentPrivate->bEnableCommandPending = 0;
                MP3DECFill_LCMLInitParamsEx(pHandle);
            }
            else {
                pComponentPrivate->bEnableCommandPending = 1;
                pComponentPrivate->bEnableCommandParam = commandData;
            }
        }
#ifndef UNDER_CE
        pthread_mutex_lock(&pComponentPrivate->AlloBuf_mutex); 
        pthread_cond_signal(&pComponentPrivate->AlloBuf_threshold);
        pthread_mutex_unlock(&pComponentPrivate->AlloBuf_mutex);    
#else
        OMX_SignalEvent(&(pComponentPrivate->AlloBuf_event));
#endif
    }
    else if (command == OMX_CommandFlush) {
        OMX_U32 aParam[3] = {0};
        if(commandData == 0x0 || commandData == -1) {
            if (pComponentPrivate->nUnhandledEmptyThisBuffers == 0)  {
                pComponentPrivate->bFlushInputPortCommandPending = OMX_FALSE;
                pComponentPrivate->first_buff = 0;

                aParam[0] = USN_STRMCMD_FLUSH; 
                aParam[1] = 0x0; 
                aParam[2] = 0x0; 

                MP3DEC_DPRINT("Flushing input port\n");
                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                           EMMCodecControlStrmCtrl, (void*)aParam);
         
                if (eError != OMX_ErrorNone) {
                    goto EXIT;
                }
            }else {
                pComponentPrivate->bFlushInputPortCommandPending = OMX_TRUE;
            }
        }
        if(commandData == 0x1 || commandData == -1){
            if (pComponentPrivate->nUnhandledFillThisBuffers == 0)  {
                pComponentPrivate->bFlushOutputPortCommandPending = OMX_FALSE;
                pComponentPrivate->first_buff = 0;

                aParam[0] = USN_STRMCMD_FLUSH; 
                aParam[1] = 0x1; 
                aParam[2] = 0x0; 

                MP3DEC_DPRINT("Flushing output port\n");
                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                           EMMCodecControlStrmCtrl, (void*)aParam);
        
                if (eError != OMX_ErrorNone) {
                    goto EXIT;
                }
            } else {
                pComponentPrivate->bFlushOutputPortCommandPending = OMX_TRUE; 
            }
        }
    }
 EXIT:
    MP3DEC_DPRINT (":: Exiting HandleCommand Function, error = %d\n", eError);
    return eError;
}


/* ================================================================================= * */
/**
 * @fn MP3DEC_HandleDataBuf_FromApp() function handles the input and output buffers
 * that come from the application. It is not direct function wich gets called by
 * the application rather, it gets called eventually.
 *
 * @param *pBufHeader This is the buffer header that needs to be processed.
 *
 * @param *pComponentPrivate  This is component's private date structure.
 *
 * @pre          None
 *
 * @post         None
 *
 *  @return      OMX_ErrorNone = Successful processing.
 *               OMX_ErrorInsufficientResources = Not enough memory
 *               OMX_ErrorHardware = Hardware error has occured lile LCML failed
 *               to do any said operartion.
 *
 *  @see         None
 */
/* ================================================================================ * */

OMX_ERRORTYPE MP3DEC_HandleDataBuf_FromApp(OMX_BUFFERHEADERTYPE* pBufHeader,
                                           MP3DEC_COMPONENT_PRIVATE *pComponentPrivate)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_DIRTYPE eDir;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDefIn = NULL;
    char *pArgs = "damedesuStr";
    OMX_U32 pValues[4];
    OMX_U32 pValues1[4];

    MP3DEC_DPRINT (":: Entering HandleDataBuf_FromApp Function\n");
    MP3DEC_DPRINT (":: pBufHeader->pMarkData = %p\n",pBufHeader->pMarkData);

    pBufHeader->pPlatformPrivate  = pComponentPrivate;
    eError = MP3DEC_GetBufferDirection(pBufHeader, &eDir);
    MP3DEC_DPRINT (":: HandleDataBuf_FromApp Function\n");
    if (eError != OMX_ErrorNone) {
        MP3DEC_EPRINT (":: The pBufHeader is not found in the list\n");
        goto EXIT;
    }

    if (eDir == OMX_DirInput) {
        pComponentPrivate->nUnhandledEmptyThisBuffers--;
        LCML_DSP_INTERFACE *pLcmlHandle = (LCML_DSP_INTERFACE *)pComponentPrivate->pLcmlHandle;
        MP3D_LCML_BUFHEADERTYPE *pLcmlHdr;
        pPortDefIn = pComponentPrivate->pPortDef[OMX_DirInput];
        
        eError = MP3DEC_GetCorresponding_LCMLHeader(pComponentPrivate, pBufHeader->pBuffer, OMX_DirInput, &pLcmlHdr);
        if (eError != OMX_ErrorNone) {
            MP3DEC_EPRINT(":: Error: Invalid Buffer Came ...\n");
            goto EXIT;
        }

        MP3DEC_DPRINT(":: pBufHeader->nFilledLen = %ld\n",pBufHeader->nFilledLen);

        if ((pBufHeader->nFilledLen > 0) || (pBufHeader->nFlags == OMX_BUFFERFLAG_EOS)) {
            pComponentPrivate->bBypassDSP = 0;
            MP3DEC_DPRINT (":: HandleDataBuf_FromApp Function\n");
            MP3DEC_DPRINT (":::Calling LCML_QueueBuffer\n");
#ifdef __PERF_INSTRUMENTATION__
            /*For Steady State Instumentation*/
#if 0 
            if ((pComponentPrivate->nLcml_nCntIp == 1)) {
                PERF_Boundary(pComponentPrivate->pPERFcomp,
                              PERF_BoundaryStart | PERF_BoundarySteadyState);
            }
#endif
            PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                              PREF(pBufHeader,pBuffer),
                              pPortDefIn->nBufferSize, 
                              PERF_ModuleCommonLayer);
#endif
            if(pComponentPrivate->SendAfterEOS){
                if(pComponentPrivate->dasfmode == 1) {
                    pComponentPrivate->pParams->unAudioFormat = 
                        (unsigned short)pComponentPrivate->mp3Params->nChannels;
                    if (pComponentPrivate->pParams->unAudioFormat == MP3D_STEREO_STREAM) {
                        pComponentPrivate->pParams->unAudioFormat = MP3D_STEREO_NONINTERLEAVED_STREAM;
                    }

                    pComponentPrivate->pParams->ulSamplingFreq = 
                        pComponentPrivate->mp3Params->nSampleRate;
                    pComponentPrivate->pParams->unUUID = pComponentPrivate->streamID;

                    pValues[0] = USN_STRMCMD_SETCODECPARAMS;
                    pValues[1] = (OMX_U32)pComponentPrivate->pParams;
                    pValues[2] = sizeof(USN_AudioCodecParams);
                    eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                               EMMCodecControlStrmCtrl,(void *)pValues);
                    if(eError != OMX_ErrorNone) {
                        MP3DEC_EPRINT(": Error Occurred in Codec StreamControl..\n");
                        pComponentPrivate->curState = OMX_StateInvalid;
                        pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle, 
                                                               pComponentPrivate->pHandle->pApplicationPrivate,
                                                               OMX_EventError, 
                                                               eError,
                                                               OMX_TI_ErrorSevere, 
                                                               NULL);
                        goto EXIT;
                    }
                }
                if((pComponentPrivate->dasfmode == 0) && 
                   (pComponentPrivate->pcmParams->bInterleaved)) {
                    pComponentPrivate->ptAlgDynParams->lOutputFormat  = IAUDIO_INTERLEAVED;
                } else {
                    pComponentPrivate->ptAlgDynParams->lOutputFormat  = IAUDIO_BLOCK;
                }
                    
                pComponentPrivate->ptAlgDynParams->lMonoToStereoCopy = 0;
                pComponentPrivate->ptAlgDynParams->lStereoToMonoCopy = 0;
                pComponentPrivate->ptAlgDynParams->size = sizeof(MP3DEC_UALGParams);
                
                pValues1[0] = IUALG_CMD_SETSTATUS;
                pValues1[1] = (OMX_U32) pComponentPrivate->ptAlgDynParams;
                pValues1[2] = sizeof(MP3DEC_UALGParams);

                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                           EMMCodecControlAlgCtrl,(void *)pValues1);
                if(eError != OMX_ErrorNone) {
                    MP3DEC_EPRINT("Error Occurred in Codec Start..\n");
                    pComponentPrivate->curState = OMX_StateInvalid;
                    pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle, 
                                                           pComponentPrivate->pHandle->pApplicationPrivate,
                                                           OMX_EventError, 
                                                           eError,
                                                           OMX_TI_ErrorSevere, 
                                                           NULL);
                    goto EXIT;
                }
                pComponentPrivate->bDspStoppedWhileExecuting = OMX_FALSE;
                eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                           EMMCodecControlStart,(void *)pArgs);
                if(eError != OMX_ErrorNone) {
                    MP3DEC_EPRINT("%d: Error Occurred in Codec Start..\n", __LINE__);
                    goto EXIT;
                }

                pComponentPrivate->SendAfterEOS = 0;
                pComponentPrivate->first_buff = 0;
            }

            pLcmlHdr->pIpParam->bLastBuffer = 0;
            if(pBufHeader->nFlags == OMX_BUFFERFLAG_EOS) {
                MP3DEC_DPRINT(":: bLastBuffer Is Set Here....\n");
                pLcmlHdr->pIpParam->bLastBuffer = 1;
                pComponentPrivate->bIsEOFSent = 1;
                pComponentPrivate->SendAfterEOS = 1;
                pBufHeader->nFlags = 0;
            }

            /* Store time stamp information */
            pComponentPrivate->arrBufIndex[pComponentPrivate->IpBufindex] = pBufHeader->nTimeStamp;
            /*add on: Store tic count information*/
            pComponentPrivate->arrBufIndexTick[pComponentPrivate->IpBufindex] = pBufHeader->nTickCount;
            pComponentPrivate->IpBufindex++;
            pComponentPrivate->IpBufindex %= pPortDefIn->nBufferCountActual;
            
            if(!pComponentPrivate->frameMode){
                if(pComponentPrivate->first_buff == 0){
                    pComponentPrivate->first_buff = 1;
                    pComponentPrivate->first_TS = pBufHeader->nTimeStamp;
                }
            }

            MP3DEC_DPRINT ("Comp:: Sending Filled Input buffer = %p, %p\
                               to LCML\n",pBufHeader,pBufHeader->pBuffer);

            if (pComponentPrivate->curState == OMX_StateExecuting) {
                if (!MP3DEC_IsPending(pComponentPrivate,pBufHeader,OMX_DirInput)) {
                    if(!pComponentPrivate->bDspStoppedWhileExecuting) {
                        MP3DEC_SetPending(pComponentPrivate,pBufHeader,OMX_DirInput,__LINE__);
                        eError = LCML_QueueBuffer(pLcmlHandle->pCodecinterfacehandle,
                                                  EMMCodecInputBuffer,
                                                  pBufHeader->pBuffer,
                                                  pBufHeader->nAllocLen,
                                                  pBufHeader->nFilledLen,
                                                  (OMX_U8 *) pLcmlHdr->pIpParam,
                                                  sizeof(MP3DEC_UAlgInBufParamStruct),
                                                  NULL);
                        if (eError != OMX_ErrorNone) {
                            MP3DEC_DPRINT ("::Comp: SetBuff: IP: Error Occurred\n");
                            eError = OMX_ErrorHardware;
                            goto EXIT;
                        }
                        MP3DEC_DPRINT("%d \t\t\t\t............ Sent IP %p, len %ld\n",__LINE__ , 
                                      pBufHeader->pBuffer,
                                      pBufHeader->nFilledLen);
                    }
                    else {
                        MP3DEC_DPRINT("Calling EmptyBufferDone from line %d\n",__LINE__);
#ifdef __PERF_INSTRUMENTATION__
                        PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                                          PREF(pBufHeader, pBuffer),
                                          0,
                                          PERF_ModuleHLMM);
#endif
                        pComponentPrivate->cbInfo.EmptyBufferDone (
                                                                   pComponentPrivate->pHandle,
                                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                                   pBufHeader
                                                                   );
                    }
                    pComponentPrivate->lcml_nCntIp++;
                    pComponentPrivate->lcml_nIpBuf++;
                    pComponentPrivate->num_Sent_Ip_Buff++;
                    MP3DEC_DPRINT ("Sending Input buffer to Codec\n");
                }
            }
            else if (pComponentPrivate->curState == OMX_StatePause) {
                pComponentPrivate->pInputBufHdrPending[pComponentPrivate->nNumInputBufPending++] = pBufHeader;
            }
        }else {
            pComponentPrivate->bBypassDSP = 1;
            MP3DEC_DPRINT ("%d :: Forcing EmptyBufferDone\n",__LINE__);
#ifdef __PERF_INSTRUMENTATION__
            PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                              PREF(pComponentPrivate->pInputBufferList->pBufHdr[0], pBuffer),
                              0,
                              PERF_ModuleHLMM);
#endif
            pComponentPrivate->cbInfo.EmptyBufferDone (pComponentPrivate->pHandle,
                                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                                       pComponentPrivate->pInputBufferList->pBufHdr[0]);
            pComponentPrivate->nEmptyBufferDoneCount++;
        }
        if(pBufHeader->pMarkData){
            MP3DEC_DPRINT (":Detected pBufHeader->pMarkData\n");

            pComponentPrivate->pMarkData = pBufHeader->pMarkData;
            pComponentPrivate->hMarkTargetComponent = pBufHeader->hMarkTargetComponent;
            pComponentPrivate->pOutputBufferList->pBufHdr[0]->pMarkData = pBufHeader->pMarkData;
            pComponentPrivate->pOutputBufferList->pBufHdr[0]->hMarkTargetComponent = pBufHeader->hMarkTargetComponent;
        
            if(pBufHeader->hMarkTargetComponent == pComponentPrivate->pHandle && pBufHeader->pMarkData){
                pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                                       OMX_EventMark,
                                                       0,
                                                       0,
                                                       pBufHeader->pMarkData);
            }
        }
        if (pComponentPrivate->bFlushInputPortCommandPending) {
            OMX_SendCommand(pComponentPrivate->pHandle,OMX_CommandFlush,0,NULL);
        }
    }
    else if (eDir == OMX_DirOutput) {
        pComponentPrivate->nUnhandledFillThisBuffers--;
        LCML_DSP_INTERFACE *pLcmlHandle = (LCML_DSP_INTERFACE *)pComponentPrivate->pLcmlHandle;
        MP3D_LCML_BUFHEADERTYPE *pLcmlHdr;
        MP3DEC_DPRINT(": pComponentPrivate->lcml_nOpBuf = %ld\n",pComponentPrivate->lcml_nOpBuf);
        MP3DEC_DPRINT(": pComponentPrivate->lcml_nIpBuf = %ld\n",pComponentPrivate->lcml_nIpBuf);
        eError = MP3DEC_GetCorresponding_LCMLHeader(pComponentPrivate, pBufHeader->pBuffer, OMX_DirOutput, &pLcmlHdr);
        if (eError != OMX_ErrorNone) {
            MP3DEC_EPRINT(":: Error: Invalid Buffer Came ...\n");
            goto EXIT;
        }
        MP3DEC_DPRINT (":::Calling LCML_QueueBuffer\n");
#ifdef __PERF_INSTRUMENTATION__
        PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                          PREF(pBufHeader,pBuffer),
                          0,
                          PERF_ModuleCommonLayer);
#endif
        if (pComponentPrivate->bBypassDSP == 0) {
            if (pComponentPrivate->curState == OMX_StateExecuting) {
                if (!MP3DEC_IsPending(pComponentPrivate,pBufHeader,OMX_DirOutput) && 
                    (pComponentPrivate->numPendingBuffers < pComponentPrivate->pOutputBufferList->numBuffers)){
                    if(!pComponentPrivate->bDspStoppedWhileExecuting){  
                        MP3DEC_SetPending(pComponentPrivate,pBufHeader,OMX_DirOutput,__LINE__);
                        eError = LCML_QueueBuffer(pLcmlHandle->pCodecinterfacehandle,
                                                  EMMCodecOuputBuffer,
                                                  pBufHeader->pBuffer,
                                                  pBufHeader->nAllocLen,
                                                  0,
                                                  (OMX_U8 *) pLcmlHdr->pOpParam,
                                                  sizeof(MP3DEC_UAlgOutBufParamStruct),
                                                  pBufHeader->pBuffer);
                        if (eError != OMX_ErrorNone ) {
                            MP3DEC_EPRINT (":: Comp:: SetBuff OP: Error Occurred\n");
                            eError = OMX_ErrorHardware;
                            goto EXIT;
                        }
                        MP3DEC_DPRINT("%d \t\t\t\t............ Sent OP %p\n",__LINE__,pBufHeader->pBuffer);
                        pComponentPrivate->lcml_nCntOp++;
                        pComponentPrivate->lcml_nOpBuf++;
                        pComponentPrivate->num_Op_Issued++;
                    }
                }
            }else if (pComponentPrivate->curState == OMX_StatePause) {
                pComponentPrivate->pOutputBufHdrPending[pComponentPrivate->nNumOutputBufPending++] = pBufHeader;
            }
        }
        if (pComponentPrivate->bFlushOutputPortCommandPending) {
            OMX_SendCommand( pComponentPrivate->pHandle, OMX_CommandFlush, 1, NULL);
        }
    }
    else {
        MP3DEC_EPRINT(": BufferHeader %p, Buffer %p Unknown ..........\n",pBufHeader, pBufHeader->pBuffer);
        eError = OMX_ErrorBadParameter;
    }
 EXIT:
    MP3DEC_DPRINT(": Exiting from  HandleDataBuf_FromApp: %x \n",eError);
    if(eError == OMX_ErrorBadParameter) {
        MP3DEC_EPRINT(": Error = OMX_ErrorBadParameter\n");
    }
    return eError;
}


/* ================================================================================= * */
/**
 * @fn MP3DEC_GetBufferDirection() function determines whether it is input buffer or
 * output buffer.
 *
 * @param *pBufHeader This is pointer to buffer header whose direction needs to
 *                    be determined. 
 *
 * @param *eDir  This is output argument which stores the direction of buffer. 
 *
 * @pre          None
 *
 * @post         None
 *
 *  @return      OMX_ErrorNone = Successful processing.
 *               OMX_ErrorBadParameter = In case of invalid buffer
 *
 *  @see         None
 */
/* ================================================================================ * */

OMX_ERRORTYPE MP3DEC_GetBufferDirection(OMX_BUFFERHEADERTYPE *pBufHeader,
                                        OMX_DIRTYPE *eDir)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    MP3DEC_COMPONENT_PRIVATE *pComponentPrivate = pBufHeader->pPlatformPrivate;
    OMX_U32 nBuf = pComponentPrivate->pInputBufferList->numBuffers;
    OMX_BUFFERHEADERTYPE *pBuf = NULL;
    int flag = 1;
    OMX_U32 i=0;

    MP3DEC_DPRINT (":: Entering GetBufferDirection Function\n");
    for(i=0; i<nBuf; i++) {
        pBuf = pComponentPrivate->pInputBufferList->pBufHdr[i];
        if(pBufHeader == pBuf) {
            *eDir = OMX_DirInput;
            MP3DEC_DPRINT (":: Buffer %p is INPUT BUFFER\n", pBufHeader);
            flag = 0;
            goto EXIT;
        }
    }

    nBuf = pComponentPrivate->pOutputBufferList->numBuffers;

    for(i=0; i<nBuf; i++) {
        pBuf = pComponentPrivate->pOutputBufferList->pBufHdr[i];
        if(pBufHeader == pBuf) {
            *eDir = OMX_DirOutput;
            MP3DEC_DPRINT (":: Buffer %p is OUTPUT BUFFER\n", pBufHeader);
            flag = 0;
            goto EXIT;
        }
    }

    if (flag == 1) {
        MP3D_OMX_ERROR_EXIT(eError, OMX_ErrorBadParameter,
                            "Buffer Not Found in List : OMX_ErrorBadParameter");
    }
 EXIT:
    MP3DEC_DPRINT (":: Exiting GetBufferDirection Function\n");
    return eError;
}

/** ================================================================================= * */
/**
 * @fn MP3DEC_LCML_Callback() function is callback which is called by LCML whenever
 * there is an even generated for the component.
 *
 * @param event  This is event that was generated.
 *
 * @param arg    This has other needed arguments supplied by LCML like handles
 *               etc.
 *
 * @pre          None
 *
 * @post         None
 *
 *  @return      OMX_ErrorNone = Successful processing.
 *               OMX_ErrorInsufficientResources = Not enough memory
 *
 *  @see         None
 */
/* ================================================================================ * */
OMX_ERRORTYPE MP3DEC_LCML_Callback (TUsnCodecEvent event,void * args [10])
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U8 *pBuffer = args[1];
    MP3D_LCML_BUFHEADERTYPE *pLcmlHdr;
    OMX_COMPONENTTYPE *pHandle;
    LCML_DSP_INTERFACE *pLcmlHandle;
    OMX_U32 i;
    static OMX_U32 TS = 0;
    MP3DEC_BUFDATA *OutputFrames;
    MP3DEC_COMPONENT_PRIVATE* pComponentPrivate = NULL;
#ifdef RESOURCE_MANAGER_ENABLED 
    OMX_ERRORTYPE rm_error = OMX_ErrorNone;
#endif  
    static double time_stmp = 0;

    pComponentPrivate = (MP3DEC_COMPONENT_PRIVATE*)((LCML_DSP_INTERFACE*)args[6])->pComponentPrivate;    

    MP3DEC_DPRINT (":: Entering the LCML_Callback() : event = %d\n",event);
    
    switch(event) {
        
    case EMMCodecDspError:
        MP3DEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecDspError\n");
        break;

    case EMMCodecInternalError:
        MP3DEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecInternalError\n");
        break;

    case EMMCodecInitError:
        MP3DEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecInitError\n");
        break;

    case EMMCodecDspMessageRecieved:
        MP3DEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecDspMessageRecieved\n");
        break;

    case EMMCodecBufferProcessed:
        MP3DEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecBufferProcessed\n");
        break;

    case EMMCodecProcessingStarted:
        MP3DEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecProcessingStarted\n");
        break;
            
    case EMMCodecProcessingPaused:
        MP3DEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecProcessingPaused\n");
        break;

    case EMMCodecProcessingStoped:
        MP3DEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecProcessingStoped\n");
        break;

    case EMMCodecProcessingEof:
        MP3DEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecProcessingEof\n");
        break;

    case EMMCodecBufferNotProcessed:
        MP3DEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecBufferNotProcessed\n");
        break;

    case EMMCodecAlgCtrlAck:
        MP3DEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecAlgCtrlAck\n");
        break;

    case EMMCodecStrmCtrlAck:
        MP3DEC_DPRINT("[LCML CALLBACK EVENT]  EMMCodecStrmCtrlAck\n");
        break;
    }


    if(event == EMMCodecBufferProcessed) {
        if( args[0] == (void *)EMMCodecInputBuffer) {
            MP3DEC_DPRINT (" :: Inside the LCML_Callback EMMCodecInputBuffer\n");
            MP3DEC_DPRINT(":: Input: pBufferr = %p\n", pBuffer);

            eError = MP3DEC_GetCorresponding_LCMLHeader(pComponentPrivate, pBuffer, OMX_DirInput, &pLcmlHdr);
            if (eError != OMX_ErrorNone) {
                MP3DEC_DPRINT(":: Error: Invalid Buffer Came ...\n");
                goto EXIT;
            }
            MP3DEC_DPRINT("%d\t\t\t\t............ Received IP %p\n",__LINE__,pBuffer);
            MP3DEC_DPRINT(":: Output: pLcmlHeader = %p\n", pLcmlHdr);
            MP3DEC_DPRINT(":: Output: pLcmlHdr->eDir = %d\n", pLcmlHdr->eDir);
            MP3DEC_DPRINT(":: Output: *pLcmlHdr->eDir = %d\n", pLcmlHdr->eDir);
            MP3DEC_DPRINT(":: Output: Filled Len = %ld\n", pLcmlHdr->pBufHdr->nFilledLen);
            MP3DEC_DPRINT(":: Input: pLcmlHeader = %p\n", pLcmlHdr);

#ifdef __PERF_INSTRUMENTATION__
            PERF_ReceivedFrame(pComponentPrivate->pPERFcomp,
                               PREF(pLcmlHdr->pBufHdr,pBuffer),
                               0,
                               PERF_ModuleCommonLayer);
#endif
            pComponentPrivate->lcml_nCntIpRes++;
            
            MP3DEC_ClearPending(pComponentPrivate,pLcmlHdr->pBufHdr,OMX_DirInput,__LINE__);

#ifdef __PERF_INSTRUMENTATION__
            PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                              PREF(pLcmlHdr->pBufHdr,pBuffer),
                              0,
                              PERF_ModuleHLMM);
#endif
            pComponentPrivate->cbInfo.EmptyBufferDone (pComponentPrivate->pHandle,
                                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                                       pLcmlHdr->pBufHdr);
            pComponentPrivate->nEmptyBufferDoneCount++;
            pComponentPrivate->lcml_nIpBuf--;
            pComponentPrivate->app_nBuf++;

        } else if (args[0] == (void *)EMMCodecOuputBuffer) {
            MP3DEC_DPRINT (" :: Inside the LCML_Callback EMMCodecOuputBuffer\n");

            MP3DEC_DPRINT("%d\t\t\t\t............ Received OP %p, \tlen %d\n",
                          __LINE__,pBuffer,(int)args[8]);

            MP3DEC_DPRINT(":: Output: pBufferr = %p\n", pBuffer);
            if (!MP3DEC_IsValid(pComponentPrivate,pBuffer,OMX_DirOutput)) {

                MP3DEC_DPRINT("%d :: ############ FillBufferDone Invalid\n",__LINE__);
                /* If the buffer we get back from the DSP is not valid call FillBufferDone
                   on a valid buffer */
#ifdef __PERF_INSTRUMENTATION__
                PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                                  pComponentPrivate->pOutputBufferList->pBufHdr[pComponentPrivate->nInvalidFrameCount]->pBuffer,
                                  pComponentPrivate->pOutputBufferList->pBufHdr[pComponentPrivate->nInvalidFrameCount]->nFilledLen,
                                  PERF_ModuleHLMM);
#endif
                pComponentPrivate->cbInfo.FillBufferDone (pComponentPrivate->pHandle,
                                                          pComponentPrivate->pHandle->pApplicationPrivate,
                                                          pComponentPrivate->pOutputBufferList->pBufHdr[pComponentPrivate->nInvalidFrameCount++]
                                                          );
                /*pComponentPrivate->nOutStandingFillDones--;*/
                pComponentPrivate->numPendingBuffers--;
            }else {
                pComponentPrivate->nOutStandingFillDones++;
                eError = MP3DEC_GetCorresponding_LCMLHeader(pComponentPrivate, pBuffer, OMX_DirOutput, &pLcmlHdr);
                if (eError != OMX_ErrorNone) {
                    MP3DEC_DPRINT(":: Error: Invalid Buffer Came ...\n");
                    goto EXIT;
                }
                pLcmlHdr->pBufHdr->nFilledLen = (int)args[8];
                MP3DEC_DPRINT(":: Output: pLcmlHeader = %p\n", pLcmlHdr);
                MP3DEC_DPRINT(":: Output: pLcmlHdr->eDir = %d\n", pLcmlHdr->eDir);
                MP3DEC_DPRINT(":: Output: Filled Len = %ld\n", pLcmlHdr->pBufHdr->nFilledLen);
                /* Recover MP3DEC_UAlgOutBufParamStruct from SN*/
                OutputFrames = pLcmlHdr->pBufHdr->pOutputPortPrivate;
                OutputFrames->nFrames = (OMX_U8)(pLcmlHdr->pOpParam->ulFrameCount);

#ifdef __PERF_INSTRUMENTATION__
                PERF_ReceivedFrame(pComponentPrivate->pPERFcomp,
                                   PREF(pLcmlHdr->pBufHdr,pBuffer),
                                   PREF(pLcmlHdr->pBufHdr,nFilledLen),
                                   PERF_ModuleCommonLayer);

                pComponentPrivate->nLcml_nCntOpReceived++;

                if ((pComponentPrivate->nLcml_nCntIp >= 1) && (pComponentPrivate->nLcml_nCntOpReceived == 1)) {
                    PERF_Boundary(pComponentPrivate->pPERFcomp,
                                  PERF_BoundaryStart | PERF_BoundarySteadyState);
                }
#endif
                MP3DEC_ClearPending(pComponentPrivate,pLcmlHdr->pBufHdr,OMX_DirOutput,__LINE__);

                /* Previously in HandleDatabuffer form LCML */
                if (pComponentPrivate->pMarkData) {
                    pLcmlHdr->pBufHdr->pMarkData = pComponentPrivate->pMarkData;
                    pLcmlHdr->pBufHdr->hMarkTargetComponent = pComponentPrivate->hMarkTargetComponent;
                }
                pComponentPrivate->num_Reclaimed_Op_Buff++;

                if (pComponentPrivate->bIsEOFSent){
                    MP3DEC_DPRINT ("Adding EOS flag to the output buffer\n");
                    pLcmlHdr->pBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
                    pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                           pComponentPrivate->pHandle->pApplicationPrivate,
                                                           OMX_EventBufferFlag,
                                                           pLcmlHdr->pBufHdr->nOutputPortIndex,
                                                           pLcmlHdr->pBufHdr->nFlags, NULL);
                    pComponentPrivate->bIsEOFSent = 0;
                }

                if(pComponentPrivate->frameMode){
                    /* Copying time stamp information to output buffer */
                    pLcmlHdr->pBufHdr->nTimeStamp = (OMX_TICKS)pComponentPrivate->arrBufIndex[pComponentPrivate->OpBufindex];
                }else{
                    if(pComponentPrivate->first_buff == 1){
                        pComponentPrivate->first_buff = 2;
                        pLcmlHdr->pBufHdr->nTimeStamp = (OMX_U32)pComponentPrivate->first_TS;
                        TS = pLcmlHdr->pBufHdr->nTimeStamp;
                    }else{ 
                        time_stmp = pLcmlHdr->pBufHdr->nFilledLen / (pComponentPrivate->pcmParams->nChannels * 
                                                                     (pComponentPrivate->pcmParams->nBitPerSample / 8));
                        time_stmp = (time_stmp / pComponentPrivate->pcmParams->nSamplingRate) * 1000;
                        /* Update time stamp information */
                        TS += (OMX_U32)time_stmp;
                        pLcmlHdr->pBufHdr->nTimeStamp = TS;
                    }
                }
                /*add on: Copyint tick count information to output buffer*/
                pLcmlHdr->pBufHdr->nTickCount = (OMX_U32)pComponentPrivate->arrBufIndexTick[pComponentPrivate->OpBufindex];
                pComponentPrivate->OpBufindex++;
                pComponentPrivate->OpBufindex %= pComponentPrivate->pPortDef[OMX_DirInput]->nBufferCountActual;


#ifdef __PERF_INSTRUMENTATION__
                PERF_SendingBuffer(pComponentPrivate->pPERFcomp,
                                   pLcmlHdr->pBufHdr->pBuffer,
                                   pLcmlHdr->pBufHdr->nFilledLen,
                                   PERF_ModuleHLMM);
#endif

                pComponentPrivate->cbInfo.FillBufferDone (pComponentPrivate->pHandle,
                                                          pComponentPrivate->pHandle->pApplicationPrivate,
                                                          pLcmlHdr->pBufHdr);
                pComponentPrivate->nOutStandingFillDones--;
                pComponentPrivate->lcml_nOpBuf--;
                pComponentPrivate->app_nBuf++;
                pComponentPrivate->nFillBufferDoneCount++;

            }
        }
    }else if(event == EMMCodecProcessingStoped) {
        if (!pComponentPrivate->bNoIdleOnStop) {
            pComponentPrivate->curState = OMX_StateIdle;
#ifdef RESOURCE_MANAGER_ENABLED
            rm_error = RMProxy_NewSendCommand(pComponentPrivate->pHandle,
                                              RMProxy_StateSet,
                                              OMX_MP3_Decoder_COMPONENT,
                                              OMX_StateIdle, 
                                              3456,
                                              NULL);
#endif  
            if(pComponentPrivate->bPreempted==0){
                pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                                       OMX_EventCmdComplete,
                                                       OMX_CommandStateSet,
                                                       pComponentPrivate->curState,
                                                       NULL);
            }else{
                pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,
                                                       pComponentPrivate->pHandle->pApplicationPrivate,
                                                       OMX_EventError,
                                                       OMX_ErrorResourcesPreempted,
                                                       OMX_TI_ErrorMajor,
                                                       NULL);
            }
        }else{
            pComponentPrivate->bDspStoppedWhileExecuting = OMX_TRUE;
            pComponentPrivate->bNoIdleOnStop = OMX_FALSE;
        }
    }
    else if(event == EMMCodecAlgCtrlAck) {
        MP3DEC_DPRINT ("GOT MESSAGE USN_DSPACK_ALGCTRL \n");
    }
    else if (event == EMMCodecDspError) {
        MP3DEC_DPRINT(":: commandedState  = %p\n",args[0]);
        MP3DEC_DPRINT(":: arg4 = %p\n",args[4]);
        MP3DEC_DPRINT(":: arg5 = %p\n",args[5]);
#ifdef _ERROR_PROPAGATION__
        /* Cheking for MMU_fault */
        if((args[4] == (void *)USN_ERR_UNKNOWN_MSG) && (args[5] == (void*)NULL)) {
            MP3DEC_DPRINT("%d :: UTIL: MMU_Fault \n",__LINE__);
            pComponentPrivate->bIsInvalidState=OMX_TRUE;
            pComponentPrivate->curState = OMX_StateInvalid;
            pHandle = pComponentPrivate->pHandle;
            pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                   pHandle->pApplicationPrivate,
                                                   OMX_EventError,
                                                   OMX_ErrorHardware, 
                                                   OMX_TI_ErrorSevere,
                                                   NULL);
        }
#endif      

        MP3DEC_DPRINT(":: --------- EMMCodecDspError Here\n");
        if(((int)args[4] == USN_ERR_WARNING) && ((int)args[5] == IUALG_WARN_PLAYCOMPLETED)) {
#ifndef UNDER_CE        
            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,                  
                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                   OMX_EventBufferFlag,
                                                   (OMX_U32)NULL,
                                                   OMX_BUFFERFLAG_EOS,
                                                   NULL);
            pComponentPrivate->pLcmlBufHeader[0]->pIpParam->bLastBuffer = 0;
#else
            /* add callback to application to indicate SN/USN has completed playing of current set of date */
            pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle,                  
                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                   OMX_EventBufferFlag,
                                                   (OMX_U32)NULL,
                                                   OMX_BUFFERFLAG_EOS,
                                                   NULL);
#endif
        }

        if((int)args[5] == IUALG_WARN_CONCEALED) {
            MP3DEC_DPRINT( "Algorithm issued a warning. But can continue" );
            MP3DEC_DPRINT("%d :: arg5 = %p\n",__LINE__,args[5]);
        }

	if((int)args[5] == IUALG_ERR_NOT_SUPPORTED) {
            MP3DEC_EPRINT( "Algorithm error. Parameter not supported" );
            MP3DEC_EPRINT("%d :: arg5 = %p\n",__LINE__,args[5]);
            MP3DEC_EPRINT("%d :: LCML_Callback: IUALG_ERR_NOT_SUPPORTED\n",__LINE__);
            pHandle = pComponentPrivate->pHandle;
            pLcmlHandle = (LCML_DSP_INTERFACE *)pComponentPrivate->pLcmlHandle;
	    // pComponentPrivate->curState = OMX_StateInvalid;
            pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                   pHandle->pApplicationPrivate,
                                                   OMX_EventError, 
                                                   OMX_ErrorInsufficientResources,
                                                   OMX_TI_ErrorSevere, 
                                                   NULL);
	}


        if((int)args[5] == IUALG_ERR_GENERAL) {
            MP3DEC_EPRINT( "Algorithm error. Cannot continue" );
            MP3DEC_EPRINT("%d :: arg5 = %p\n",__LINE__,args[5]);
            MP3DEC_EPRINT("%d :: LCML_Callback: IUALG_ERR_GENERAL\n",__LINE__);
            pHandle = pComponentPrivate->pHandle;
            pLcmlHandle = (LCML_DSP_INTERFACE *)pComponentPrivate->pLcmlHandle;
            pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                   pHandle->pApplicationPrivate,
                                                   OMX_EventError, 
                                                   OMX_ErrorUndefined,
                                                   OMX_TI_ErrorSevere, 
                                                   NULL);
        }

        if( (int)args[5] == IUALG_ERR_DATA_CORRUPT ){
            char *pArgs = "damedesuStr";
            MP3DEC_EPRINT("%d :: arg5 = %p\n",__LINE__,args[5]);
            MP3DEC_EPRINT("%d :: LCML_Callback: IUALG_ERR_DATA_CORRUPT\n",__LINE__);
            pHandle = pComponentPrivate->pHandle;
            pLcmlHandle = (LCML_DSP_INTERFACE *)pComponentPrivate->pLcmlHandle;
#ifndef UNDER_CE
            pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                   pHandle->pApplicationPrivate,
                                                   OMX_EventError, 
                                                   OMX_ErrorStreamCorrupt, 
                                                   OMX_TI_ErrorMajor, 
                                                   NULL);
            eError = LCML_ControlCodec(((LCML_DSP_INTERFACE*)pLcmlHandle)->pCodecinterfacehandle,
                                       MMCodecControlStop,(void *)pArgs);
            if(eError != OMX_ErrorNone) {
                MP3DEC_EPRINT("%d: Error Occurred in Codec Stop..\n",
                              __LINE__);
                goto EXIT;
            }
            MP3DEC_DPRINT("%d :: MP3DEC: Codec has been Stopped here\n",__LINE__);
            pComponentPrivate->curState = OMX_StateIdle;
            pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                   pHandle->pApplicationPrivate,
                                                   OMX_EventCmdComplete, 
                                                   OMX_ErrorNone,
                                                   0, 
                                                   NULL);
#else
            pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                   pHandle->pApplicationPrivate,
                                                   OMX_EventError, 
                                                   OMX_ErrorUndefined,
                                                   OMX_TI_ErrorSevere, 
                                                   NULL);
#endif                  
        }
        if( (int)args[5] == IUALG_WARN_OVERFLOW ){
            MP3DEC_DPRINT( "Algorithm error. Overflow" );
        }
        if( (int)args[5] == IUALG_WARN_UNDERFLOW ){
            MP3DEC_DPRINT( "Algorithm error. Underflow" );
        }

    } else if (event == EMMCodecStrmCtrlAck) {
        MP3DEC_DPRINT(":: GOT MESSAGE USN_DSPACK_STRMCTRL ----\n");
        if (args[1] == (void *)USN_STRMCMD_FLUSH) {
            pHandle = pComponentPrivate->pHandle; 
            if ( args[2] == (void *)EMMCodecInputBuffer) {
                if (args[0] == (void *)USN_ERR_NONE ) {
                    MP3DEC_DPRINT("Flushing input port %d\n",__LINE__);
                    for (i=0; i < pComponentPrivate->nNumInputBufPending; i++) {
#ifdef __PERF_INSTRUMENTATION__
                        PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                                          PREF(pComponentPrivate->pInputBufHdrPending[i],pBuffer),
                                          0,
                                          PERF_ModuleHLMM);
#endif

                        pComponentPrivate->cbInfo.EmptyBufferDone (pComponentPrivate->pHandle,
                                                                   pComponentPrivate->pHandle->pApplicationPrivate,
                                                                   pComponentPrivate->pInputBufHdrPending[i]);
                        pComponentPrivate->pInputBufHdrPending[i] = NULL;
                    }
                    pComponentPrivate->nNumInputBufPending=0;    
                    pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                           pHandle->pApplicationPrivate,
                                                           OMX_EventCmdComplete, 
                                                           OMX_CommandFlush,
                                                           MP3D_INPUT_PORT, 
                                                           NULL); 
                } else {
                    MP3DEC_EPRINT ("LCML reported error while flushing input port\n");
                    goto EXIT;                            
                }
            }
            else if ( args[2] == (void *)EMMCodecOuputBuffer) { 
                if (args[0] == (void *)USN_ERR_NONE ) {                      
                    MP3DEC_DPRINT("Flushing output port %d\n",__LINE__);
                    for (i=0; i < pComponentPrivate->nNumOutputBufPending; i++) {
#ifdef __PERF_INSTRUMENTATION__
                        PERF_SendingFrame(pComponentPrivate->pPERFcomp,
                                          PREF(pComponentPrivate->pOutputBufHdrPending[i],pBuffer),
                                          PREF(pComponentPrivate->pOutputBufHdrPending[i],nFilledLen),
                                          PERF_ModuleHLMM);
#endif  

                        pComponentPrivate->cbInfo.FillBufferDone (pComponentPrivate->pHandle,
                                                                  pComponentPrivate->pHandle->pApplicationPrivate,
                                                                  pComponentPrivate->pOutputBufHdrPending[i]
                                                                  );
                        pComponentPrivate->nOutStandingFillDones--;
                        pComponentPrivate->pOutputBufHdrPending[i] = NULL;
                    }
                    pComponentPrivate->nNumOutputBufPending=0;
                    pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle, 
                                                           pComponentPrivate->pHandle->pApplicationPrivate,
                                                           OMX_EventCmdComplete, 
                                                           OMX_CommandFlush,
                                                           MP3D_OUTPUT_PORT,
                                                           NULL);
                } else {
                    MP3DEC_EPRINT ("LCML reported error while flushing output port\n");
                    goto EXIT;                            
                }
            }
        }
    }
    else if (event == EMMCodecProcessingPaused) {
        pComponentPrivate->nUnhandledEmptyThisBuffers = 0;
        pComponentPrivate->nUnhandledFillThisBuffers = 0;
        pComponentPrivate->curState = OMX_StatePause;
        pComponentPrivate->cbInfo.EventHandler(pComponentPrivate->pHandle, 
                                               pComponentPrivate->pHandle->pApplicationPrivate,
                                               OMX_EventCmdComplete, OMX_CommandStateSet,
                                               pComponentPrivate->curState, NULL);
    }
#ifdef _ERROR_PROPAGATION__
    else if (event == EMMCodecInitError){
        /* Cheking for MMU_fault */
        if((args[4] == (void*)USN_ERR_UNKNOWN_MSG) && (args[5] == (void*)NULL)) {
            MP3DEC_EPRINT("%d :: UTIL: MMU_Fault \n",__LINE__);
            pComponentPrivate->bIsInvalidState = OMX_TRUE;
            pComponentPrivate->curState = OMX_StateInvalid;
            pHandle = pComponentPrivate->pHandle;
            pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                   pHandle->pApplicationPrivate,
                                                   OMX_EventError,
                                                   OMX_ErrorHardware, 
                                                   OMX_TI_ErrorSevere,
                                                   NULL);
        }   
    }
    else if (event == EMMCodecInternalError){
        /* Cheking for MMU_fault */
        if((args[4] == (void*)USN_ERR_UNKNOWN_MSG) && (args[5] == (void*)NULL)) {
            MP3DEC_EPRINT("%d :: UTIL: MMU_Fault \n",__LINE__);
            pComponentPrivate->bIsInvalidState = OMX_TRUE;
            pComponentPrivate->curState = OMX_StateInvalid;
            pHandle = pComponentPrivate->pHandle;
            pComponentPrivate->cbInfo.EventHandler(pHandle, 
                                                   pHandle->pApplicationPrivate,
                                                   OMX_EventError,
                                                   OMX_ErrorHardware, 
                                                   OMX_TI_ErrorSevere,
                                                   NULL);
        }

    }
#endif
 EXIT:
    MP3DEC_DPRINT (":: Exiting the LCML_Callback() \n");
    return eError;
}


/* ================================================================================= * */
/**
 * @fn MP3DEC_GetCorresponding_LCMLHeader() function gets the corresponding LCML
 * header from the actual data buffer for required processing.
 *
 * @param *pBuffer This is the data buffer pointer. 
 *
 * @param eDir   This is direction of buffer. Input/Output.
 *
 * @param *MP3D_LCML_BUFHEADERTYPE  This is pointer to LCML Buffer Header.
 *
 * @pre          None
 *
 * @post         None
 *
 *  @return      OMX_ErrorNone = Successful Inirialization of the component\n
 *               OMX_ErrorHardware = Hardware error has occured.
 *
 *  @see         None
 */
/* ================================================================================ * */
OMX_ERRORTYPE MP3DEC_GetCorresponding_LCMLHeader(MP3DEC_COMPONENT_PRIVATE* pComponentPrivate,
                                                 OMX_U8 *pBuffer,
                                                 OMX_DIRTYPE eDir,
                                                 MP3D_LCML_BUFHEADERTYPE **ppLcmlHdr)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    MP3D_LCML_BUFHEADERTYPE *pLcmlBufHeader;
    int nIpBuf=0, nOpBuf=0, i=0;

    MP3DEC_DPRINT (":: Entering the MP3DEC_GetCorresponding_LCMLHeader()\n");

    MP3DEC_DPRINT (":: eDir = %d\n",eDir);

    while (!pComponentPrivate->bInitParamsInitialized) {
#ifndef UNDER_CE
        sched_yield();
#else
        Sleep(0);
#endif
    }


    if(eDir == OMX_DirInput) {
        MP3DEC_DPRINT (":: In GetCorresponding_LCMLHeader()\n");

        nIpBuf = pComponentPrivate->pInputBufferList->numBuffers;

        pLcmlBufHeader = pComponentPrivate->pLcmlBufHeader[MP3D_INPUT_PORT];

        for(i=0; i<nIpBuf; i++) {
            MP3DEC_DPRINT("pBuffer = %p\n",pBuffer);
            MP3DEC_DPRINT("pLcmlBufHeader->pBufHdr->pBuffer = %p\n",pLcmlBufHeader->pBufHdr->pBuffer);
            if(pBuffer == pLcmlBufHeader->pBufHdr->pBuffer) {
                *ppLcmlHdr = pLcmlBufHeader;
                MP3DEC_DPRINT("::Corresponding LCML Header Found\n");
                goto EXIT;
            }
            pLcmlBufHeader++;
        }
    } else if (eDir == OMX_DirOutput) {
        i = 0;
        nOpBuf = pComponentPrivate->pOutputBufferList->numBuffers;

        pLcmlBufHeader = pComponentPrivate->pLcmlBufHeader[MP3D_OUTPUT_PORT];
        MP3DEC_DPRINT (":: nOpBuf = %d\n",nOpBuf);

        for(i=0; i<nOpBuf; i++) {
            MP3DEC_DPRINT("pBuffer = %p\n",pBuffer);
            MP3DEC_DPRINT("pLcmlBufHeader->pBufHdr->pBuffer = %p\n",pLcmlBufHeader->pBufHdr->pBuffer);

            if(pBuffer == pLcmlBufHeader->pBufHdr->pBuffer) {
                *ppLcmlHdr = pLcmlBufHeader;
                MP3DEC_DPRINT("::Corresponding LCML Header Found\n");
                goto EXIT;
            }
            pLcmlBufHeader++;
        }
    } else {
        MP3DEC_DPRINT(":: Invalid Buffer Type :: exiting...\n");
    }

 EXIT:
    MP3DEC_DPRINT (":: Exiting the GetCorresponding_LCMLHeader() \n");
    return eError;
}

/* ================================================================================= * */
/**
 * @fn MP3DEC_GetLCMLHandle() function gets the LCML handle and interacts with LCML
 * by using this LCML Handle.
 *
 * @param *pBufHeader This is the buffer header that needs to be processed.
 *
 * @param *pComponentPrivate  This is component's private date structure.
 *
 * @pre          None
 *
 * @post         None
 *
 *  @return      OMX_HANDLETYPE = Successful loading of LCML library.
 *               OMX_ErrorHardware = Hardware error has occured.
 *
 *  @see         None
 */
/* ================================================================================ * */
#ifndef UNDER_CE
OMX_HANDLETYPE MP3DEC_GetLCMLHandle(MP3DEC_COMPONENT_PRIVATE *pComponentPrivate)
{
    /* This must be taken care by WinCE */
    OMX_HANDLETYPE pHandle = NULL;
    OMX_ERRORTYPE eError;
    void *handle;
    OMX_ERRORTYPE (*fpGetHandle)(OMX_HANDLETYPE);
    char *error;

    handle = dlopen("libLCML.so", RTLD_LAZY);
    if (!handle) {
        fputs(dlerror(), stderr);
        goto EXIT;
    }

    fpGetHandle = dlsym (handle, "GetHandle");
    if ((error = dlerror()) != NULL) {
        fputs(error, stderr);
        goto EXIT;
    }
    eError = (*fpGetHandle)(&pHandle);
    if(eError != OMX_ErrorNone) {
        eError = OMX_ErrorUndefined;
        MP3DEC_DPRINT("eError != OMX_ErrorNone...\n");
        pHandle = NULL;
        goto EXIT;
    }

    ((LCML_DSP_INTERFACE*)pHandle)->pComponentPrivate = pComponentPrivate;

 EXIT:
    return pHandle;
}
#else
/* WINDOWS Explicit dll load procedure */
OMX_HANDLETYPE MP3DEC_GetLCMLHandle(MP3DEC_COMPONENT_PRIVATE *pComponentPrivate)
{
    typedef OMX_ERRORTYPE (*LPFNDLLFUNC1)(OMX_HANDLETYPE);
    OMX_HANDLETYPE pHandle = NULL;
    OMX_ERRORTYPE eError;
    LPFNDLLFUNC1 fpGetHandle1;

    g_hLcmlDllHandle = LoadLibraryEx(TEXT("OAF_BML.dll"), NULL,0);
    if (g_hLcmlDllHandle == NULL)
        {
            /* fputs(dlerror(), stderr); */
            MP3DEC_EPRINT("BML Load Failed!!!\n");
            return pHandle;
        }
    fpGetHandle1 = (LPFNDLLFUNC1)GetProcAddress(g_hLcmlDllHandle,TEXT("GetHandle"));
    if (!fpGetHandle1)
        {
            /* handle the error*/
            FreeLibrary(g_hLcmlDllHandle);
            g_hLcmlDllHandle = NULL;
            return pHandle;
        }
    /* call the function */
    eError = fpGetHandle1(&pHandle);
    if(eError != OMX_ErrorNone) 
        {
            eError = OMX_ErrorUndefined;
            MP3DEC_EPRINT("eError != OMX_ErrorNone...\n");
            FreeLibrary(g_hLcmlDllHandle);
            g_hLcmlDllHandle = NULL;
            pHandle = NULL;
            return pHandle;
        }

    ((LCML_DSP_INTERFACE*)pHandle)->pComponentPrivate = pComponentPrivate;

    return pHandle;
}
#endif

#ifndef UNDER_CE
OMX_ERRORTYPE MP3DECFreeLCMLHandle(MP3DEC_COMPONENT_PRIVATE *pComponentPrivate)
{

    OMX_S16 retValue;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    if (pComponentPrivate->bLcmlHandleOpened) {
        retValue = dlclose(pComponentPrivate->pLcmlHandle);

        if (retValue != 0) {
            eError = OMX_ErrorUndefined;
        }
        pComponentPrivate->bLcmlHandleOpened = 0;
    }

    return eError;
}
#else
OMX_ERRORTYPE MP3DECFreeLCMLHandle(MP3DEC_COMPONENT_PRIVATE *pComponentPrivate)
{

    OMX_S16 retValue;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    if (pComponentPrivate->bLcmlHandleOpened) {

        retValue = FreeLibrary(pComponentPrivate->pLcmlHandle);
        if (retValue == 0) {          /* Zero Indicates failure */
            eError = OMX_ErrorUndefined;
        }
        pComponentPrivate->bLcmlHandleOpened = 0;
    }

    return eError;
}
#endif

/* ========================================================================== */
/**
 * @MP3DEC_CleanupInitParams() This function is called by the component during
 * de-init to close component thread, Command pipe, data pipe & LCML pipe.
 *
 * @param pComponent  handle for this instance of the component
 *
 * @pre
 *
 * @post
 *
 * @return none
 */
/* ========================================================================== */

void MP3DEC_CleanupInitParams(OMX_HANDLETYPE pComponent)
{
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    MP3DEC_COMPONENT_PRIVATE *pComponentPrivate = (MP3DEC_COMPONENT_PRIVATE *)
        pHandle->pComponentPrivate;
    MP3D_LCML_BUFHEADERTYPE *pTemp_lcml;

    OMX_U32 nIpBuf = pComponentPrivate->nRuntimeInputBuffers;
    OMX_U32 nOpBuf = pComponentPrivate->nRuntimeOutputBuffers;

    OMX_U32 i=0;
    char *tempt = NULL;

    MP3DEC_DPRINT (":: MP3DEC_CleanupInitParams()\n");

    MP3DEC_MEMPRINT(":: Freeing:  pComponentPrivate->strmAttr = %p\n", pComponentPrivate->strmAttr);
    MP3D_OMX_FREE(pComponentPrivate->strmAttr); 

    pTemp_lcml = pComponentPrivate->pLcmlBufHeader[MP3D_INPUT_PORT];
    for(i=0; i<nIpBuf; i++) {
        MP3DEC_MEMPRINT(":: Freeing: pTemp_lcml->pIpParam = %p\n",pTemp_lcml->pIpParam);
        tempt = (char*)pTemp_lcml->pIpParam;
        if(tempt != NULL){
            tempt -= EXTRA_BYTES;
        }
        pTemp_lcml->pIpParam = (MP3DEC_UAlgInBufParamStruct*)tempt;
        MP3D_OMX_FREE(pTemp_lcml->pIpParam);

        pTemp_lcml++;
    }

    MP3DEC_MEMPRINT(":: Freeing pComponentPrivate->pLcmlBufHeader[MP3D_INPUT_PORT] = %p\n",
                    pComponentPrivate->pLcmlBufHeader[MP3D_INPUT_PORT]);
    MP3D_OMX_FREE(pComponentPrivate->pLcmlBufHeader[MP3D_INPUT_PORT]);

    pTemp_lcml = pComponentPrivate->pLcmlBufHeader[MP3D_OUTPUT_PORT];
    for(i=0; i<nOpBuf; i++) {
        MP3DEC_MEMPRINT(":: Freeing: pTemp_lcml->pOpParam = %p\n",pTemp_lcml->pOpParam);

        tempt = (char*)pTemp_lcml->pOpParam;
        if(tempt != NULL){
            tempt -= EXTRA_BYTES;
        }
        pTemp_lcml->pOpParam = (MP3DEC_UAlgOutBufParamStruct*)tempt;
        MP3D_OMX_FREE(pTemp_lcml->pOpParam);
        pTemp_lcml++;
    }

    MP3DEC_MEMPRINT(":: Freeing: pComponentPrivate->pLcmlBufHeader[MP3D_OUTPUT_PORT] = %p\n",
                    pComponentPrivate->pLcmlBufHeader[MP3D_OUTPUT_PORT]);
    MP3D_OMX_FREE(pComponentPrivate->pLcmlBufHeader[MP3D_OUTPUT_PORT]);
    
    tempt = (char*)pComponentPrivate->pParams;
    if(tempt != NULL){
        tempt -= EXTRA_BYTES;
    }
    pComponentPrivate->pParams = (USN_AudioCodecParams *) tempt;
    MP3D_OMX_FREE(pComponentPrivate->pParams);
    
    tempt = (char *) pComponentPrivate->ptAlgDynParams;
    if(tempt != NULL) {
        tempt -= EXTRA_BYTES;
    }
    pComponentPrivate->ptAlgDynParams = (MP3DEC_UALGParams*) tempt;
    MP3D_OMX_FREE(pComponentPrivate->ptAlgDynParams);
    
    MP3DEC_DPRINT ("Exiting Successfully MP3DEC_CleanupInitParams()\n");
}


/* ========================================================================== */
/**
 * @MP3DEC_SetPending() This function marks the buffer as pending when it is sent
 * to DSP/
 *
 * @param pComponentPrivate This is component's private date area.
 *
 * @param pBufHdr This is poiter to OMX Buffer header whose buffer is sent to DSP
 *
 * @param eDir This is direction of buffer i.e. input or output.
 *
 * @pre None
 *
 * @post None
 *
 * @return none
 */
/* ========================================================================== */
void MP3DEC_SetPending(MP3DEC_COMPONENT_PRIVATE *pComponentPrivate, OMX_BUFFERHEADERTYPE *pBufHdr, OMX_DIRTYPE eDir, OMX_U32 lineNumber)
{
    OMX_U16 i;

    MP3DEC_DPRINT("Called MP3DEC_SetPending\n");
    MP3DEC_DPRINT("eDir = %d\n",eDir);

    if (eDir == OMX_DirInput) {
        for (i=0; i < pComponentPrivate->pInputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pInputBufferList->pBufHdr[i]) {
                pComponentPrivate->pInputBufferList->bBufferPending[i] = 1;
                MP3DEC_DPRINT("INPUT BUFFER %d IS PENDING Line %ld\n",i,lineNumber);
            }
        }
    }
    else {
        for (i=0; i < pComponentPrivate->pOutputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pOutputBufferList->pBufHdr[i]) {
                pComponentPrivate->pOutputBufferList->bBufferPending[i] = 1;
                MP3DEC_DPRINT("OUTPUT BUFFER %d IS PENDING Line %ld\n",i,lineNumber);
            }
        }
    }
}

/* ========================================================================== */
/**
 * @MP3DEC_ClearPending() This function clears the buffer status from pending
 * when it is received back from DSP.
 *
 * @param pComponentPrivate This is component's private date area.
 *
 * @param pBufHdr This is poiter to OMX Buffer header that is received from
 * DSP/LCML.
 *
 * @param eDir This is direction of buffer i.e. input or output.
 *
 * @pre None
 *
 * @post None
 *
 * @return none
 */
/* ========================================================================== */

void MP3DEC_ClearPending(MP3DEC_COMPONENT_PRIVATE *pComponentPrivate, OMX_BUFFERHEADERTYPE *pBufHdr, OMX_DIRTYPE eDir, OMX_U32 lineNumber)
{
    OMX_U16 i;

    if (eDir == OMX_DirInput) {
        for (i=0; i < pComponentPrivate->pInputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pInputBufferList->pBufHdr[i]) {
                pComponentPrivate->pInputBufferList->bBufferPending[i] = 0;
                MP3DEC_DPRINT("INPUT BUFFER %d IS RECLAIMED Line %ld\n",i,lineNumber);
            }
        }
    }
    else {
        for (i=0; i < pComponentPrivate->pOutputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pOutputBufferList->pBufHdr[i]) {
                pComponentPrivate->pOutputBufferList->bBufferPending[i] = 0;
                MP3DEC_DPRINT("OUTPUT BUFFER %d IS RECLAIMED Line %ld\n",i,lineNumber);
            }
        }
    }
}
  
/* ========================================================================== */
/**
 * @MP3DEC_IsPending() This function checks whether or not a buffer is pending.
 *
 * @param pComponentPrivate This is component's private date area.
 *
 * @param pBufHdr This is poiter to OMX Buffer header of interest.
 *
 * @param eDir This is direction of buffer i.e. input or output.
 *
 * @pre None
 *
 * @post None
 *
 * @return none
 */
/* ========================================================================== */

OMX_U32 MP3DEC_IsPending(MP3DEC_COMPONENT_PRIVATE *pComponentPrivate, OMX_BUFFERHEADERTYPE *pBufHdr, OMX_DIRTYPE eDir)
{
    OMX_U16 i;

    if (eDir == OMX_DirInput) {
        for (i=0; i < pComponentPrivate->pInputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pInputBufferList->pBufHdr[i]) {
                return pComponentPrivate->pInputBufferList->bBufferPending[i];
            }
        }
    }
    else {
        for (i=0; i < pComponentPrivate->pOutputBufferList->numBuffers; i++) {
            if (pBufHdr == pComponentPrivate->pOutputBufferList->pBufHdr[i]) {
                return pComponentPrivate->pOutputBufferList->bBufferPending[i];
            }
        }
    }
    return -1;
}


/* ========================================================================== */
/**
 * @MP3DEC_IsValid() This function identifies whether or not buffer recieved from
 * LCML is valid. It searches in the list of input/output buffers to do this.
 *
 * @param pComponentPrivate This is component's private date area.
 *
 * @param pBufHdr This is poiter to OMX Buffer header of interest.
 *
 * @param eDir This is direction of buffer i.e. input or output.
 *
 * @pre None
 *
 * @post None
 *
 * @return status of the buffer.
 */
/* ========================================================================== */

OMX_U32 MP3DEC_IsValid(MP3DEC_COMPONENT_PRIVATE *pComponentPrivate, OMX_U8 *pBuffer, OMX_DIRTYPE eDir)
{
    OMX_U16 i;
    int found=0;

    if (eDir == OMX_DirInput) {
        for (i=0; i < pComponentPrivate->pInputBufferList->numBuffers; i++) {
            if (pBuffer == pComponentPrivate->pInputBufferList->pBufHdr[i]->pBuffer) {
                found = 1;
            }
        }
    }
    else {
        for (i=0; i < pComponentPrivate->pOutputBufferList->numBuffers; i++) {
            if (pBuffer == pComponentPrivate->pOutputBufferList->pBufHdr[i]->pBuffer) {
                found = 1;
            }
        }
    }
    return found;
}

/* ========================================================================== */
/**
 * @MP3DECFill_LCMLInitParamsEx() This function initializes the init parameter of
 * the LCML structure when a port is enabled and component is in idle state.
 *
 * @param pComponent This is component handle.
 *
 * @pre None
 *
 * @post None
 *
 * @return appropriate OMX Error.
 */
/* ========================================================================== */

OMX_ERRORTYPE MP3DECFill_LCMLInitParamsEx(OMX_HANDLETYPE pComponent)

{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 nIpBuf,nIpBufSize,nOpBuf,nOpBufSize;
    OMX_U16 i;
    OMX_BUFFERHEADERTYPE *pTemp;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)pComponent;
    MP3DEC_COMPONENT_PRIVATE *pComponentPrivate =
        (MP3DEC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;
    MP3D_LCML_BUFHEADERTYPE *pTemp_lcml;
    OMX_U32 size_lcml;
    OMX_U8 *ptr;
    char *char_temp = NULL;

    MP3DEC_DPRINT(":: Entered Fill_LCMLInitParams");

    nIpBuf = pComponentPrivate->pInputBufferList->numBuffers;
    nOpBuf = pComponentPrivate->pOutputBufferList->numBuffers;
    nIpBufSize = pComponentPrivate->pPortDef[MP3D_INPUT_PORT]->nBufferSize;
    nOpBufSize = pComponentPrivate->pPortDef[MP3D_OUTPUT_PORT]->nBufferSize;

    MP3DEC_BUFPRINT("Input Buffer Count = %ld\n",nIpBuf);
    MP3DEC_BUFPRINT("Input Buffer Size = %ld\n",nIpBufSize);
    MP3DEC_BUFPRINT("Output Buffer Count = %ld\n",nOpBuf);
    MP3DEC_BUFPRINT("Output Buffer Size = %ld\n",nOpBufSize);

    MP3DEC_DPRINT(":: bufAlloced = %d\n",pComponentPrivate->bufAlloced);
    size_lcml = nIpBuf * sizeof(MP3D_LCML_BUFHEADERTYPE);

    MP3D_OMX_MALLOC_SIZE(ptr,size_lcml,OMX_U8);
    pTemp_lcml = (MP3D_LCML_BUFHEADERTYPE *)ptr;

    pComponentPrivate->pLcmlBufHeader[MP3D_INPUT_PORT] = pTemp_lcml;

    for (i=0; i<nIpBuf; i++) {
        pTemp = pComponentPrivate->pInputBufferList->pBufHdr[i];
        pTemp->nSize = sizeof(OMX_BUFFERHEADERTYPE);

        pTemp->nAllocLen = nIpBufSize;
        pTemp->nFilledLen = nIpBufSize;
        pTemp->nVersion.s.nVersionMajor = MP3DEC_MAJOR_VER;
        pTemp->nVersion.s.nVersionMinor = MP3DEC_MINOR_VER;

        pTemp->pPlatformPrivate = pHandle->pComponentPrivate;
        pTemp->nTickCount = 0;

        pTemp_lcml->pBufHdr = pTemp;
        pTemp_lcml->eDir = OMX_DirInput;
        pTemp_lcml->pOtherParams[i] = NULL;

        MP3D_OMX_MALLOC_SIZE(pTemp_lcml->pIpParam,
                             (sizeof(MP3DEC_UAlgInBufParamStruct) + DSP_CACHE_ALIGNMENT),
                             MP3DEC_UAlgInBufParamStruct);
        char_temp = (char*)pTemp_lcml->pIpParam;
        char_temp += EXTRA_BYTES;
        pTemp_lcml->pIpParam = (MP3DEC_UAlgInBufParamStruct*)char_temp;

        pTemp_lcml->pIpParam->bLastBuffer = 0;

        pTemp->nFlags = NORMAL_BUFFER;
        ((MP3DEC_COMPONENT_PRIVATE *) pTemp->pPlatformPrivate)->pHandle = pHandle;

        MP3DEC_DPRINT("::Comp: InBuffHeader[%d] = %p\n", i, pTemp);
        MP3DEC_DPRINT("::Comp:  >>>> InputBuffHeader[%d]->pBuffer = %p\n", i, pTemp->pBuffer);
        MP3DEC_DPRINT("::Comp: Ip : pTemp_lcml[%d] = %p\n", i, pTemp_lcml);

        pTemp_lcml++;
    }

    size_lcml = nOpBuf * sizeof(MP3D_LCML_BUFHEADERTYPE);
    MP3D_OMX_MALLOC_SIZE(pTemp_lcml,size_lcml,MP3D_LCML_BUFHEADERTYPE);
    pComponentPrivate->pLcmlBufHeader[MP3D_OUTPUT_PORT] = pTemp_lcml;

    for (i=0; i<nOpBuf; i++) {
        pTemp = pComponentPrivate->pOutputBufferList->pBufHdr[i];
        pTemp->nSize = sizeof(OMX_BUFFERHEADERTYPE);
        pTemp->nAllocLen = nOpBufSize;
        pTemp->nFilledLen = nOpBufSize;
        pTemp->nVersion.s.nVersionMajor = MP3DEC_MAJOR_VER;
        pTemp->nVersion.s.nVersionMinor = MP3DEC_MINOR_VER;

        pTemp->pPlatformPrivate = pHandle->pComponentPrivate;
        pTemp->nTickCount = 0;

        pTemp_lcml->pBufHdr = pTemp;
        pTemp_lcml->eDir = OMX_DirOutput;
        pTemp_lcml->pOtherParams[i] = NULL;

        MP3D_OMX_MALLOC_SIZE(pTemp_lcml->pOpParam,
                             (sizeof(MP3DEC_UAlgOutBufParamStruct) + DSP_CACHE_ALIGNMENT),
                             MP3DEC_UAlgOutBufParamStruct);
        char_temp = (char*)pTemp_lcml->pOpParam;
        char_temp += EXTRA_BYTES;
        pTemp_lcml->pOpParam = (MP3DEC_UAlgOutBufParamStruct*)char_temp;
        pTemp_lcml->pOpParam->ulFrameCount = DONT_CARE;

        pTemp->nFlags = NORMAL_BUFFER;
        ((MP3DEC_COMPONENT_PRIVATE *)pTemp->pPlatformPrivate)->pHandle = pHandle;
        MP3DEC_DPRINT("::Comp:  >>>>>>>>>>>>> OutBuffHeader[%d] = %p\n", i, pTemp);
        MP3DEC_DPRINT("::Comp:  >>>> OutBuffHeader[%d]->pBuffer = %p\n", i, pTemp->pBuffer);
        MP3DEC_DPRINT("::Comp: Op : pTemp_lcml[%d] = %p\n", i, pTemp_lcml);
        pTemp_lcml++;
    }
    pComponentPrivate->bPortDefsAllocated = 1;

    MP3DEC_DPRINT(":: Exiting Fill_LCMLInitParams");

    pComponentPrivate->bInitParamsInitialized = 1;

 EXIT:
    return eError;
}
/*void MP3_ResourceManagerCallback(RMPROXY_COMMANDDATATYPE cbData){
    OMX_COMMANDTYPE Cmd = OMX_CommandStateSet;
    OMX_STATETYPE state = OMX_StateIdle;
    OMX_COMPONENTTYPE *pHandle = (OMX_COMPONENTTYPE *)cbData.hComponent;
    MP3DEC_COMPONENT_PRIVATE *pCompPrivate = NULL;

    pCompPrivate = (MP3DEC_COMPONENT_PRIVATE *)pHandle->pComponentPrivate;

    if (*(cbData.RM_Error) == OMX_RmProxyCallback_ResourcesPreempted) {
        if (pCompPrivate->curState == OMX_StateExecuting || 
            pCompPrivate->curState == OMX_StatePause) {
            write (pCompPrivate->cmdPipe[1], &Cmd, sizeof(Cmd));
            write (pCompPrivate->cmdDataPipe[1], &state ,sizeof(OMX_U32));

            pCompPrivate->bPreempted = 1;
        }
    }
    else if (*(cbData.RM_Error) == OMX_RmProxyCallback_ResourcesAcquired){
        pCompPrivate->cbInfo.EventHandler (pHandle, 
                                           pHandle->pApplicationPrivate,
                                           OMX_EventResourcesAcquired, 
                                           0,
                                           0,
                                           NULL);
    }
}
*/
