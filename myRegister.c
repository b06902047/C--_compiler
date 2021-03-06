#include <stdlib.h>
#include <string.h>
#include "myRegister.h"
#include "myIntVector.h"


char* zeroRegisterName = "$0";

char* intRegisterName[] = {
    "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23",
    "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15"
};

char* intWorkRegisterName[] = {"$24", "$25"};

char* floatRegisterName[] = {
    "$f20", "$f22", "$f24", "$f26", "$f28", "$f30",
    "$f2", "$f4", "$f6", "$f8", "$f10", "$f12", "$f14",
};

char* floatWorkRegisterName[] = {"$f16", "$f18"};

IntRegisterTable g_intRegisterTable;
FloatRegisterTable g_floatRegisterTable;
PseudoRegisterTable g_pseudoRegisterTable;
int g_pseudoRegisterBeginOffset = -4;

void initializeRegisterTable()
{
    g_pseudoRegisterTable.isAllocatedVector = getMyIntVector(10);
}


void resetRegisterTable(int maxLocalVariableOffset)
{
    memset(g_intRegisterTable.isAllocated, 0, sizeof(g_intRegisterTable.isAllocated));
    memset(g_floatRegisterTable.isAllocated, 0, sizeof(g_floatRegisterTable.isAllocated));
    memset(g_pseudoRegisterTable.isAllocatedVector->data, 0, sizeof(int) * g_pseudoRegisterTable.isAllocatedVector->capacity);
    g_pseudoRegisterTable.isAllocatedVector->size = 0;
    g_pseudoRegisterBeginOffset = maxLocalVariableOffset - 4;
}


int getRegister(ProcessorType processorType)
{
    int realTableIndex = 0;
    int realRegisterCount = (processorType == INT_REG) ? INT_REGISTER_COUNT : FLOAT_REGISTER_COUNT;
    int* realRegIsAllocated = (processorType == INT_REG) ? g_intRegisterTable.isAllocated : g_floatRegisterTable.isAllocated;
    for(realTableIndex= 0; realTableIndex < realRegisterCount; ++realTableIndex)
    {
        if(!realRegIsAllocated[realTableIndex])
        {
            realRegIsAllocated[realTableIndex] = 1;
            return realTableIndex;
        }
    }

    int pseudoTableIndex = 0;
    for(pseudoTableIndex = 0; pseudoTableIndex < g_pseudoRegisterTable.isAllocatedVector->size; ++pseudoTableIndex)
    {
        if(!g_pseudoRegisterTable.isAllocatedVector->data[pseudoTableIndex])
        {
            g_pseudoRegisterTable.isAllocatedVector->data[pseudoTableIndex] = 1;
            return (processorType == INT_REG) ? (INT_REGISTER_COUNT + pseudoTableIndex) : (FLOAT_REGISTER_COUNT + pseudoTableIndex);
        }
    }

    myPushBack(g_pseudoRegisterTable.isAllocatedVector, 1);

    return (processorType == INT_REG) ? (INT_REGISTER_COUNT + pseudoTableIndex) : (FLOAT_REGISTER_COUNT + pseudoTableIndex);
}

void freeRegister(ProcessorType processorType, int registerIndex)
{
    int realRegisterCount = (processorType == INT_REG) ? INT_REGISTER_COUNT : FLOAT_REGISTER_COUNT;
    int* realRegIsAllocated = (processorType == INT_REG) ? g_intRegisterTable.isAllocated : g_floatRegisterTable.isAllocated;
    
    if(registerIndex < realRegisterCount)
    {
        //free real register
        realRegIsAllocated[registerIndex] = 0;
    }
    else
    {
        //free pseudo register
        int pseudoTableIndex = registerIndex - realRegisterCount;
        g_pseudoRegisterTable.isAllocatedVector->data[pseudoTableIndex] = 0;
    }
}


void printStoreRegister(FILE* codeGenOutputFp)
{
    int index = 0;
    int tmpOffset = 4;
    for(index = 0; index < INT_REGISTER_COUNT; ++index)
    {
        fprintf(codeGenOutputFp, "sw %s, %d($sp)\n", intRegisterName[index], tmpOffset);
        tmpOffset += 4;
    }
    for(index = 0; index < INT_WORK_REGISTER_COUNT; ++index)
    {
        fprintf(codeGenOutputFp, "sw %s, %d($sp)\n", intWorkRegisterName[index], tmpOffset);
        tmpOffset += 4;
    }

    for(index = 0; index < FLOAT_REGISTER_COUNT; ++index)
    {
        fprintf(codeGenOutputFp, "s.s %s, %d($sp)\n", floatRegisterName[index], tmpOffset);
        tmpOffset += 4;
    }
    for(index = 0; index < FLOAT_WORK_REGISTER_COUNT; ++index)
    {
        fprintf(codeGenOutputFp, "s.s %s, %d($sp)\n", floatWorkRegisterName[index], tmpOffset);
        tmpOffset += 4;
    }
}


void printRestoreRegister(FILE* codeGenOutputFp)
{
    int index = 0;
    int tmpOffset = 4;
    for(index = 0; index < INT_REGISTER_COUNT; ++index)
    {
        fprintf(codeGenOutputFp, "lw %s, %d($sp)\n", intRegisterName[index], tmpOffset);
        tmpOffset += 4;
    }
    for(index = 0; index < INT_WORK_REGISTER_COUNT; ++index)
    {
        fprintf(codeGenOutputFp, "lw %s, %d($sp)\n", intWorkRegisterName[index], tmpOffset);
        tmpOffset += 4;
    }

    for(index = 0; index < FLOAT_REGISTER_COUNT; ++index)
    {
        fprintf(codeGenOutputFp, "l.s %s, %d($sp)\n", floatRegisterName[index], tmpOffset);
        tmpOffset += 4;
    }
    for(index = 0; index < FLOAT_WORK_REGISTER_COUNT; ++index)
    {
        fprintf(codeGenOutputFp, "l.s %s, %d($sp)\n", floatWorkRegisterName[index], tmpOffset);
        tmpOffset += 4;
    }
}

int getPseudoRegisterCorrespondingOffset(int pseudoRegisterIndex)
{
    return g_pseudoRegisterBeginOffset - pseudoRegisterIndex * 4;
}
