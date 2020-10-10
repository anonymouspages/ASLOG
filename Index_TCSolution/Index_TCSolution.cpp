// AllPersonAncestor.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h" 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <stdbool.h>
#include<windows.h>
#include <sys/timeb.h>
//#include <sys/mman.h>  
#include "mmap.h" 
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "btree.h"
#include "datalog.h"

//#define VERTEX 270000
#define BUFFSIZE 1024
#define _CRT_SECURE_NO_DEPRECATE

//calculate the time
int co = 0;
struct timeval starttime, endtime;
double timeuse;
extern int alloc_count;
comp* c = (comp*)malloc(sizeof(comp));


//所有人祖先结果不再放入这个链表中
/*typedef struct Succ{
	int successor;
	struct Succ *next;
}Succ_Linklist;  //后继链表
struct retult{
	Succ_Linklist *head;  //后继链表头结点
}result[VERTEX];  //所有点的后继链表*/

//求所有人祖先结果链表（类似二维表格）
typedef struct Ancestors {
	int person;
	int ancestor;
	struct Ancestors* next;
}Ancestors_Linklist;
Ancestors_Linklist* head_Ancestor;

//btree_node *root;
//index_head *infor;


/*
* 鎻掑叆鏃舵敞鎰忓弬鏁扮被鍨嬶紝0锛氭暣鍨嬶紝1锛氭诞鐐瑰瀷锛?锛氬瓧绗︿覆
*
*/
//#include "stdafx.h"
//#include "btree.h"
//#include "mmap.h" 
//#include<windows.h>
int alloc_count = 0;
btree_node* last_leaf_node;
btree_node* head_leaf;
//int count = 1;
long int count1 = 0, count2 = 0;

long int btnode_count = 0, btrecord_count = 0, key_type_count = 0, L_Keypointer_Node_count = 0;
//make a empty leaf as the head leaf
void* alloc_add(int length, int type, index_head* infor, key_type* key) //allocate the free address
{
	block_node* next = (block_node*)infor->block[type].next, * add;
	//	if(type==0)printf("%d ",next); 
	if (next != NULL) {
		if (type == 4) {
			alloc_count += 5;
			//			printf("%d  ",next);
		}
		add = (block_node*)((long int)next + (long int)infor->base);
		//	if(next == 188){next = (block_node *)add;if(next->next==0)next->next=256;}
		next = (block_node*)add;
		infor->block[type].next = next->next;//delete the block
	}
	else {

		add = (block_node*)((long int)infor->offset + (long int)infor->base);
		infor->offset = infor->offset + (long int)length;
	}
	return add;
}
void initialize_keys(key_type* keys, key_type* key_input, int keys_type, key_type* tkey) {
	if (keys_type == 0)//the key type is interger 
	{
		keys->ivalue = key_input->ivalue;

		keys->Keypointer = NULL;  //指向和当前块x相同的原表某些行 的链表的头结点（不为空） 
		keys->KeyYpointer = NULL;
		//keys->L_KeyYpointer = NULL;  //指向和当前块y相同的原表某些行 的链表的头结点（不为空）
		//keys->L_Tuplepointer = NULL;
		//keys->L_TupleYpointer = NULL;//五类指针  
		//	keys->L_TupleZpointer = NULL;
	}
}
int str_cmp(key_type* a, key_type* b, int keys_type) {
	if (keys_type == 0) {
		return a->ivalue - b->ivalue;
	}
}
int get_key_len(key_type* key, int type) {//get the length of the key 
	return sizeof(key_type);
}
index_head* create_index(char* database_name, char* table_name, char* key_name, int key_type, int count, char* file_name) {
	//create the index file; 
	int file_len, i;
	index_head* infor = NULL;
	strcpy(file_name, "../");
	strcat(file_name, database_name);
	strcat(file_name, "/");
	strcat(file_name, table_name);
	strcat(file_name, ".");
	strcat(file_name, key_name);
	strcat(file_name, ".idx");
	/*file_name = "student.no.idx";*/
	file_len = count * 90;//    
	//int fd = open(file_name, O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
	fp = fopen(file_name, "rt+"); //allow read and write
	if (fp == NULL)
	{
		printf("open error\n");
		return 0;
	}
	fseek(fp, file_len - 1, SEEK_SET);
	fwrite("1", 4, 1, fp);
	fclose(fp);

	infor = (index_head*)open_mmap(file_name);
	infor->base = (void*)infor;
	infor->keys_type = key_type;//0:int 1:double 2:char
	infor->root = NULL;
	infor->node_count = 0;
	infor->offset = sizeof(index_head);//this is the last btree_node;
	for (i = 0; i < BLOCK_TYPE; i++)
		infor->block[i].next = NULL;
	infor->block[0].block_size = sizeof(btree_node);
	infor->block[1].block_size = btree_size * sizeof(void*);
	infor->block[2].block_size = (btree_size - 1) * sizeof(void*);
	infor->block[3].block_size = sizeof(btree_record);
	infor->block[4].block_size = 4;//the length of key
	return infor;
}
btree_node* find_leaf(btree_node* root, key_type* key, index_head* infor) //find the proper location for the key
{
	btree_node* nd;
	key_type** nd_keys;
	int i, count = 0;
	void** nd_pointers;
	if (root == NULL)
		return root;
	nd = root;
	while (!nd->is_leaf) {
		nd_keys = (key_type**)((long int)nd->keys + (long int)infor->base);
		nd_pointers = (void**)((long int)nd->pointers + (long int)infor->base);
		for (i = 0; i < nd->num_keys && str_cmp((key_type*)((long int)nd_keys[i] + (long int)infor->base), key, infor->keys_type) <= 0; i++, count++);
		nd = (btree_node*)((long int)nd_pointers[i] + (long int)infor->base);	 ///(btree_node *)nd->pointers[i]

	}
	return nd;
}
btree_record* make_new_btree_record(int value, index_head* infor, key_type* key) {
	btree_record* rec;
	rec = (btree_record*)alloc_add(sizeof(btree_record), 3, infor, key); ///(btree_record *)malloc(sizeof(btree_record))
	rec->value = value;
	return rec;
}
btree_node* make_new_btree_node(index_head* infor, key_type* key) {
	btree_node* nd;

	nd = (btree_node*)alloc_add(sizeof(btree_node), 0, infor, key);   ///(btree_node *)malloc(sizeof(btree_node))   
	nd->pointers = (void**)((long int)alloc_add(btree_size * sizeof(void*), 1, infor, key) - (long int)infor->base);//, key);///malloc(size * sizeof(void *))
	nd->keys = (key_type**)((long int)alloc_add((btree_size - 1) * sizeof(key_type*), 2, infor, key) - (long int)infor->base);//, key); ///malloc((size - 1) * sizeof(char *))
	nd->parent = NULL;
	nd->next_leaf_node = NULL;
	nd->num_keys = 0;
	nd->is_leaf = false;
	return nd;
}
btree_node* make_new_leaf(index_head* infor, key_type* key) {
	btree_node* leaf;
	leaf = make_new_btree_node(infor, key);
	leaf->is_leaf = true;
	last_leaf_node->next_leaf_node = leaf;
	last_leaf_node = leaf;
	return leaf;
}
btree_node* make_new_tree(key_type* key, int value, index_head* infor) {
	btree_node* root;
	btree_record* rec;
	root = make_new_leaf(infor, key);
	int type_len = get_key_len(key, infor->keys_type);
	void** root_pointers = (void**)((long int)root->pointers + (long int)infor->base);
	key_type** root_keys = (key_type**)((long int)root->keys + (long int)infor->base);
	rec = make_new_btree_record(value, infor, key);
	root_pointers[0] = (void*)((long int)rec - (long int)infor->base); //store the offset of rec
	root_keys[0] = (key_type*)((long int)alloc_add(type_len, 4, infor, key) - (long int)infor->base);//, key); /// malloc(MAX_KEY_LEN) 
	key_type* temp = (key_type*)((long int)root_keys[0] + (long int)infor->base);

	initialize_keys(temp, key, infor->keys_type, key); /// strcpy(root->keys[0], key)
	root_pointers[btree_size - 1] = NULL; ///root->pointers[size-1]
	root->num_keys++;
	return root;
}
btree_node* make_new_root(btree_node* left, btree_node* right, key_type* key, index_head* infor, key_type* tkey) {
	btree_node* root;
	root = make_new_btree_node(infor, key);
	long int base = (long int)infor->base;
	int type_len = get_key_len(key, infor->keys_type);
	void** root_pointers = (void**)((long int)root->pointers + base);
	key_type** root_keys = (key_type**)((long int)root->keys + base);
	root_pointers[0] = (void*)((long int)left - base); /// root->pointers[0] = left; 
	root_pointers[1] = (void*)((long int)right - base); ///root->pointers[1] = right;
	root_keys[0] = (key_type*)((long int)alloc_add(type_len, 4, infor, key) - base); /// malloc(MAX_KEY_LEN)
	key_type* temp = (key_type*)((long int)root_keys[0] + base);
	initialize_keys(temp, key, infor->keys_type, tkey);
	root->num_keys++;
	left->parent = (btree_node*)((long int)root - base); ///root
	right->parent = (btree_node*)((long int)root - base); ///root
	return root;
}

