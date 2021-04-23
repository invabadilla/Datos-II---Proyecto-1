//
// Created by usuario on 4/10/21.
//

#include "CMemoryPool.h"
#include "SMemoryChunk.h"
#include "Variable.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

namespace MemPool {

    static const int FREEED_MEMORY_CONTENT = 0xAA;
    static const int NEW_ALLOCATED_MEMORY_CONTENT = 0xFF;

    CMemoryPool::CMemoryPool(const size_t &sInitialMemoryPoolSize, const size_t &sMemoryChunkSize,
                             const size_t &sMinimalMemorySizeToAllocate, bool bSetMemoryData) {

        m_ptrFirstChunk = NULL;
        m_ptrLastChunk = NULL;
        m_ptrCursorChunk = NULL;

        m_sTotalMemoryPoolSize = 0 ;
        m_sUsedMemoryPoolSize = 0 ;
        m_sFreeMemoryPoolSize = 0 ;

        m_sMemoryChunkSize = sMemoryChunkSize ;
        m_uiMemoryChunkCount = 0 ;
        m_uiObjectCount = 0 ;

        m_bSetMemoryData = bSetMemoryData ;

        AllocateMemory(sInitialMemoryPoolSize) ;
    }

    CMemoryPool::~CMemoryPool(){
        FreeAllAllocatedMemory() ;
        DeallocateAllChunks();
        assert((m_uiObjectCount == 0) && "WARNING : Memory-Leak : You have not freed all allocated Memory") ;
    }

/******************
GetMemory
******************/
    void *CMemoryPool::GetMemory(const std::size_t &sMemorySize){
        std::size_t sBestMemBlockSize = CalculateBestMemoryBlockSize(sMemorySize) ;
        SMemoryChunk *ptrChunk = NULL ;
        while(!ptrChunk){
            ptrChunk = FindChunkSuitableToHoldMemory(sBestMemBlockSize) ;
            if(!ptrChunk){
                std::cout << "toy chikito";
                //INSERTAR LOG
                break;
            }
        }

        m_sUsedMemoryPoolSize += sBestMemBlockSize ;
        m_sFreeMemoryPoolSize -= sBestMemBlockSize ;
        m_uiObjectCount++ ;
        SetMemoryChunkValues(ptrChunk, sBestMemBlockSize) ;

        return ((void *) ptrChunk->Data) ;
    }

/******************
FreeMemory
******************/
    void CMemoryPool::FreeMemory(void *ptrMemoryBlock){

        SMemoryChunk *ptrChunk = FindChunkHoldingPointerTo(ptrMemoryBlock) ;
        std::cout << ptrChunk << std::endl;
        if(ptrChunk){
            FreeChunks(ptrChunk) ;
        }
        else{
            assert(false && "ERROR : Requested Pointer not in Memory Pool") ;
        }
        assert((m_uiObjectCount > 0) && "ERROR : Request to delete more Memory then allocated.") ;
        m_uiObjectCount-- ;
    }

/******************
AllocateMemory
******************/
    bool CMemoryPool::AllocateMemory(const std::size_t &sMemorySize){

        unsigned int uiNeededChunks = CalculateNeededChunks(sMemorySize) ;
        std::size_t sBestMemBlockSize = CalculateBestMemoryBlockSize(sMemorySize) ;

        TByte *ptrNewMemBlock = (TByte *) malloc(sBestMemBlockSize) ; // allocate from Operating System
        SMemoryChunk *ptrNewChunks = (SMemoryChunk *) malloc((uiNeededChunks * sizeof(SMemoryChunk))) ; // allocate Chunk-Array to Manage the Memory
        assert(((ptrNewMemBlock) && (ptrNewChunks)) && "Error : System ran out of Memory") ;

        m_sTotalMemoryPoolSize += sBestMemBlockSize ;
        m_sFreeMemoryPoolSize += sBestMemBlockSize ;
        m_uiMemoryChunkCount += uiNeededChunks ;

        if(m_bSetMemoryData){
            memset(((void *) ptrNewMemBlock), NEW_ALLOCATED_MEMORY_CONTENT, sBestMemBlockSize) ;
        }

        return LinkChunksToData(ptrNewChunks, uiNeededChunks, ptrNewMemBlock) ; ;
    }

/******************
CalculateNeededChunks
******************/
    unsigned int CMemoryPool::CalculateNeededChunks(const std::size_t &sMemorySize){
        float f = (float) (((float)sMemorySize) / ((float)m_sMemoryChunkSize)) ;
        return ((unsigned int) ceil(f)) ;
    }

/******************
CalculateBestMemoryBlockSize
******************/
    std::size_t CMemoryPool::CalculateBestMemoryBlockSize(const std::size_t &sRequestedMemoryBlockSize){
        unsigned int uiNeededChunks = CalculateNeededChunks(sRequestedMemoryBlockSize) ;
        return std::size_t((uiNeededChunks * m_sMemoryChunkSize)) ;
    }

/******************
FreeChunks
******************/
    void CMemoryPool::FreeChunks(SMemoryChunk *ptrChunk){
        SMemoryChunk *ptrCurrentChunk = ptrChunk ;
        unsigned int uiChunkCount = CalculateNeededChunks(ptrCurrentChunk->UsedSize);
        for(unsigned int i = 0; i < uiChunkCount; i++){
            if(ptrCurrentChunk){
                if(m_bSetMemoryData){
                    memset(((void *) ptrCurrentChunk->Data), FREEED_MEMORY_CONTENT, m_sMemoryChunkSize) ;
                }
                ptrCurrentChunk->UsedSize = 0 ;
                m_sUsedMemoryPoolSize -= m_sMemoryChunkSize ;
                ptrCurrentChunk = ptrCurrentChunk->Next ;
            }
        }
    }


/******************
FindChunkSuitableToHoldMemory
******************/
    SMemoryChunk *CMemoryPool::FindChunkSuitableToHoldMemory(const std::size_t &sMemorySize){
        unsigned int uiChunksToSkip = 0 ;
        bool bContinueSearch = true ;
        SMemoryChunk *ptrChunk = m_ptrCursorChunk ;
        for(unsigned int i = 0; i < m_uiMemoryChunkCount; i++){
            if(ptrChunk){
                if(ptrChunk == m_ptrLastChunk){
                    ptrChunk = m_ptrFirstChunk ;
                }
                if(ptrChunk->DataSize >= sMemorySize){
                    if(ptrChunk->UsedSize == 0){
                        m_ptrCursorChunk = ptrChunk ;
                        return ptrChunk ;
                    }
                }
                uiChunksToSkip = CalculateNeededChunks(ptrChunk->UsedSize) ;
                if(uiChunksToSkip == 0) uiChunksToSkip = 1 ;
                ptrChunk = SkipChunks(ptrChunk, uiChunksToSkip) ;
            }
            else{
                bContinueSearch = false ;
            }
        }
        return NULL ;
    }

/******************
SkipChunks
******************/
    SMemoryChunk *CMemoryPool::SkipChunks(SMemoryChunk *ptrStartChunk, unsigned int uiChunksToSkip){
        SMemoryChunk *ptrCurrentChunk = ptrStartChunk ;
        for(unsigned int i = 0; i < uiChunksToSkip; i++){
            if(ptrCurrentChunk){
                ptrCurrentChunk = ptrCurrentChunk->Next ;
            }
            else{
                assert(false && "Error : Chunk == NULL was not expected.") ;
                break ;
            }
        }
        return ptrCurrentChunk ;
    }

/******************
SetMemoryChunkValues
******************/
    void CMemoryPool::SetMemoryChunkValues(SMemoryChunk *ptrChunk, const std::size_t &sMemBlockSize){
        if((ptrChunk)){
            ptrChunk->UsedSize = sMemBlockSize ;
        }
        else{
            assert(false && "Error : Invalid NULL-Pointer passed") ;
        }
    }

/******************
LinkChunksToData
******************/
    bool CMemoryPool::LinkChunksToData(SMemoryChunk *ptrNewChunks, unsigned int uiChunkCount, TByte *ptrNewMemBlock){
        SMemoryChunk *ptrNewChunk = NULL ;
        unsigned int uiMemOffSet = 0 ;
        bool bAllocationChunkAssigned = false ;
        for(unsigned int i = 0; i < uiChunkCount; i++){
            if(!m_ptrFirstChunk){
                m_ptrFirstChunk = SetChunkDefaults(&(ptrNewChunks[0])) ;
                m_ptrLastChunk = m_ptrFirstChunk ;
                m_ptrCursorChunk = m_ptrFirstChunk ;
            }
            else{
                ptrNewChunk = SetChunkDefaults(&(ptrNewChunks[i])) ;
                m_ptrLastChunk->Next = ptrNewChunk ;
                m_ptrLastChunk = ptrNewChunk ;
            }

            uiMemOffSet = (i * ((unsigned int) m_sMemoryChunkSize)) ;
            m_ptrLastChunk->Data = &(ptrNewMemBlock[uiMemOffSet]) ;

            if(!bAllocationChunkAssigned){
                m_ptrLastChunk->IsAllocationChunk = true ;
                bAllocationChunkAssigned = true ;
            }
        }
        return RecalcChunkMemorySize(m_ptrFirstChunk, m_uiMemoryChunkCount) ;
    }

/******************
RecalcChunkMemorySize
******************/
    bool CMemoryPool::RecalcChunkMemorySize(SMemoryChunk *ptrChunk, unsigned int uiChunkCount){
        unsigned int uiMemOffSet = 0 ;
        for(unsigned int i = 0; i < uiChunkCount; i++){
            if(ptrChunk){
                uiMemOffSet = (i * ((unsigned int) m_sMemoryChunkSize)) ;
                ptrChunk->DataSize = (((unsigned int) m_sTotalMemoryPoolSize) - uiMemOffSet) ;
                ptrChunk = ptrChunk->Next ;
            }
            else{
                assert(false && "Error : ptrChunk == NULL") ;
                return false ;
            }
        }
        return true ;
    }

/******************
SetChunkDefaults
******************/
    SMemoryChunk *CMemoryPool::SetChunkDefaults(SMemoryChunk *ptrChunk){
        if(ptrChunk){
            ptrChunk->counter = 0;
            ptrChunk->name = "0";
            ptrChunk->type = "0";
            ptrChunk->Data = NULL ;
            ptrChunk->DataSize = 0 ;
            ptrChunk->UsedSize = 0 ;
            ptrChunk->IsAllocationChunk = false ;
            ptrChunk->Next = NULL ;
            ptrChunk->isReference = false;
            ptrChunk->isscope = false;
            ptrChunk->isstruct = false;
            ptrChunk->deap =0;
            ptrChunk->reference = nullptr;
            ptrChunk->mstruct.clear();
        }
        return ptrChunk ;
    }

