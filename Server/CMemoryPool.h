//
// Created by usuario on 4/10/21.
//

#ifndef SERVER_CMEMORYPOOL_H
#define SERVER_CMEMORYPOOL_H

#include <string>
#include "IMemoryBlock.h"
#include "SMemoryChunk.h"

namespace MemPool
{
    static const std::size_t DEFAULT_MEMORY_POOL_SIZE        = 1000 ;                          //!< Initial MemoryPool size (in Bytes)
    static const std::size_t DEFAULT_MEMORY_CHUNK_SIZE       = 128 ;                           //!< Default MemoryChunkSize (in Bytes)
    static const std::size_t DEFAULT_MEMORY_SIZE_TO_ALLOCATE = DEFAULT_MEMORY_CHUNK_SIZE;

    class CMemoryPool : public IMemoryBlock{
    public:
        CMemoryPool(const size_t &sInitialMemoryPoolSize, const size_t &sMemoryChunkSize, const size_t &sMinimalMemorySizeToAllocate, bool bSetMemoryData);
        virtual ~CMemoryPool(); //Destructor

        virtual void *GetMemory(const size_t &sMemorySize);

        virtual void  FreeMemory(void  *ptrMemoryBlock, const size_t &sMemoryBlockSize);

        bool WriteMemoryDumpToFile(const std::string &strFileName);

        bool IsValidPointer(void *ptrPointer);

        SMemoryChunk *FindChunkHoldingNameTo(std::string name);
        SMemoryChunk *FindChunkHoldingPointerTo(void *ptrMemoryBlock);
        bool FindChunkHoldingSameName(std::string name_);

    private:
        bool AllocateMemory(const size_t &sMemorySize);
        void FreeAllAllocatedMemory();

        unsigned int CalculateNeededChunks(const size_t &sMemorySize);
        size_t CalculateBestMemoryBlockSize(const size_t &sRequestMemoryBlockSize);

        SMemoryChunk *FindChunkSuitableToHoldMemory(const size_t &sMemorySize);


        SMemoryChunk *SkipChunks(SMemoryChunk *ptrStartChunk, unsigned int uiChunksToSkip);
        SMemoryChunk *SetChunkDefaults(SMemoryChunk *ptrChunk);

        void FreeChunks(SMemoryChunk *ptrChunk);
        void DeallocateAllChunks();

        bool LinkChunksToData(SMemoryChunk *ptrNewChunk,unsigned int uiChunkCount, TByte *ptrNewMemBlock);
        void SetMemoryChunkValues(SMemoryChunk *ptrChunk, const size_t &sMemBlockSize);
        bool RecalcChunkMemorySize(SMemoryChunk *ptrChunks, unsigned int uiChunkCount);

        size_t MaxValue(const size_t &sValueA, const size_t &sValueB) const;

        SMemoryChunk *m_ptrFirstChunk;
        SMemoryChunk *m_ptrLastChunk;
        SMemoryChunk *m_ptrCursorChunk;

        size_t m_sTotalMemoryPoolSize;
        size_t m_sUsedMemoryPoolSize;
        size_t m_sFreeMemoryPoolSize;

        size_t m_sMemoryChunkSize;
        unsigned int m_uiMemoryChunkCount;
        unsigned int m_uiObjectCount;

        bool m_bSetMemoryData;
        size_t m_sMinimalMemorySizeToAllocate;



    };

}

#endif //SERVER_CMEMORYPOOL_H