void init_head_leaf(index_head* infor)
{
	key_type* head_leaf_key = (key_type*)malloc(MAX_KEY_LEN);
	key_type_count++;
	head_leaf_key->ivalue = 0;
	head_leaf = make_new_btree_node(infor, head_leaf_key);  //make a empty leaf as the head leaf
	head_leaf->is_leaf = true;
	head_leaf->next_leaf_node = NULL;  //初始化空头叶子节点的next_leaf_node指针
	last_leaf_node = head_leaf;
}
btree_node* btree_insert(btree_node* root, key_type* key, int value, index_head* infor) {

	btree_record* rec;
	btree_node* leaf;
	int index, cond;

	leaf = find_leaf(root, key, infor);  //////////////////////////////////////////////////////////

	if (!leaf) {  // cannot find the leaf, the tree is empty 
		infor->node_count++;
		return  make_new_tree(key, value, infor);
	}

	key_type** leaf_keys = (key_type**)((long int)leaf->keys + (long int)infor->base);
	key_type* temp = (key_type*)((long int)leaf_keys[0] + (long int)infor->base);
	for (index = 0; index < leaf->num_keys && (cond = str_cmp((key_type*)((long int)leaf_keys[index] + (long int)infor->base), key, infor->keys_type)) < 0; index++);//strcmp(leaf->keys[index], key)

	if (cond == 0) {  // ignore duplicates,here we can change to store the duplicates by using the linklist
		return root;
	}
	infor->node_count++;
	rec = make_new_btree_record(value, infor, key);
	if (leaf->num_keys < btree_size - 1) {

		insert_into_leaf(leaf, index, key, rec, infor);
		return root;  // the root remains unchanged
	}
	return insert_into_leaf_after_splitting(root, leaf, index, key, rec, infor);//////////////////////////////////////////////////////////////
}

btree_node* insert_into_parent(btree_node* root, btree_node* left, btree_node* right, key_type* key, index_head* infor, key_type* tkey) {

	btree_node* parent;
	int index, i;
	parent = (btree_node*)left->parent;
	if (parent == NULL) {
		return make_new_root(left, right, key, infor, tkey);
	}
	parent = (btree_node*)((long int)left->parent + (long int)infor->base);  ///parent = left->parent;
	key_type** parent_pointers = (key_type**)((long int)parent->pointers + (long int)infor->base);
	for (index = 0; index < parent->num_keys && parent_pointers[index] != (key_type*)((long int)left - (long int)infor->base); index++);  ///parent->pointers[index] != left
	if (parent->num_keys < btree_size - 1) {
		insert_into_btree_node(parent, right, index, key, infor);
		return root;  // the root remains unchanged
	}
	return insert_into_btree_node_after_splitting(root, parent, right, index, key, infor);
}

void insert_into_btree_node(btree_node* nd, btree_node* right, int index, key_type* key, index_head* infor) {
	int i;
	void** nd_pointers = (void**)((long int)nd->pointers + (long int)infor->base);
	key_type** nd_keys = (key_type**)((long int)nd->keys + (long int)infor->base);
	int type_len = get_key_len(key, infor->keys_type);
	for (i = nd->num_keys; i > index; i--) {
		nd_keys[i] = nd_keys[i - 1];  ///nd->keys[i] = nd->keys[i-1]
		nd_pointers[i + 1] = nd_pointers[i]; ///nd->pointers[i+1] = nd->pointers[i];
	}
	nd_keys[index] = (key_type*)((long int)alloc_add(type_len, 4, infor, key) - (long int)infor->base); /// malloc(MAX_KEY_LEN)
	key_type* temp = (key_type*)((long int)nd_keys[index] + (long int)infor->base);
	initialize_keys(temp, key, infor->keys_type, key);
	nd_pointers[index + 1] = (void*)((long int)right - (long int)infor->base);  ///nd->pointers[index+1]
	nd->num_keys++;
}

btree_node* insert_into_btree_node_after_splitting(btree_node* root, btree_node* nd, btree_node* right, int index, key_type* key, index_head* infor) {
	int i, split;
	btree_node** temp_ps, * new_nd, * child;
	key_type** temp_ks, * new_key;
	void** nd_pointers = (void**)((long int)nd->pointers + (long int)infor->base);
	key_type** nd_keys = (key_type**)((long int)nd->keys + (long int)infor->base);
	int type_len = get_key_len(key, infor->keys_type);
	void** new_nd_p;
	key_type** new_nd_k;
	temp_ps = (btree_node**)malloc((btree_size + 1) * sizeof(btree_node*));
	btnode_count++;
	temp_ks = (key_type**)malloc(btree_size * sizeof(key_type*));
	key_type_count++;
	for (i = 0; i < btree_size + 1; i++) {
		if (i == index + 1)
			temp_ps[i] = (btree_node*)((long int)right - (long int)infor->base);
		else if (i < index + 1)
			temp_ps[i] = (btree_node*)nd_pointers[i];  ///nd->pointers[i]
		else
			temp_ps[i] = (btree_node*)nd_pointers[i - 1];  ///nd->pointers[i-1]
	}
	for (i = 0; i < btree_size; i++) {
		if (i == index) {
			temp_ks[i] = (key_type*)((long int)alloc_add(type_len, 4, infor, key) - (long int)infor->base); /// malloc(MAX_KEY_LEN)
			key_type* temp = (key_type*)((long int)temp_ks[i] + (long int)infor->base);
			initialize_keys(temp, key, infor->keys_type, key);
		}
		else if (i < index)
			temp_ks[i] = nd_keys[i];
		else
			temp_ks[i] = nd_keys[i - 1];
	}
	split = btree_size % 2 ? btree_size / 2 + 1 : btree_size / 2;  // split is #pointers
	nd->num_keys = split - 1;
	for (i = 0; i < split - 1; i++) {
		nd_pointers[i] = temp_ps[i];
		nd_keys[i] = temp_ks[i];
	}
	nd_pointers[i] = temp_ps[i];  // i = split - 1
	new_key = (key_type*)((long int)temp_ks[split - 1] + (long int)infor->base);

	new_nd = make_new_btree_node(infor, key);
	new_nd_p = (void**)((long int)new_nd->pointers + (long int)infor->base);
	new_nd_k = (key_type**)((long int)new_nd->keys + (long int)infor->base);
	new_nd->num_keys = btree_size - split;
	for (++i; i < btree_size; i++) {
		new_nd_p[i - split] = temp_ps[i];////
		new_nd_k[i - split] = temp_ks[i];

	}
	new_nd_p[i - split] = temp_ps[i];
	new_nd->parent = nd->parent;
	for (i = 0; i <= new_nd->num_keys; i++) {  //  #pointers = num_keys + 1
		child = (btree_node*)((long int)new_nd_p[i] + (long int)infor->base);
		child->parent = (btree_node*)((long int)new_nd - (long int)infor->base);
	}
	free(temp_ps);
	free(temp_ks);

	return insert_into_parent(root, nd, new_nd, new_key, infor, key);
}

void insert_into_leaf(btree_node* leaf, int index, key_type* key, btree_record* rec, index_head* infor) // insert the btree_node to the leaf without spliting
{
	int i;
	void** leaf_pointers = (void**)((long int)leaf->pointers + (long int)infor->base);
	key_type** leaf_keys = (key_type**)((long int)leaf->keys + (long int)infor->base);
	int type_len = get_key_len(key, infor->keys_type);
	for (i = leaf->num_keys; i > index; i--) {
		leaf_keys[i] = leaf_keys[i - 1];
		leaf_pointers[i] = leaf_pointers[i - 1];
	}
	leaf_keys[index] = (key_type*)((long int)alloc_add(type_len, 4, infor, key) - (long int)infor->base); /// malloc(MAX_KEY_LEN) 
	key_type* temp = (key_type*)((long int)leaf_keys[index] + (long int)infor->base);
	initialize_keys(temp, key, infor->keys_type, key);
	//printf("keys %d\n", key->ivalue);
	leaf_pointers[index] = (btree_record*)((long int)rec - (long int)infor->base);
	leaf->num_keys++;
}