    void CMemoryPool::SetChunktoDefault(SMemoryChunk *ptrChunk){
        if(ptrChunk){
            ptrChunk->counter = 0;
            ptrChunk->name = "0";
            ptrChunk->type = "0";
            ptrChunk->UsedSize = 0 ;
            ptrChunk->IsAllocationChunk = false ;
            ptrChunk->isReference = false;
            ptrChunk->isscope = false;
            ptrChunk->isstruct = false;
            ptrChunk->deap=0;
            ptrChunk->reference = nullptr;
            ptrChunk->mstruct.clear();
        }
    }
/******************
FindChunkHoldingPointerTo
******************/
    SMemoryChunk *CMemoryPool::FindChunkHoldingPointerTo(void *ptrMemoryBlock){
        SMemoryChunk *ptrTempChunk = m_ptrFirstChunk ;
        while(ptrTempChunk){
            if(ptrTempChunk->Data == ((TByte *) ptrMemoryBlock)){
                break ;
            }

            ptrTempChunk = ptrTempChunk->Next ;
        }
        return ptrTempChunk ;
    }

/******************
FindChunkHoldingNameTo
******************/

    SMemoryChunk *CMemoryPool::FindChunkHoldingNameTo(std::string name){
        SMemoryChunk *ptrTempChunk = CMemoryPool::m_ptrFirstChunk ;
        while(ptrTempChunk){
            if(ptrTempChunk->name == name){
                break ;
            }
            ptrTempChunk = ptrTempChunk->Next ;
        }
        return ptrTempChunk ;
    }

/******************
FindChunkHoldingNameTo
******************/

