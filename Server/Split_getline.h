//
// Created by usuario on 4/14/21.
//

#ifndef SERVER_SPLIT_GETLINE_H
#define SERVER_SPLIT_GETLINE_H

#endif //SERVER_SPLIT_GETLINE_H

#include <iostream>
#include <string>
#include "CMemoryPool.h"
#include "json.hpp"

using namespace std;

double split_getline(string str,  MemPool::CMemoryPool *ptr_mpool ) {
    string myword;
    double total;
    char final;
    for (int i=0; i < str.size(); i++){

        if (str[i] == '+') {
            try{
                total += stod(myword);
            }catch(std::invalid_argument){
                cout<<myword;
                if(ptr_mpool->FindChunkHoldingSameName(myword, ptr_mpool)){
                    cout<< "No hay variable con ese nombre1";
                    return -1;
                }
                else{
                    MemPool::SMemoryChunk ptrChunk =*ptr_mpool->FindChunkHoldingNameTo(myword);
                    if (ptrChunk.type != "char" && ptrChunk.type != "struct"){
                        total += *ptrChunk.Data;
                    }
                        //eslseif(reference)
                    else{
                        cout<< "El tipo de la variable llamada no coincide con el operador usado";
                        return -1;
                    }

                }
            }
            myword = "";
            final = '+';
        }
        else if (str[i] == '-'){
            try{
                total -= stod(myword);
            }catch(std::invalid_argument){
                if(!ptr_mpool->FindChunkHoldingSameName(myword, ptr_mpool)){
                    MemPool::SMemoryChunk ptrChunk = *ptr_mpool->FindChunkHoldingNameTo(myword);
                    if (ptrChunk.type != "char" && ptrChunk.type != "struct"){
                        total -= *ptrChunk.Data;
                    }
                        //eslseif(reference)
                    else{
                        cout<< "El tipo de la variable llamada no coincide con el operador usado";
                        return -1;
                    }
                }
                else{
                    cout<< "No hay variable con ese nombre";
                    return -1;
                }
            }
            myword = "";
            final = '-';
        }
        else if (str[i] == '*'){
            try{
                total *= stod(myword);
            }catch(std::invalid_argument){
                if(!ptr_mpool->FindChunkHoldingSameName(myword, ptr_mpool)){
                    MemPool::SMemoryChunk ptrChunk =*ptr_mpool->FindChunkHoldingNameTo(myword);
                    if (ptrChunk.type != "char" && ptrChunk.type != "struct"){
                        total *= *ptrChunk.Data;
                    }
                        //eslseif(reference)
                    else{
                        cout<< "El tipo de la variable llamada no coincide con el operador usado";
                        return -1;
                    }
                }
                else{
                    cout<< "No hay variable con ese nombre";
                    return -1;
                }
            }
            myword = "";
            final = '*';
        }
        else if (str[i] == '/'){
            try{
                total /= stod(myword);
            }catch(std::invalid_argument){
                if(!ptr_mpool->FindChunkHoldingSameName(myword, ptr_mpool)){
                    MemPool::SMemoryChunk ptrChunk =*ptr_mpool->FindChunkHoldingNameTo(myword);
                    if (ptrChunk.type != "char" && ptrChunk.type != "struct"){
                        total /= *ptrChunk.Data;
                    }
                        //eslseif(reference)
                    else{
                        cout<< "El tipo de la variable llamada no coincide con el operador usado";
                        return -1;
                    }
                }
                else{
                    cout<< "No hay variable con ese nombre";
                    return -1;
                }
            }
            myword = "";
            final = '/';
        }
        else{
            myword = myword + str[i];
        }
    }
    if (myword != ""){
        if (final == '+') {
            try{
                total += stod(myword);
            }catch(std::invalid_argument){
                if(!ptr_mpool->FindChunkHoldingSameName(myword, ptr_mpool)){
                    MemPool::SMemoryChunk ptrChunk =*ptr_mpool->FindChunkHoldingNameTo(myword);
                    if (ptrChunk.type != "char" && ptrChunk.type != "struct"){
                        total /= *ptrChunk.Data;
                    }
                        //eslseif(reference)
                    else{
                        cout<< "El tipo de la variable llamada no coincide con el operador usado";
                        return -1;
                    }
                }
                else{
                    cout<< "No hay variable con ese nombre";
                    return -1;
                }
            }
        }
        else if (final == '-'){
            try{
                total -= stod(myword);
            }catch(std::invalid_argument){
                if(!ptr_mpool->FindChunkHoldingSameName(myword, ptr_mpool)){
                    MemPool::SMemoryChunk ptrChunk =*ptr_mpool->FindChunkHoldingNameTo(myword);
                    if (ptrChunk.type != "char" && ptrChunk.type != "struct"){
                        total /= *ptrChunk.Data;
                    }
                        //eslseif(reference)
                    else{
                        cout<< "El tipo de la variable llamada no coincide con el operador usado";
                        return -1;
                    }
                }
                else{
                    cout<< "No hay variable con ese nombre";
                    return -1;
                }
            }
        }
        else if (final == '*'){
            try{
                total *= stod(myword);
            }catch(std::invalid_argument){
                if(!ptr_mpool->FindChunkHoldingSameName(myword, ptr_mpool)){
                    MemPool::SMemoryChunk ptrChunk =*ptr_mpool->FindChunkHoldingNameTo(myword);
                    if (ptrChunk.type != "char" && ptrChunk.type != "struct"){
                        total /= *ptrChunk.Data;
                    }
                        //eslseif(reference)
                    else{
                        cout<< "El tipo de la variable llamada no coincide con el operador usado";
                        return -1;
                    }
                }
                else{
                    cout<< "No hay variable con ese nombre";
                    return -1;
                }
            }
        }
        else if (final == '/'){
            try{
                total /= stod(myword);
            }catch(std::invalid_argument){
                if(!ptr_mpool->FindChunkHoldingSameName(myword, ptr_mpool)){
                    MemPool::SMemoryChunk ptrChunk =*ptr_mpool->FindChunkHoldingNameTo(myword);
                    if (ptrChunk.type != "char" && ptrChunk.type != "struct"){
                        total /= *ptrChunk.Data;
                    }
                        //eslseif(reference)
                    else{
                        cout<< "El tipo de la variable llamada no coincide con el operador usado";
                        return -1;
                    }
                }
                else{
                    cout<< "No hay variable con ese nombre";
                    return -1;
                }
            }
        }
    }
    else{
        cout<< "Exrepresion esperada despues del ultimo operador";
        return -1;

    }
    cout<< total;
    return total;
}