btree_node* insert_into_leaf_after_splitting(btree_node* root, btree_node* leaf, int index, key_type* key, btree_record* rec, index_head* infor) {
	btree_node* new_leaf;
	btree_record** temp_ps;
	key_type** temp_ks, * new_key;
	int i, split;
	void** leaf_pointers = (void**)((long int*)leaf->pointers + (long int)infor->base);
	key_type** leaf_keys = (key_type**)((long int)leaf->keys + (long int)infor->base);
	int type_len = get_key_len(key, infor->keys_type);
	void** new_leaf_pointers;
	key_type** new_leaf_keys;
	temp_ps = (btree_record**)malloc(btree_size * sizeof(btree_record*));
	btrecord_count++;
	temp_ks = (key_type**)malloc(btree_size * sizeof(key_type*));
	key_type_count++;
	for (i = 0; i < btree_size; i++) {
		if (i == index) {
			temp_ps[i] = rec;
			temp_ks[i] = (key_type*)alloc_add(type_len, 4, infor, key);
			initialize_keys(temp_ks[i], key, infor->keys_type, key);
		}
		else if (i < index) {
			temp_ps[i] = (btree_record*)((long int)leaf_pointers[i] + (long int)infor->base);
			temp_ks[i] = (key_type*)((long int)leaf_keys[i] + (long int)infor->base);
		}
		else {
			temp_ps[i] = (btree_record*)((long int)leaf_pointers[i - 1] + (long int)infor->base);
			temp_ks[i] = (key_type*)((long int)leaf_keys[i - 1] + (long int)infor->base);
		}
	}
	split = btree_size / 2;
	leaf->num_keys = split;
	for (i = 0; i < split; i++) {
		leaf_pointers[i] = (void**)((long int)temp_ps[i] - (long int)infor->base);
		leaf_keys[i] = (key_type*)((long int)temp_ks[i] - (long int)infor->base);
	}
	new_leaf = make_new_leaf(infor, key);
	new_leaf->num_keys = btree_size - split;
	new_leaf_pointers = (void**)((long int)new_leaf->pointers + (long int)infor->base);
	new_leaf_keys = (key_type**)((long int)new_leaf->keys + (long int)infor->base);
	for (; i < btree_size; i++) {
		new_leaf_pointers[i - split] = (void**)((long int)temp_ps[i] - (long int)infor->base);
		new_leaf_keys[i - split] = (key_type*)((long int)temp_ks[i] - (long int)infor->base);
	}
	new_leaf->parent = leaf->parent;
	new_leaf_pointers[btree_size - 1] = leaf_pointers[btree_size - 1];
	leaf_pointers[btree_size - 1] = (void**)((long int)new_leaf - (long int)infor->base);
	free(temp_ps);
	free(temp_ks);
	new_key = (key_type*)((long int)new_leaf_keys[0] + (long int)infor->base);
	return insert_into_parent(root, leaf, new_leaf, new_key, infor, key);/////////////////////////////////////////////////////
}
/********************************求解传递闭包*********************************/

//int A[ARC_NUM + 1][2];  //存放边的表A
//bool  reach[VERTEX_NUM + 1][VERTEX_NUM + 1] = { false };  //两点之间直接可达值为1 
IndexTable* L, * last;
//L_TupleYpointer_Node *TupleYpointer_last;
//L_TupleZpointer_Node *TupleZpointer_last;
//L_Keypointer_Node *Keypointer_last;//, *KeyYpointer_last;
struct Map* Y;  //压缩图邻接表空表头
//bool visit[VERTEX_NUM + 1] = { false };  //初始化每个点访问标记  
//int CHeight[VERTEX_NUM + 1] = { 0 };
//int ID[VERTEX_NUM + 1] = { 0 };  //每个点访问顺序
//int Ltfl[VERTEX_NUM + 1] = { -1 }; //-1代表所有点现在还不属于某个分量
int id = 0;
int ar_num = 0;
int num = 0;

/***************栈操作******************/
int popNS() {
	if (ns.top >= 0) {
		int elem = ns.ns[ns.top];
		ns.top--;
		return elem;
	}
}

void pushNS(int elem) {
	ns.ns[ns.top + 1] = elem;
	ns.top++;
}

int popCS() {
	if (cs.top >= 0) {
		int elem = cs.cs[cs.top];
		cs.top--;
		return elem;
	}
}

void pushCS(int elem) {
	cs.cs[cs.top + 1] = elem;
	cs.top++;
}

int heightCS() {
	return cs.top;
}
int popset1s() {
	if (set1s.top >= 0) {
		int elem = set1s.set1s[set1s.top];
		set1s.top--;
		return elem;
	}
}

void pushset1s(int elem) {
	set1s.set1s[set1s.top + 1] = elem;
	set1s.top++;
}

int heightset1s() {
	return set1s.top;
}
int popset2s() {
	if (set2s.top >= 0) {
		int elem = set2s.set2s[set2s.top];
		set2s.top--;
		return elem;
	}
}

void pushset2s(int elem) {
	set2s.set2s[set2s.top + 1] = elem;
	set2s.top++;
}

int heightset2s() {
	return set2s.top;
}
void pushset3s(int elem) {
	set3s.set3s[set3s.top + 1] = elem;
	set3s.top++;
}
int popset3s() {
	if (set3s.top >= 0) {
		int elem = set3s.set3s[set3s.top];
		set3s.top--;
		return elem;
	}
}
int heightset3s() {
	return set3s.top;
}
/****************栈操作******************/


key_type* Search_block(int x, btree_node* root, index_head* infor)
{//查找指定顶点所在的块  （没问题）
	long int base = (long int)infor->base;
	int i;
	btree_node* p = root;
	key_type** p_keys, * temp;
	void** p_pointers;
	if (p == NULL) return NULL;
	else {
		while (!p->is_leaf)
		{
			p_pointers = (void**)((long int)p->pointers + (long int)base);
			p_keys = (key_type**)((long int)p->keys + (long int)base);
			for (i = 0; i < p->num_keys; i++)
			{
				//printf("内部%d\n", p->num_keys);
				temp = (key_type*)((long int)p_keys[i] + (long int)base);
				if (temp->ivalue == x)
				{
					i++;
					break;
				}
				if (temp->ivalue > x) break;
			}
			p = (btree_node*)((long int)p_pointers[i] + (long int)base);
		}
		if (p->is_leaf)
		{
			p_keys = (key_type**)((long int)p->keys + (long int)base);
			for (i = 0; i < p->num_keys; i++)
			{
				//printf("叶子%d\n", p->num_keys);
				temp = (key_type*)((long int)p_keys[i] + (long int)base);
				if (temp->ivalue == x) { return temp; }
			}
			return NULL;
		}
	}
}
void SetIndex_transitive(char* rule, char* realtions, char attribute_list[], int attribute_list_length, BPTree_Index* btree_index)
{//成功地把图从文件中读入表A (没问题)
	int r = 1, i, j;
	btree_node* root = btree_index->root;
	index_head* infor = btree_index->infor;

	/***********初始化************/
	for (i = 0; i <= ARC_NUM; i++)
	{
		array_list[i].x = -1;
		array_list[i].y = -1;
		array_list[i].Forpointer = NULL;
		array_list[i].Repointer = NULL;
	}
	for (i = 0; i < VERTEX_NUM; i++)
	{
		arindex[i].v = -1;
		arindex[i].v_Forpointer = NULL;
	}
	array_list[0].x = (int)rule;  //预留空间保存规则路径

	key_type* k = NULL;
	fp = fopen(realtions, "r");
	char str[15];    //存从文件中读入的一行字符串
	if (fp == NULL)
	{
		printf("打开文件失败\n");
	}
	i = 0;
	//FILE* file = NULL;
	//file = fopen(pathStr, "r");

	char buff[BUFFSIZE];
	char* token = NULL;
	int v, w;

	key_type** p_keys, * temp;
	p_keys = (key_type**)((long int)root->keys + (long int)infor->base);
	temp = (key_type*)((long int)p_keys[0] + (long int)infor->base);

	while (fgets(buff, 81, fp))
	{
		sscanf(buff, "%*[<]%d,%d%*[>]", &v, &w);
		array_list[r].x = v;
		array_list[r].y = w;

		k = Search_block(array_list[r].x, root, infor);


		if (!isexit_array_index[array_list[r].x])
		{
			isexit_array_index[array_list[r].x] = 1;
			arindex[ar_num].v = array_list[r].x;
			arindex[ar_num].v_Forpointer = k;
			ar_num++;
		}

		array_list[r].Forpointer = Search_block(array_list[r].y, root, infor);
		array_list[r].Repointer = Search_block(array_list[r].x, root, infor);

		//array_list[r].Repointer = k; 

		L_Keypointer_Node* q;
		q = (L_Keypointer_Node*)malloc(sizeof(L_Keypointer_Node)); L_Keypointer_Node_count++;
		q->array_row = r;

		if (k->Keypointer == NULL)
		{
			//printf("） %d %d\n", r, temp->ivalue);
			q->next = NULL;
			k->Keypointer = q;
			k->Keypointer_last = q;
		}
		else
		{
			q->next = k->Keypointer_last->next;
			k->Keypointer_last->next = q;
			k->Keypointer_last = q;
		}

		k = Search_block(array_list[r].y, root, infor);
		L_Keypointer_Node* p;
		p = (L_Keypointer_Node*)malloc(sizeof(L_Keypointer_Node)); L_Keypointer_Node_count++;
		p->array_row = r;

		if (k->KeyYpointer == NULL)
		{
			//printf("） %d %d\n", r, temp->ivalue);
			p->next = NULL;
			k->KeyYpointer = p;
			k->KeyYpointer_last = p;
		}
		else
		{
			p->next = k->KeyYpointer_last->next;
			k->KeyYpointer_last->next = p;
			k->KeyYpointer_last = p;
		}


		/*	p = k->KeyYpointer;
		printf("%d ", r);
		while (p != NULL)
		{
		printf("%d %d  \n" ,array_list[p->array_row].x, array_list[p->array_row].y);
		p = p->next;
		}*/
		r++;
	}

	/* 	printf("表A：\n");
	for (i = 1; i <= ARC_NUM; i++)
	printf("<%d,%d,%d ,%d>\n", array_list[i].x, array_list[i].y, array_list[i].Forpointer->ivalue, array_list[i].Repointer->ivalue);
	printf("array_list: %d\n",ar_num);
	for (i = 0; i < ar_num; i++)
	printf("<%d %d>\n", arindex[i].v, arindex[i].v_Forpointer->ivalue);  */

}

