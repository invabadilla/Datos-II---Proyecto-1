//
// Created by usuario on 4/10/21.
//

#ifndef SERVER_CMEMORYPOOL_H
#define SERVER_CMEMORYPOOL_H

#include <string>
#include <vector>
#include "IMemoryBlock.h"
#include "SMemoryChunk.h"
#include "Variable.h"

namespace MemPool
{
    static const std::size_t DEFAULT_MEMORY_POOL_SIZE        = 1000 ;  //Size en Bytes por default de la memoria inicial del Pool
    static const std::size_t DEFAULT_MEMORY_CHUNK_SIZE       = 128 ;   //Size en Bytes por default de los Chunks
    static const std::size_t DEFAULT_MEMORY_SIZE_TO_ALLOCATE = DEFAULT_MEMORY_CHUNK_SIZE; //Size en Bytes por default a almacenar

    /**
     * Clase encargade de la asignacion y administracion de la memoria
     */
    class CMemoryPool : public IMemoryBlock{
    public:
        CMemoryPool(const size_t &sInitialMemoryPoolSize, const size_t &sMemoryChunkSize, const size_t &sMinimalMemorySizeToAllocate, bool bSetMemoryData);

        virtual ~CMemoryPool();

        virtual void *GetMemory(const size_t &sMemorySize);

        virtual void FreeMemory(void *ptrMemoryBlock);

        SMemoryChunk *FindChunkHoldingNameTo(std::string name);

        SMemoryChunk *FindChunkHoldingPointerTo(void *ptrMemoryBlock);

        bool FindChunkHoldingSameName(std::string name_);

        void FreeAllAllocatedMemory();

        void Freeinscope(int deap);

        void GarbageCollector();

    private:
        bool AllocateMemory(const size_t &sMemorySize);

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

        void SetChunktoDefault(SMemoryChunk *ptrChunk);

        SMemoryChunk *m_ptrFirstChunk; //Puntero al Chunk inical
        SMemoryChunk *m_ptrLastChunk; //Puntero al Chunk final
        SMemoryChunk *m_ptrCursorChunk; //Puntero al ultimo Chunk utilizado

        size_t m_sTotalMemoryPoolSize; //Size de la memoria total del Pool
        size_t m_sUsedMemoryPoolSize; //Size de la memoria utilizada del Pool
        size_t m_sFreeMemoryPoolSize; //Size de la memoria libre del Pool

        size_t m_sMemoryChunkSize; //Size de cada Chunk
        unsigned int m_uiMemoryChunkCount; //Contador del total de Chunks
        unsigned int m_uiObjectCount; //Contador de los Chunks utilizados

        bool m_bSetMemoryData; //Bool para establecer un espacio definido para el memset

    };

}
#endif //SERVER_CMEMORYPOOL_H