    bool CMemoryPool::FindChunkHoldingSameName(std::string name_) {
        SMemoryChunk *ptrTempChunk = m_ptrFirstChunk;
        bool can = true;
        while(ptrTempChunk){
            if(ptrTempChunk->name == name_){
                can = false;
                break ;
            }
            ptrTempChunk = ptrTempChunk->Next ;
        }

        return can;
    }

/******************
FreeAllAllocatedMemory
******************/
    void CMemoryPool::FreeAllAllocatedMemory(){
        SMemoryChunk *ptrChunk = m_ptrFirstChunk ;
        while(ptrChunk){
            SetChunktoDefault(ptrChunk);
            ptrChunk = ptrChunk->Next;
        }
        m_ptrCursorChunk = m_ptrFirstChunk;
    }

/******************
DeallocateAllChunks
******************/
    void CMemoryPool::DeallocateAllChunks(){
        SMemoryChunk *ptrChunk = m_ptrFirstChunk ;
        SMemoryChunk *ptrChunkToDelete = NULL ;
        while(ptrChunk){
            if(ptrChunk->IsAllocationChunk){
                if(ptrChunkToDelete){
                    free(((void *) ptrChunkToDelete)) ;
                }
                ptrChunkToDelete = ptrChunk ;
            }
            ptrChunk = ptrChunk->Next ;
        }
    }


    void CMemoryPool::Freeinscope(int deap){
        SMemoryChunk *ptrChunk = m_ptrFirstChunk ;
        while(ptrChunk){
            if (ptrChunk->isscope && deap == ptrChunk->deap){
                std::cout << "inscope" << endl;
                ptrChunk->counter = 0;
            }
            ptrChunk = ptrChunk->Next;
        }
    }

    void CMemoryPool::GarbageCollector(){
        SMemoryChunk *ptrChunk = m_ptrFirstChunk ;
        while(ptrChunk){
            if (ptrChunk->counter == 0){
                if (ptrChunk->isReference){
                    ptrChunk->reference->counter -=1;
                    if (ptrChunk->reference->counter < 0){
                        ptrChunk->reference->counter =0;
                    }
                    SetChunktoDefault(ptrChunk);
                }
            }
            ptrChunk = ptrChunk->Next;
        }
        ptrChunk = m_ptrFirstChunk ;
        while(ptrChunk) {
            if (ptrChunk->counter == 0){
                SetChunktoDefault(ptrChunk);
            }
            ptrChunk = ptrChunk->Next;
        }
        m_ptrCursorChunk = m_ptrFirstChunk;
    }
}