void SetIndex_transitive(char* rule, char* realtions, char attribute_list[], BPTree_Index* btree_index)
{//成功地把图从文件中读入表A (没问题)
	int r = 1, i, j;
	btree_node* root = btree_index->root;
	index_head* infor = btree_index->infor;

	/***********初始化************/
	for (i = 0; i <= ARC_NUM; i++)
	{
		array_list[i].x = -1;
		array_list[i].y = -1;
		array_list[i].Forpointer = NULL;
		array_list[i].Repointer = NULL;
	}
	for (i = 0; i < VERTEX_NUM; i++)
	{
		arindex[i].v = -1;
		arindex[i].v_Forpointer = NULL;
	}
	array_list[0].x = (int)rule;  //预留空间保存规则路径

	key_type* k = NULL;
	fp = fopen(realtions, "r");
	char str[15];    //存从文件中读入的一行字符串
	if (fp == NULL)
	{
		printf("打开文件失败\n");
	}
	i = 0;
	//FILE* file = NULL;
	//file = fopen(pathStr, "r");

	char buff[BUFFSIZE];
	char* token = NULL;
	int v, w;

	key_type** p_keys, * temp;
	p_keys = (key_type**)((long int)root->keys + (long int)infor->base);
	temp = (key_type*)((long int)p_keys[0] + (long int)infor->base);

	while (fgets(buff, 81, fp))
	{
		sscanf(buff, "%*[<]%d,%d%*[>]", &v, &w);
		array_list[r].x = v;
		array_list[r].y = w;

		k = Search_block(array_list[r].x, root, infor);


		if (!isexit_array_index[array_list[r].x])
		{
			isexit_array_index[array_list[r].x] = 1;
			arindex[ar_num].v = array_list[r].x;
			arindex[ar_num].v_Forpointer = k;
			ar_num++;
		}

		array_list[r].Forpointer = Search_block(array_list[r].y, root, infor);
		array_list[r].Repointer = Search_block(array_list[r].x, root, infor);

		//array_list[r].Repointer = k; 

		L_Keypointer_Node* q;
		q = (L_Keypointer_Node*)malloc(sizeof(L_Keypointer_Node)); L_Keypointer_Node_count++;
		q->array_row = r;

		if (k->Keypointer == NULL)
		{
			//printf("） %d %d\n", r, temp->ivalue);
			q->next = NULL;
			k->Keypointer = q;
			k->Keypointer_last = q;
		}
		else
		{
			q->next = k->Keypointer_last->next;
			k->Keypointer_last->next = q;
			k->Keypointer_last = q;
		}

		k = Search_block(array_list[r].y, root, infor);
		L_Keypointer_Node* p;
		p = (L_Keypointer_Node*)malloc(sizeof(L_Keypointer_Node)); L_Keypointer_Node_count++;
		p->array_row = r;

		if (k->KeyYpointer == NULL)
		{
			//printf("） %d %d\n", r, temp->ivalue);
			p->next = NULL;
			k->KeyYpointer = p;
			k->KeyYpointer_last = p;
		}
		else
		{
			p->next = k->KeyYpointer_last->next;
			k->KeyYpointer_last->next = p;
			k->KeyYpointer_last = p;
		}


		/*	p = k->KeyYpointer;
		printf("%d ", r);
		while (p != NULL)
		{
		printf("%d %d  \n" ,array_list[p->array_row].x, array_list[p->array_row].y);
		p = p->next;
		}*/
		r++;
	}

	/* 	printf("表A：\n");
	for (i = 1; i <= ARC_NUM; i++)
	printf("<%d,%d,%d ,%d>\n", array_list[i].x, array_list[i].y, array_list[i].Forpointer->ivalue, array_list[i].Repointer->ivalue);
	printf("array_list: %d\n",ar_num);
	for (i = 0; i < ar_num; i++)
	printf("<%d %d>\n", arindex[i].v, arindex[i].v_Forpointer->ivalue);  */

}

btree_node* test_insert(btree_node* root, index_head* infor, char* path, char attribute_bptree[], int attribute_bptree_list) {
	 
	key_type* key;
	int i, value;
	key = (key_type*)malloc(MAX_KEY_LEN);
	init_head_leaf(infor);
	int x, y;
	char file_name[50];
	char buff[BUFFSIZE];
	fp = fopen(path, "r");
	if (fp == NULL)
	{
		printf("打开文件失败\n");
	}
	while (fgets(buff, 81, fp))
	{
		sscanf(buff, "%*[<]%d,%d%*[>]", &x, &y);
		if (attribute_bptree[0] == 'x')
		{
			if (Search_block(x, root, infor) == NULL) {
				srand(time(NULL));
				value = rand(); //printf("value=%d\n ", value);
				key->ivalue = x;  //10000000 + i;//鍒濆鍖杒ey锛屾暣鍨	/************	 *姝ゅ涓鸿皟鐢ㄦ彃鍏ユ帴鍙ｏ紝	 */  
				root = btree_insert(root, key, value, infor);
				infor->root = (btree_node*)((long int)root - (long int)infor->base);
			}
		} 
		if (attribute_bptree[1] == 'y')
		{
			if (Search_block(y, root, infor) == NULL) {
				srand(time(NULL));
				value = rand(); //printf("value=%d\n ", value);
				key->ivalue = y;  //10000000 + i;//鍒濆鍖杒ey锛屾暣鍨	/************	 *姝ゅ涓鸿皟鐢ㄦ彃鍏ユ帴鍙ｏ紝	 */  
				root = btree_insert(root, key, value, infor);
				infor->root = (btree_node*)((long int)root - (long int)infor->base);
			}
		}
	}
	free(key); 
	return root; 
}

btree_node* test_insert(btree_node* root, index_head* infor, char* path, char attribute_bptree[]) {

	key_type* key;
	int i, value;
	key = (key_type*)malloc(MAX_KEY_LEN);
	init_head_leaf(infor);
	int x, y;
	char file_name[50];
	char buff[BUFFSIZE];
	fp = fopen(path, "r");
	if (fp == NULL)
	{
		printf("打开文件失败\n");
	}
	while (fgets(buff, 81, fp))
	{
		sscanf(buff, "%*[<]%d,%d%*[>]", &x, &y);
		if (attribute_bptree[0] == 'x')
		{
			if (Search_block(x, root, infor) == NULL) {
				srand(time(NULL));
				value = rand(); //printf("value=%d\n ", value);
				key->ivalue = x;  //10000000 + i;//鍒濆鍖杒ey锛屾暣鍨	/************	 *姝ゅ涓鸿皟鐢ㄦ彃鍏ユ帴鍙ｏ紝	 */  
				root = btree_insert(root, key, value, infor);
				infor->root = (btree_node*)((long int)root - (long int)infor->base);
			}
		}
		if (attribute_bptree[1] == 'y')
		{
			if (Search_block(y, root, infor) == NULL) {
				srand(time(NULL));
				value = rand(); //printf("value=%d\n ", value);
				key->ivalue = y;  //10000000 + i;//鍒濆鍖杒ey锛屾暣鍨	/************	 *姝ゅ涓鸿皟鐢ㄦ彃鍏ユ帴鍙ｏ紝	 */  
				root = btree_insert(root, key, value, infor);
				infor->root = (btree_node*)((long int)root - (long int)infor->base);
			}
		}
	}
	free(key);
	return root;
}




