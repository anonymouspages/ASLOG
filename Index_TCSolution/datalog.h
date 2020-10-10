#ifndef _DATALOG_H_
#define _DATALOG_H_
#include "btree.h"
#ifdef DATALOG_EXPORTS
#define DATALOG _declspec( dllexport )
#else
#define DATALOG _declspec(dllimport)
#endif
typedef struct oudui{
	struct oudui* next;
	char* x_char;
	char* y_char;
	int x;
	int y;
}OuDui;
//extern "C" DATALOG int Add(int a, int b);
extern "C" DATALOG BPTree_Index * CreateIndex(char* rule, int index_type, char* relations, char* attribute_list, int attribute_type);
extern "C" DATALOG OuDui * One_Person_AncestorsM(int person, BPTree_Index * btree_index);
extern "C" DATALOG void One_Person_AncestorsD(int person, BPTree_Index * btree_index);
extern "C" DATALOG OuDui * Person_AncestorsM(int* person_list, BPTree_Index * btree_index);
extern "C" DATALOG void Person_AncestorsD(int* person_list, BPTree_Index * btree_index);
extern "C" DATALOG OuDui * One_Person_descendantM(int person, BPTree_Index * btree_index);
extern "C" DATALOG void One_Person_descendantD(int person, BPTree_Index * btree_index);
extern "C" DATALOG OuDui * Person_descendantM(int* person_list, BPTree_Index * btree_index);
extern "C" DATALOG void Person_descendantD(int* person_list, BPTree_Index * btree_index);
extern "C" DATALOG OuDui * Transitive_closureM(BPTree_Index * btree_index);
extern "C" DATALOG void Transitive_closureD(BPTree_Index * btree_index);
extern "C" DATALOG OuDui * IncrementM(BPTree_Index * btree_index, char* increment);
extern "C" DATALOG void IncrementD(BPTree_Index * btree_index, char* increment);


#endif
#pragma once
