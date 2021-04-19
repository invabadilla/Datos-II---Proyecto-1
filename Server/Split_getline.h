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
    string myword = "";
    double total = 0;
    char final;
    for (int i=0; i < str.size(); i++){

        if (str[i] == '+') {
            try{
                if (myword.length() == to_string(stoi(myword)).length()){
                    total += stod(myword);
                    myword = "";
                    final = '+';
                }
                else{
                    stod("a");
                }
            }catch(std::invalid_argument){
                if(ptr_mpool->FindChunkHoldingSameName(myword)){
                    cout<< "No hay variable con ese nombre1";
                    return -1;
                }
                else{
                    MemPool::SMemoryChunk *ptrChunk =ptr_mpool->FindChunkHoldingNameTo(myword);
                    if (ptrChunk->type != "char" && ptrChunk->type != "struct" && !ptrChunk->isReference){
                        total += *ptrChunk->Data;
                    }
                        //eslseif(reference)
                    else{
                        cout<< "El tipo de la variable llamada no coincide con el operador usado";
                        return -1;
                    }

                }
                myword = "";
                final = '+';
            }

        }
        else if (str[i] == '-'){
            try{
                if (myword.length() == to_string(stoi(myword)).length()){
                    total -= stod(myword);
                    myword = "";
                    final = '-';
                }
                else{
                    stod("a");
                }
            }catch(std::invalid_argument){
                if(!ptr_mpool->FindChunkHoldingSameName(myword)){
                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingNameTo(myword);
                    if (ptrChunk->type != "char" && ptrChunk->type != "struct" && !ptrChunk->isReference){
                        total -= *ptrChunk->Data;
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
                myword = "";
                final = '-';
            }
        }
        else if (str[i] == '*'){
            try{
                if (myword.length() == to_string(stoi(myword)).length()){
                    total *= stod(myword);
                    myword = "";
                    final = '*';
                }
                else{
                    stod("a");
                }
            }catch(std::invalid_argument){
                if(!ptr_mpool->FindChunkHoldingSameName(myword)){
                    MemPool::SMemoryChunk *ptrChunk =ptr_mpool->FindChunkHoldingNameTo(myword);
                    if (ptrChunk->type != "char" && ptrChunk->type != "struct" && !ptrChunk->isReference){
                        total *= *ptrChunk->Data;
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
                myword = "";
                final = '*';
            }
        }
        else if (str[i] == '/'){
            try{
                if (myword.length() == to_string(stoi(myword)).length()){
                    total /= stod(myword);
                    myword = "";
                    final = '/';
                }
                else{
                    stod("a");
                }
            }catch(std::invalid_argument){
                if(!ptr_mpool->FindChunkHoldingSameName(myword)){
                    MemPool::SMemoryChunk *ptrChunk =ptr_mpool->FindChunkHoldingNameTo(myword);
                    if (ptrChunk->type != "char" && ptrChunk->type != "struct" && !ptrChunk->isReference){
                        total /= *ptrChunk->Data;
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
                myword = "";
                final = '/';
            }

        }
        else{
            myword = myword + str[i];
            cout<< i << ": "<< myword << endl;
            cout<<"Total: "<< total << endl;
        }
    }
    if (myword != ""){
        if (final == '+') {
            try{
                if (myword.length() == to_string(stoi(myword)).length()){
                    total += stod(myword);
                }
                else{
                    stod("a");
                }
            }catch(std::invalid_argument){
                if(!ptr_mpool->FindChunkHoldingSameName(myword)){
                    MemPool::SMemoryChunk *ptrChunk = ptr_mpool->FindChunkHoldingNameTo(myword);
                    if (ptrChunk->type != "char" && ptrChunk->type != "struct" && !ptrChunk->isReference){
                        total += *ptrChunk->Data;
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
                if (myword.length() == to_string(stoi(myword)).length()){
                    total -= stod(myword);
                }
                else{
                    stod("a");
                }
            }catch(std::invalid_argument){
                if(!ptr_mpool->FindChunkHoldingSameName(myword)){
                    MemPool::SMemoryChunk *ptrChunk =ptr_mpool->FindChunkHoldingNameTo(myword);
                    if (ptrChunk->type != "char" && ptrChunk->type != "struct" && !ptrChunk->isReference){
                        total -= *ptrChunk->Data;
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
                if (myword.length() == to_string(stoi(myword)).length()){
                    total *= stod(myword);
                }
                else{
                    stod("a");
                }
            }catch(std::invalid_argument){
                if(!ptr_mpool->FindChunkHoldingSameName(myword)){
                    MemPool::SMemoryChunk *ptrChunk =ptr_mpool->FindChunkHoldingNameTo(myword);
                    if (ptrChunk->type != "char" && ptrChunk->type != "struct" && !ptrChunk->isReference){
                        total *= *ptrChunk->Data;
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
                if (myword.length() == to_string(stoi(myword)).length()){
                    total /= stod(myword);
                }
                else{
                    stod("a");
                }
            }catch(std::invalid_argument){
                if(!ptr_mpool->FindChunkHoldingSameName(myword)){
                    MemPool::SMemoryChunk *ptrChunk =ptr_mpool->FindChunkHoldingNameTo(myword);
                    if (ptrChunk->type != "char" && ptrChunk->type != "struct"&& !ptrChunk->isReference){
                        total /= *ptrChunk->Data;
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
        else{
            if(!ptr_mpool->FindChunkHoldingSameName(myword)){
                MemPool::SMemoryChunk *ptrChunk =ptr_mpool->FindChunkHoldingNameTo(myword);
                if (ptrChunk->type != "char" && ptrChunk->type != "struct"&& !ptrChunk->isReference){
                    total = *ptrChunk->Data;
                }
                    //eslseif(reference)
                else{
                    cout<< "El tipo de la variable llamada no coincide con el operador usado";
                    return -1;
                }
            }
            else{
                cout<< "No hay variable con ese nombre o valor no coincide con tipo de variable";
                return -1;
            }
        }
    }

    else{
        cout<< "Exrepresion esperada despues del ultimo operador";
        return -1;
    }
    cout<< total << endl;
    return total;
}