int count = 0;
int Tom_forwarddfs_feidigui(int i, key_type* kv, BPTree_Index* btree_index)
{
	btree_node* root = btree_index->root;
	index_head* infor = btree_index->infor;
	int count = 0;
	int w;
	LinkList* head, * end, * node, * e;
	L_Keypointer_List p;
	//L_Keypointer_List p = kv->Keypointer;
	head = (LinkList*)malloc(sizeof(LinkList));
	end = head;
	head->next = NULL;  //创建链表，开辟了头指针结点，头尾指针现在在一起
	//初始化链表，把V点插入，end指向V
	int v = kv->ivalue; //V是Tom
	node = (LinkList*)malloc(sizeof(LinkList));
	node->id = v;
	node->kv = kv; //kv是对应的B+树结点
	node->next = NULL;
	head->next = node;
	end = node;
	//相继插入V点后继
	//print_linklist(head);

	while (head->next)
	{
		e = head->next;//e是即将要删除的第一个结点
		head->next = e->next; //删除了第一个结点  
		if (head->next == NULL)
		{//如果链表已经空了
			end = head;
		}
		p = e->kv->Keypointer;//p是Keypoint第一项，指向e->id的第一个索引表项
		free(e);

		while (p)
		{//通过q把e的后继全都放到链表

			w = array_list[p->array_row].y;  //w是e->id的一个后继
			if (add_visited[w] != i)
			{
				node = (LinkList*)malloc(sizeof(LinkList));
				node->id = w;
				node->kv = array_list[p->array_row].Forpointer;
				node->next = end->next;  //尾插
				end->next = node;
				end = node;
				add_visited[node->id] = i; //标位已访问  
				count++;
				result_x.push_back(i);
				result_y.push_back(w);  //结果放入动态数组
			}

			p = p->next;
		}
	}
	return count;
}
int Tom_forwarddfs_feidigui_increment(int i, key_type* kv, BPTree_Index* btree_index)
{
	memset(add_visited, 0, sizeof(add_visited));
	int count = 0;
	int w;
	LinkList* head, * end, * node, * e;
	L_Keypointer_List p;
	//L_Keypointer_List p = kv->Keypointer;
	head = (LinkList*)malloc(sizeof(LinkList));
	end = head;
	head->next = NULL;  //创建链表，开辟了头指针结点，头尾指针现在在一起
	//初始化链表，把V点插入，end指向V
	int v = kv->ivalue; //V是Tom
	node = (LinkList*)malloc(sizeof(LinkList));
	node->id = v;
	node->kv = kv; //kv是对应的B+树结点
	node->next = NULL;
	head->next = node;
	end = node;
	//相继插入V点后继
	//print_linklist(head);

	while (head->next)
	{
		e = head->next;//e是即将要删除的第一个结点
		head->next = e->next; //删除了第一个结点  
		if (head->next == NULL)
		{//如果链表已经空了
			end = head;
		}
		//printf("弹出：%d\n", e->id);
		p = e->kv->Keypointer;//p是Keypoint第一项，指向e->id的第一个索引表项
		free(e);

		while (p)
		{//通过q把e的后继全都放到链表

			w = array_list[p->array_row].y;  //w是e->id的一个后继
			if (add_visited[w] != i)
			{
				node = (LinkList*)malloc(sizeof(LinkList));
				node->id = w;
				node->kv = array_list[p->array_row].Forpointer;
				node->next = end->next;  //尾插
				end->next = node;
				end = node;
				add_visited[node->id] = i; //标位已访问  
				count++;
				//result_x.push_back(i);
				//result_y.push_back(w);  //结果放入动态数组
				pushset1s(w);
			}

			p = p->next;
		}
	}
	return count;
}

BPTree_Index* Create_BPTree(char* path, char attribute_bptree[], int attribute_bptree_length)
{
	BPTree_Index* btree_index;
	btree_index = (BPTree_Index*)malloc(sizeof(BPTree_Index));
	btree_index->root = NULL;
	char file_name[50];
	char buff[BUFFSIZE];
	int v, w, max_num = 0;  //假设关系中全都是数字且没有负数

	fp = fopen(path, "r");
	if (fp == NULL)
	{
		printf("打开文件失败\n");
	}
	while (fgets(buff, 81, fp))
	{
		sscanf(buff, "%*[<]%d,%d%*[>]", &v, &w);
		if (v > max_num) max_num = v;
		if (w > max_num) max_num = w;
	}
	max_num += 2; //多建两个节点，防止溢出
   //建立B+树索引
	if ((btree_index->infor = create_index("school", "student", "no", 0, 2000000, file_name)))
	{
		btree_index->root = test_insert(btree_index->root, btree_index->infor, path, attribute_bptree, attribute_bptree_length); //返回B+数索引的根节点
		alloc_count = 0;
	}
	return btree_index;
}

BPTree_Index* Create_BPTree(char* path, char attribute_bptree[])
{
	BPTree_Index* btree_index;
	btree_index = (BPTree_Index*)malloc(sizeof(BPTree_Index));
	btree_index->root = NULL;
	char file_name[50];
	char buff[BUFFSIZE];
	int v, w, max_num = 0;  //假设关系中全都是数字且没有负数

	fp = fopen(path, "r");
	if (fp == NULL)
	{
		printf("打开文件失败\n");
	}
	while (fgets(buff, 81, fp))
	{
		sscanf(buff, "%*[<]%d,%d%*[>]", &v, &w);
		if (v > max_num) max_num = v;
		if (w > max_num) max_num = w;
	}
	max_num += 2; //多建两个节点，防止溢出
   //建立B+树索引
	if ((btree_index->infor = create_index("school", "student", "no", 0, 2000000, file_name)))
	{
		btree_index->root = test_insert(btree_index->root, btree_index->infor, path, attribute_bptree); //返回B+数索引的根节点
		alloc_count = 0;
	}
	return btree_index;
}

void Put_result_into_File(FILE* fp)
{
	//写文件
	std::vector<int>::iterator temp1 = result_x.begin();
	std::vector<int>::iterator temp2 = result_y.begin();
	while (temp1 != result_x.end() || temp2 != result_y.end())
	{
		/*Res result;
		result.x = *temp1;
		result.y = *temp2; //printf("%d %d\n", result.x, result.y);
		fwrite(&result, sizeof(result), 1, fp);*/
		char buf[10];
		_itoa_s(*temp1, buf, 10);
		char buf2[10];
		_itoa_s(*temp2, buf2, 10);

		char str[80];
		strcpy(str, "<");
		strcat(str, buf);
		strcat(str, ",");
		strcat(str, buf2);
		strcat(str, ">");
		//	printf("%s\n", str);
		fputs(str, fp);
		fputs("\n", fp);

		temp1++; temp2++;
	}

	rewind(fp);
}
void Person_Ancestors(int person_list[], int person_list_length, BPTree_Index* btree_index)
{
	int  count;
	for (int v = 0; v < VERTEX_NUM; v++)
		add_visited[v] = -1;  //访问标记初始化 

	for (int i = 0; i < person_list_length; i++)
	{
		count = 0;
		key_type* k = Search_block(person_list[i], btree_index->root, btree_index->infor);
		count = Tom_forwarddfs_feidigui(person_list[i], k, btree_index);  //Tom_forwarddfs就是从Tom开始向前深度搜索，旨在求出Tom的所有祖先。
		oudui += count;
	}
	//FILE* fp = fopen("E:\\test02.txt", "w+");
	//Put_result_into_File(fp);  //结果写入文件 
}
bool Reachable(int i, int j, key_type* kv, BPTree_Index* btree_index)
{
	btree_node* root = btree_index->root;
	index_head* infor = btree_index->infor;
	memset(add_visited, 0, sizeof(add_visited));
	int count = 0;
	int w;
	LinkList* head, * end, * node, * e;
	L_Keypointer_List p;
	head = (LinkList*)malloc(sizeof(LinkList));
	end = head;
	head->next = NULL;  //创建链表，开辟了头指针结点，头尾指针现在在一起
	//初始化链表，把V点插入，end指向V
	int v = kv->ivalue; //V是Tom
	node = (LinkList*)malloc(sizeof(LinkList));
	node->id = v;
	node->kv = kv; //kv是对应的B+树结点
	node->next = NULL;
	head->next = node;
	end = node;
	//相继插入V点后继
	//print_linklist(head);

	while (head->next)
	{
		e = head->next;//e是即将要删除的第一个结点
		head->next = e->next; //删除了第一个结点  
		if (head->next == NULL)
		{//如果链表已经空了
			end = head;
		}
		//printf("弹出：%d\n", e->id);
		p = e->kv->Keypointer;//p是Keypoint第一项，指向e->id的第一个索引表项
		free(e);

		while (p)
		{//通过q把e的后继全都放到链表

			w = array_list[p->array_row].y;  //w是e->id的一个后继

			if (add_visited[w] != i)
			{
				node = (LinkList*)malloc(sizeof(LinkList));
				node->id = w;
				node->kv = array_list[p->array_row].Forpointer;
				node->next = end->next;  //尾插
				end->next = node;
				end = node;
				add_visited[node->id] = i; //标位已访问  
			//	count++;
				//result_x.push_back(i);
				//result_y.push_back(w);  //结果放入动态数组
				//pushset1s(w);
				if (j == w) return true;
			}
			p = p->next;
		}
	}
	return false;
}
void Cartesian_product(int add_v, int add_w, BPTree_Index* btree_index)
{//set1,set2做笛卡尔积
	btree_node* root = btree_index->root;
	index_head* infor = btree_index->infor;
	int add_v_reachable[VERTEX_NUM + 1] = { 0 };
	int m, n;
	int Size_set1 = set1s.top;
	key_type* k = Search_block(add_v, root, infor);
	while (Size_set1 >= 0)
	{
		m = set1s.set1s[Size_set1];

		if (Reachable(add_v, m, k, btree_index))
		{
			add_v_reachable[m] = 1;//表示add_v可达这个点，之后set2中的点都不判断这个点了，因为set2集合都可达这个点。

		}
		else
		{
			//产生增量<add_v, m>
			result_x.push_back(add_v);
			result_y.push_back(m);
			//printf("<%d,%d> ", add_v, m);
		}
		Size_set1--;
	}
	int Size_set2 = set2s.top;
	while (Size_set2 >= 0)
	{
		m = set2s.set2s[Size_set2];
		k = Search_block(m, root, infor);
		if (!Reachable(m, add_w, k, btree_index))  //如果m能到add_w，则可到set1全体，m不产生增量跳过

		{
			//产生增量<m, add_w>
			result_x.push_back(m);
			result_y.push_back(add_w);
			//printf("<%d,%d> ", m, add_w);

			Size_set1 = set1s.top;
			while (Size_set1 >= 0)
			{
				n = set1s.set1s[Size_set1];
				if (!add_v_reachable[m])

				{
					if (!Reachable(m, n, k, btree_index))
					{
						//产生增量<m, n>
						result_x.push_back(m);
						result_y.push_back(n);
						//printf("<%d,%d> ", m, n);
					}
				}
				Size_set1--;
			}
		}
		Size_set2--;
	}
}
int Tom_reversedfs_feidigui(int i, key_type* kv, BPTree_Index* btree_index)
{
	memset(add_visited, 0, sizeof(add_visited));
	int count = 0;
	int w;
	LinkList* head, * end, * node, * e;
	L_Keypointer_List p;
	//L_Keypointer_List p = kv->Keypointer;
	head = (LinkList*)malloc(sizeof(LinkList));
	end = head;
	head->next = NULL;  //创建链表，开辟了头指针结点，头尾指针现在在一起
	//初始化链表，把V点插入，end指向V
	int v = kv->ivalue; //V是Tom
	node = (LinkList*)malloc(sizeof(LinkList));
	node->id = v;
	node->kv = kv; //kv是对应的B+树结点
	node->next = NULL;
	head->next = node;
	end = node;
	//相继插入V点后继
	//print_linklist(head);

	while (head->next)
	{
		e = head->next;//e是即将要删除的第一个结点
		head->next = e->next; //删除了第一个结点  
		if (head->next == NULL)
		{//如果链表已经空了
			end = head;
		}
		//printf("弹出：%d\n", e->id);
		p = e->kv->KeyYpointer;//p是Keypoint第一项，指向e->id的第一个索引表项
		free(e);

		while (p)
		{//通过q把e的前继全都放到链表

			w = array_list[p->array_row].x;  //w是e->id的一个前继
			if (add_visited[w] != i)
			{
				node = (LinkList*)malloc(sizeof(LinkList));
				node->id = w;
				node->kv = array_list[p->array_row].Repointer;
				node->next = end->next;  //尾插
				end->next = node;
				end = node;
				add_visited[node->id] = i; //标位已访问  
				count++;
				result_x.push_back(i);
				result_y.push_back(w);  //结果放入动态数组
				//pushset2s(w);
			}

			p = p->next;
		}
	}
	return count;
}
void add_tran(int add_v, int add_w, BPTree_Index* btree_index)
{
	btree_node* root = btree_index->root;
	index_head* infor = btree_index->infor;

	//先判断add_v和add_w是否可达
	key_type* k = Search_block(add_v, root, infor);
	if (Reachable(add_v, add_w, k, btree_index))
		printf("v,w本身可达!\n");
	else
	{
		//产生增量<add_v, add_w>
		result_x.push_back(add_v);
		result_y.push_back(add_w);
		//printf("<%d,%d> ", add_v, add_w);

		k = Search_block(add_w, root, infor);
		Tom_forwarddfs_feidigui_increment(add_w, k, btree_index);

		k = Search_block(add_v, root, infor);
		Tom_reversedfs_feidigui(add_v, k, btree_index);

		Cartesian_product(add_v, add_w, btree_index);//set1,set2做笛卡尔积
	}
}
void Increment(BPTree_Index* btree_index, char* path)
{
	LARGE_INTEGER litmp;
	LONGLONG   QPart1, QPart2;
	double   dfMinus, dfFreq, dfTim, t1;


	FILE* fp = fopen(path, "r");
	if (fp == NULL)
	{
		printf("打开增量文件失败\n");
	}
	int i = 0;
	int add_v, add_w;
	char buff[BUFFSIZE];
	while (fgets(buff, 81, fp))
	{

		memset(add_visited, 0, sizeof(add_visited));
		set1s.top = -1;
		set2s.top = -1;
		Size_set1 = 0; Size_set2 = 0;
		sscanf(buff, "%*[<]%d,%d%*[>]", &add_v, &add_w);


		QueryPerformanceFrequency(&litmp);
		//   获得计数器的时钟频率  
		dfFreq = (double)litmp.QuadPart;
		QueryPerformanceCounter(&litmp);
		QPart1 = litmp.QuadPart;

		add_tran(add_v, add_w, btree_index);

		QueryPerformanceCounter(&litmp);
		QPart2 = litmp.QuadPart;
		dfMinus = (double)(QPart2 - QPart1);
		dfTim = dfMinus / dfFreq;
		printf("求增量cost %f s.\n", dfTim);
	}
	//FILE* fp2 = fopen("E:\\test02.txt", "w+");
	//Put_result_into_File(fp2);  //结果写入文件

}
void Transitive_closure(BPTree_Index* btree_index)
{
	//int person_list[10] = { 1,2,3,4,5,6,7,8,9,10 };   //数组保存需要求其祖先的人
	int person_list[200];
	for (int j = 0; j < 200; j++)
		person_list[j] = j;
	int person_list_length = sizeof(person_list) / sizeof(person_list[0]);
	Person_Ancestors(person_list, person_list_length, btree_index);    //求person_list数组里的人的祖先
}
void Create_IndexTable(char* rule, int index_type, char* realtions, char attribute_list[], int attribute_list_length, BPTree_Index* btree_index)
{
	switch (index_type) {
	case 1:    //传递
	{
		SetIndex_transitive(rule, realtions, attribute_list, attribute_list_length, btree_index);
	}
	break;
	case 2:    //join
		//SetIndex_join(rule, realtions, attribute_list, attribute_list_length, btree_index);
		break;
	case 3:    //含非规则
		//SetIndex_not(rule, realtions, attribute_list, attribute_list_length, btree_index);
		break;
	}
}

//吃个鱼优化后的
//求后代
void Person_Descendants(int person_list[], int person_list_length, BPTree_Index* btree_index)
{
	int  count;
	for (int v = 0; v < VERTEX_NUM; v++)
		add_visited[v] = -1;  //访问标记初始化 

	for (int i = 0; i < person_list_length; i++)
	{
		count = 0;
		key_type* k = Search_block(person_list[i], btree_index->root, btree_index->infor);
		count = Tom_reversedfs_feidigui(person_list[i], k, btree_index);  //Tom_forwarddfs就是从Tom开始向前深度搜索，旨在求出Tom的所有祖先。
		oudui += count;
	}
	//FILE* fp = fopen("E:\\test02.txt", "w+");
	//Put_result_into_File(fp);  //结果写入文件 
}

//求共同祖先的前置操作
void Tom_forwarddfs_person_ancentor(int i, key_type* kv, BPTree_Index* btree_index)
{
	btree_node* root = btree_index->root;
	index_head* infor = btree_index->infor;
	int count = 0;
	int w;
	LinkList* head, * end, * node, * e;
	L_Keypointer_List p;
	//L_Keypointer_List p = kv->Keypointer;
	head = (LinkList*)malloc(sizeof(LinkList));
	end = head;
	head->next = NULL;  //创建链表，开辟了头指针结点，头尾指针现在在一起
	//初始化链表，把V点插入，end指向V
	int v = kv->ivalue; //V是Tom
	node = (LinkList*)malloc(sizeof(LinkList));
	node->id = v;
	node->kv = kv; //kv是对应的B+树结点
	node->next = NULL;
	head->next = node;
	end = node;
	//相继插入V点后继
	//print_linklist(head);

	while (head->next)
	{
		e = head->next;//e是即将要删除的第一个结点
		head->next = e->next; //删除了第一个结点  
		if (head->next == NULL)
		{//如果链表已经空了
			end = head;
		}
		p = e->kv->Keypointer;//p是Keypoint第一项，指向e->id的第一个索引表项
		free(e);

		while (p)
		{//通过q把e的后继全都放到链表

			w = array_list[p->array_row].y;  //w是e->id的一个后继
			if (add_visited[w] != i)
			{
				node = (LinkList*)malloc(sizeof(LinkList));
				node->id = w;
				node->kv = array_list[p->array_row].Forpointer;
				node->next = end->next;  //尾插
				end->next = node;
				end = node;
				add_visited[node->id]=1; //标位已访问  
				count++;
			}
			p = p->next;
		}
	}
}

void Jim_forwarddfs_person_ancentor(int i, key_type* kv, BPTree_Index* btree_index)
{
	btree_node* root = btree_index->root;
	index_head* infor = btree_index->infor;
	int count = 0;
	int w;
	LinkList* head, * end, * node, * e;
	L_Keypointer_List p;
	//L_Keypointer_List p = kv->Keypointer;
	head = (LinkList*)malloc(sizeof(LinkList));
	end = head;
	head->next = NULL;  //创建链表，开辟了头指针结点，头尾指针现在在一起
	//初始化链表，把V点插入，end指向V
	int v = kv->ivalue; //V是Tom
	node = (LinkList*)malloc(sizeof(LinkList));
	node->id = v;
	node->kv = kv; //kv是对应的B+树结点
	node->next = NULL;
	head->next = node;
	end = node;
	//相继插入V点后继
	//print_linklist(head);

	while (head->next)
	{
		e = head->next;//e是即将要删除的第一个结点
		head->next = e->next; //删除了第一个结点  
		if (head->next == NULL)
		{//如果链表已经空了
			end = head;
		}
		p = e->kv->Keypointer;//p是Keypoint第一项，指向e->id的第一个索引表项
		free(e);

		while (p)
		{//通过q把e的后继全都放到链表

			w = array_list[p->array_row].y;  //w是e->id的一个后继
			if (add_visited[w] != i)
			{
				node = (LinkList*)malloc(sizeof(LinkList));
				node->id = w;
				node->kv = array_list[p->array_row].Forpointer;
				node->next = end->next;  //尾插
				end->next = node;
				end = node;
				add_visited[node->id]++; //标位已访问  
				count++;
			}
			p = p->next;
		}
	}
}
//共同后代
void Tom_forwarddfs_person_descendant(int i, key_type* kv, BPTree_Index* btree_index)
{
	btree_node* root = btree_index->root;
	index_head* infor = btree_index->infor;
	int count = 0;
	int w;
	LinkList* head, * end, * node, * e;
	L_Keypointer_List p;
	//L_Keypointer_List p = kv->Keypointer;
	head = (LinkList*)malloc(sizeof(LinkList));
	end = head;
	head->next = NULL;  //创建链表，开辟了头指针结点，头尾指针现在在一起
	//初始化链表，把V点插入，end指向V
	int v = kv->ivalue; //V是Tom
	node = (LinkList*)malloc(sizeof(LinkList));
	node->id = v;
	node->kv = kv; //kv是对应的B+树结点
	node->next = NULL;
	head->next = node;
	end = node;
	//相继插入V点后继
	//print_linklist(head);

	while (head->next)
	{
		e = head->next;//e是即将要删除的第一个结点
		head->next = e->next; //删除了第一个结点  
		if (head->next == NULL)
		{//如果链表已经空了
			end = head;
		}
		p = e->kv->Keypointer;//p是Keypoint第一项，指向e->id的第一个索引表项
		free(e);

		while (p)
		{//通过q把e的后继全都放到链表

			w = array_list[p->array_row].y;  //w是e->id的一个后继
			if (add_visited[w] != i)
			{
				node = (LinkList*)malloc(sizeof(LinkList));
				node->id = w;
				node->kv = array_list[p->array_row].Repointer;
				node->next = end->next;  //尾插
				end->next = node;
				end = node;
				add_visited[node->id] = 1; //标位已访问  
				count++;
			}
			p = p->next;
		}
	}
}

void Jim_forwarddfs_person_descendant(int i, key_type* kv, BPTree_Index* btree_index)
{
	btree_node* root = btree_index->root;
	index_head* infor = btree_index->infor;
	int count = 0;
	int w;
	LinkList* head, * end, * node, * e;
	L_Keypointer_List p;
	//L_Keypointer_List p = kv->Keypointer;
	head = (LinkList*)malloc(sizeof(LinkList));
	end = head;
	head->next = NULL;  //创建链表，开辟了头指针结点，头尾指针现在在一起
	//初始化链表，把V点插入，end指向V
	int v = kv->ivalue; //V是Tom
	node = (LinkList*)malloc(sizeof(LinkList));
	node->id = v;
	node->kv = kv; //kv是对应的B+树结点
	node->next = NULL;
	head->next = node;
	end = node;
	//相继插入V点后继
	//print_linklist(head);

	while (head->next)
	{
		e = head->next;//e是即将要删除的第一个结点
		head->next = e->next; //删除了第一个结点  
		if (head->next == NULL)
		{//如果链表已经空了
			end = head;
		}
		p = e->kv->Keypointer;//p是Keypoint第一项，指向e->id的第一个索引表项
		free(e);

		while (p)
		{//通过q把e的后继全都放到链表

			w = array_list[p->array_row].y;  //w是e->id的一个后继
			if (add_visited[w] != i)
			{
				node = (LinkList*)malloc(sizeof(LinkList));
				node->id = w;
				node->kv = array_list[p->array_row].Repointer;
				node->next = end->next;  //尾插
				end->next = node;
				end = node;
				add_visited[node->id]++; //标位已访问  
				count++;
			}
			p = p->next;
		}
	}
}

//共同祖先和共同后代写文件
void Put_result_into_File(FILE* fp,int *person_list,int size) {
	//写文件
	std::vector<int>::iterator temp1 = result_x.begin();
	std::vector<int>::iterator temp2 = result_y.begin();
	for (int i = 0; i < size; i++) {
		while (heightset3s() > -1) {
			char buf[10];
			_itoa_s(person_list[i], buf, 10);
			char buf2[10];
			int e = popset3s();
			_itoa_s(e, buf2, 10);
			char str[80];
			strcpy(str, "<");
			strcat(str, buf);
			strcat(str, ",");
			strcat(str, buf2);
			strcat(str, ">");
			//	printf("%s\n", str);
			fputs(str, fp);
			fputs("\n", fp);
			pushset2s(e);
		}
		while (heightset2s() > -1) {
			pushset3s(popset3s());
		}
	}
	rewind(fp);
}

//求一个列表的后代
void Person_Descendants(int person_list[], int person_list_length, BPTree_Index* btree_index)
{
	int  count;
	for (int v = 0; v < VERTEX_NUM; v++)
		add_visited[v] = -1;  //访问标记初始化 

	for (int i = 0; i < person_list_length; i++)
	{
		count = 0;
		key_type* k = Search_block(person_list[i], btree_index->root, btree_index->infor);
		count = Tom_reversedfs_feidigui(person_list[i], k, btree_index);  //Tom_forwarddfs就是从Tom开始向前深度搜索，旨在求出Tom的所有祖先。
		oudui += count;
	}
	//FILE* fp = fopen("E:\\test02.txt", "w+");
	//Put_result_into_File(fp);  //结果写入文件 
}

BPTree_Index* CreateIndex(char* rule, int index_type, char* relations, char* attribute_list, int attribute_type) {
	BPTree_Index* btree_index;
	btree_index = Create_BPTree(relations, attribute_list);
	//1是int类型，否则是char型
	if (attribute_type == 1) {
		switch (index_type) {
		case 1:    //传递
		{
			SetIndex_transitive(rule, relations, attribute_list, btree_index);
		}
		break;
		case 2:    //join
			//SetIndex_join(rule, realtions, attribute_list, attribute_list_length, btree_index);
			break;
		case 3:    //含非规则
			//SetIndex_not(rule, realtions, attribute_list, attribute_list_length, btree_index);
			break;
		default:
			break;
		}
	}
	else {

	}

	return btree_index;
	
}

OuDui* One_Person_AncestorsM(int person, BPTree_Index* btree_index) {

	int person_list[10];
	person_list[0] = person;
	Person_Ancestors(person_list, 1, btree_index);
	std::vector<int>::iterator temp1 = result_x.begin();
	std::vector<int>::iterator temp2 = result_y.begin();
	//写一个链表
	OuDui* head, * s, * r;
	head = (OuDui*)malloc(sizeof(OuDui));
	r = head;
	while (temp2 != result_y.end() || temp1 != result_x.end()) {
		s = (OuDui*)malloc(sizeof(OuDui));
		s->x = person;
		s->y = *temp2;
		r->next = s;
		r = s;
		temp2++;
	}
	r->next = NULL;
	return head;
}

void One_Person_AncestorsD(int person, BPTree_Index* btree_index) {
	int person_list[10];
	person_list[0] = person;
	Person_Ancestors(person_list, 1, btree_index);
	FILE* fp2 = fopen("E:\\One_Person_AncestorsD.txt", "w+");
	Put_result_into_File(fp2);
	fclose(fp2);
}

//求共同祖先
OuDui* Person_AncestorsM(int* person_list, BPTree_Index* btree_index) {
	
	int size = sizeof(person_list) / sizeof(person_list[0]);
	key_type* k = Search_block(person_list[0], btree_index->root, btree_index->infor);
	add_visited[person_list[0]] = 1;
	Tom_forwarddfs_person_ancentor(person_list[0], k, btree_index);
	for (int i = 1; i < size; i++) {
		key_type* k = Search_block(person_list[i], btree_index->root, btree_index->infor);
		add_visited[person_list[i]]++;
		Jim_forwarddfs_person_ancentor(person_list[i], k, btree_index);
	}
	for (int i = 0; i < VERTEX_NUM; i++) {
		if (add_visited[i] == size)
			pushset3s(i);
	}
	OuDui* head, * s, * r;
	head = (OuDui*)malloc(sizeof(OuDui));
	r = head;
	for (int i = 0; i < size; i++){
		while (heightset3s()>-1) {
			s = (OuDui*)malloc(sizeof(OuDui));
			s->x = person_list[i];
			s->y = popset3s();
			pushset2s(s->y);
			r->next = s;
			r = s;
		}
		while (heightset2s() > -1) {
			pushset3s(popset2s());
		}
	}
	r->next = NULL;
	return head;
}

void Person_AncestorsD(int* person_list, BPTree_Index* btree_index) {

	int size = sizeof(person_list) / sizeof(person_list[0]);
	key_type* k = Search_block(person_list[0], btree_index->root, btree_index->infor);
	add_visited[person_list[0]] = 1;
	Tom_forwarddfs_person_ancentor(person_list[0], k, btree_index);
	for (int i = 1; i < size; i++) {
		key_type* k = Search_block(person_list[i], btree_index->root, btree_index->infor);
		add_visited[person_list[i]]++;
		Jim_forwarddfs_person_ancentor(person_list[i], k, btree_index);
	}
	for (int i = 0; i < VERTEX_NUM; i++) {
		if (add_visited[i] == size)
			pushset3s(i);
	}
	OuDui* head, * s, * r;
	head = (OuDui*)malloc(sizeof(OuDui));
	r = head;
	for (int i = 0; i < size; i++) {
		while (heightset3s() > -1) {
			s = (OuDui*)malloc(sizeof(OuDui));
			s->x = person_list[i];
			s->y = popset3s();
			pushset2s(s->y);
			r->next = s;
			r = s;
		}
		while (heightset2s() > -1) {
			pushset3s(popset2s());
		}
	}
	r->next = NULL;
	FILE* fp2 = fopen("E:\\One_Person_AncestorsD.txt", "w+");
	Put_result_into_File(fp2, person_list, size);
}

//求一个人的后代
OuDui* One_Person_descendantM(int person, BPTree_Index* btree_index) {
	
	int person_list[10];
	person_list[0] = person;
	Person_Descendants(person_list, 1, btree_index);
	std::vector<int>::iterator temp1 = result_x.begin();
	std::vector<int>::iterator temp2 = result_y.begin();
	//写一个链表
	OuDui* head, * s, * r;
	head = (OuDui*)malloc(sizeof(OuDui));
	r = head;
	while (temp2 != result_y.end() || temp1 != result_x.end()) {
		s = (OuDui*)malloc(sizeof(OuDui));
		s->x = person;
		s->y = *temp2;
		r->next = s;
		r = s;
		temp2++;
	}
	r->next = NULL;
	return head;

}

void One_Person_descendantD(int person, BPTree_Index* btree_index) {

	int person_list[10];
	person_list[0] = person;
	Person_Descendants(person_list, 1, btree_index);
	FILE* fp2 = fopen("E:\\One_Person_descendantD.txt", "w+");
	Put_result_into_File(fp2);
	fclose(fp2);

}

//求共同后代
OuDui* Person_descendantM(int* person_list, BPTree_Index* btree_index) {
	int size = sizeof(person_list) / sizeof(person_list[0]);
	key_type* k = Search_block(person_list[0], btree_index->root, btree_index->infor);
	add_visited[person_list[0]] = 1;
	Tom_forwarddfs_person_descendant(person_list[0], k, btree_index);
	for (int i = 1; i < size; i++) {
		key_type* k = Search_block(person_list[i], btree_index->root, btree_index->infor);
		add_visited[person_list[i]]++;
		Jim_forwarddfs_person_descendant(person_list[i], k, btree_index);
	}
	for (int i = 0; i < VERTEX_NUM; i++) {
		if (add_visited[i] == size)
			pushset3s(i);
	}
	OuDui* head, * s, * r;
	head = (OuDui*)malloc(sizeof(OuDui));
	r = head;
	for (int i = 0; i < size; i++) {
		while (heightset3s() > -1) {
			s = (OuDui*)malloc(sizeof(OuDui));
			s->x = person_list[i];
			s->y = popset3s();
			pushset2s(s->y);
			r->next = s;
			r = s;
		}
		while (heightset2s() > -1) {
			pushset3s(popset2s());
		}
	}
	r->next = NULL;
	return head;
}

void Person_descendantD(int* person_list, BPTree_Index* btree_index) {

	int size = sizeof(person_list) / sizeof(person_list[0]);
	key_type* k = Search_block(person_list[0], btree_index->root, btree_index->infor);
	add_visited[person_list[0]] = 1;
	Tom_forwarddfs_person_descendant(person_list[0], k, btree_index);
	for (int i = 1; i < size; i++) {
		key_type* k = Search_block(person_list[i], btree_index->root, btree_index->infor);
		add_visited[person_list[i]]++;
		Jim_forwarddfs_person_descendant(person_list[i], k, btree_index);
	}
	for (int i = 0; i < VERTEX_NUM; i++) {
		if (add_visited[i] == size)
			pushset3s(i);
	}
	OuDui* head, * s, * r;
	head = (OuDui*)malloc(sizeof(OuDui));
	r = head;
	for (int i = 0; i < size; i++) {
		while (heightset3s() > -1) {
			s = (OuDui*)malloc(sizeof(OuDui));
			s->x = person_list[i];
			s->y = popset3s();
			pushset2s(s->y);
			r->next = s;
			r = s;
		}
		while (heightset2s() > -1) {
			pushset3s(popset2s());
		}
	}
	r->next = NULL;
	FILE* fp2 = fopen("E:\\Person_AncestorsD.txt", "w+");
	Put_result_into_File(fp2, person_list, size);
}

//求传递闭包
OuDui* Transitive_closureM(BPTree_Index* btree_index) {
	int person_list[VERTEX_NUM];
	for (int i = 1; i <= VERTEX_NUM; i++) {
		person_list[i-1] = i;
	}
	Person_Ancestors(person_list, VERTEX_NUM, btree_index);
	std::vector<int>::iterator temp1 = result_x.begin();
	std::vector<int>::iterator temp2 = result_y.begin();
	OuDui* head, * s, * r;
	head = (OuDui*)malloc(sizeof(OuDui));
	r = head;
	while (temp2 != result_y.end() || temp1 != result_x.end()) {
		s = (OuDui*)malloc(sizeof(OuDui));
		s->x = *temp1;
		s->y = *temp2;
		r->next = s;
		r = s;
		temp1++;
		temp2++;
	}
	r->next = NULL;
	return head;
}

void Transitive_closureD(BPTree_Index* btree_index) {
	int person_list[VERTEX_NUM];
	for (int i = 1; i <= VERTEX_NUM; i++) {
		person_list[i - 1] = i;
	}
	Person_Ancestors(person_list, VERTEX_NUM, btree_index);
	FILE* fp2 = fopen("E:\\Transitive_closureD.txt", "w+");
	Put_result_into_File(fp2);
	fclose(fp2);
}
//求增量
OuDui* IncrementM(BPTree_Index* btree_index, char* increment) {
	Increment(btree_index, increment);
	std::vector<int>::iterator temp1 = result_x.begin();
	std::vector<int>::iterator temp2 = result_y.begin();
	OuDui* head, * s, * r;
	head = (OuDui*)malloc(sizeof(OuDui));
	r = head;
	while (temp2 != result_y.end() || temp1 != result_x.end()) {
		s = (OuDui*)malloc(sizeof(OuDui));
		s->x = *temp1;
		s->y = *temp2;
		r->next = s;
		r = s;
		temp1++;
		temp2++;
	}
	r->next = NULL;
	return head;
}

void IncrementD(BPTree_Index* btree_index, char* increment) {
	Increment(btree_index, increment);
	FILE* fp2 = fopen("E:\\Increment.txt", "w+");
	Put_result_into_File(fp2);  //结果写入文件
}

