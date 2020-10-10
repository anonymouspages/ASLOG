#ifndef BTREES_H
#define BTREES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <stdbool.h>
//#include <sys/timeb.h>
//#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <vector> 
using namespace std;

#define MAX_KEY_LEN 15
#define BLOCK_TYPE 5
#define VERTEX_NUM 201
#define ARC_NUM 4000
int Size_set1 = 0, Size_set2 = 0;
//int add_v = 2;
//int add_w = 3; 
//int DFN[VERTEX_NUM] = { -1 };
//int rot[VERTEX_NUM] = { -1 };
// int cw[VERTEX_NUM] = { -1 };//���ֳ�ʼ����ʽֻ��ʼ���˵�һ��Ԫ�أ����ܳ�ʼ��ȫ��Ԫ��
vector <int>DFN(VERTEX_NUM, -1);
vector <int>rot(VERTEX_NUM, -1);
vector <int>cw(VERTEX_NUM, -1);
int visited[VERTEX_NUM] = { 0 };
//int hsvaed[VERTEX_NUM] = { -1 }; 
vector <int>hsvaed(VERTEX_NUM, -1);
int add_visited[VERTEX_NUM + 1] = { 0 };
int is_insert[VERTEX_NUM] = { 0 };
int dereplicate[VERTEX_NUM] = { 0 }; //ȥ�����飬��¼ĳ��ǿ�����Ƿ��Ѿ������ʣ���ʵֻ�õ�c->num���ռ�
//0��ʾ��δ������;1��ʾ�������Ҳ���������-1��ʾ�������������������Ĳ�������Ϊ0��ķ�һЩʱ��

static int btree_size = 4;  // number of pointers of each btree_node
FILE *fp, *fp2;
int index = 1, is_increment = 0;
int oudui = 0;




/*
* the key
*/

/*typedef struct {
int elem[VERTEX_NUM + 1];
int top;
}stack;*/
/*stypedef struct IndexTable{
int x;
int y;
struct key_type *Forpointer;   //��������ָ��B�����ָ��
//struct key_type *Repointer;   //���ڷ���ָ��B�����ָ��
};//�������������ݽṹ
typedef IndexTable AdjList[ARC_NUM];*/
struct Res{
	int x;
	int y;
};
typedef struct
{
	int x;
	int y;
	struct key_type *Forpointer;
	struct key_type *Repointer;
}IndexTable;
typedef IndexTable AdjList[ARC_NUM + 1];
AdjList array_list;


typedef struct
{
	int v;
	struct key_type *v_Forpointer;
}Index_array;
typedef Index_array ar_Index[VERTEX_NUM];
ar_Index arindex;
int isexit_array_index[VERTEX_NUM] = { 0 };

typedef struct L_Keypointer_Node
{  //����Keypointer��KeyYpointer �ֱ��Ҷ�ӿ鶨λ��ԭ��ĵ�ĳ��
	int array_row;
	struct L_Keypointer_Node *next;
}L_Keypointer_Node, *L_Keypointer_List;
/*typedef struct L_Tuplepointer_Node
{
struct IndexTable *Tuplepointer;
struct L_Tuplepointer_Node *next;
}; //ָ���Ե�ǰ�鿪ͷ�����������������*/

typedef struct key_type{
	int ivalue;
	L_Keypointer_List Keypointer;  //ָ��͵�ǰ��x��ͬ��ԭ��ĳЩ�� �������ͷ��㣨��Ϊ�գ�
	L_Keypointer_List KeyYpointer;  //ָ��͵�ǰ��x��ͬ��ԭ��ĳЩ�� �������ͷ��㣨��Ϊ�գ�
	L_Keypointer_Node *Keypointer_last;
	L_Keypointer_Node *KeyYpointer_last;
	//L_Keypointer_List L_KeyYpointer;  //ָ��͵�ǰ��y��ͬ��ԭ��ĳЩ�� �������ͷ��㣨��Ϊ�գ�
	//L_Tuplepointer_Node   *Tuplepointer_last; 
}key_type;
/*
* Btree btree_node
*/
typedef struct btree_node {//Btree btree_node
	void **pointers;
	key_type **keys;
	struct btree_node *parent;
	int num_keys;
	bool is_leaf;
	struct btree_node *next_leaf_node;
} btree_node;
/*
* store the result of the range search
*/
typedef struct bvalue{
	void *value;
	struct bvalue *next;
}bvalue_t, blist_t;
//typedef bvalue_t blist_t;
/*
* the rowid
*/
typedef struct btree_record {
	int value;
} btree_record;


typedef struct queue {//the queue
	int capacity;
	int front;
	int rear;
	int size;
	btree_node **items;
} queue;

typedef struct head_add_node{
	void *next;
	short int block_size;
}head_add_node;
typedef struct block_node{
	void *next;
}block_node;
typedef struct index_head {// the index head
	btree_node *root;     //the root of the index tree
	void *base;           //the base address of the mmap
	int keys_type;        //store the type of the key
	int node_count;       //the number of the keys
	long int offset;           //the offset of the index file
	head_add_node block[BLOCK_TYPE]; //point to the next free block
} index_head;
/*
* the data for test
*/

typedef struct data_btree_node{
	int no;
	int age;
	char name[12];
	char tel_number[12];
}data_btree_node;
typedef struct
{
	int ns[VERTEX_NUM + 1];
	int top;
}nstack;

typedef  struct
{
	int cs[VERTEX_NUM + 1];
	int top;
}cstack;
typedef  struct
{
	int set1s[VERTEX_NUM + 1];
	int top;
}set1stack;
typedef  struct
{
	int set2s[VERTEX_NUM + 1];
	int top; //�߶Ⱥ������±�һ��
}set2stack;
typedef  struct
{
	int set3s[VERTEX_NUM + 1];
	int top; //�߶Ⱥ������±�һ��
}set3stack;
typedef struct node
{
	int adjvertex;
	node* next;
	int flag; //1��ʾ�Ǹ�ǿ�����ĵ㣬2��ʾ�Ǹ�ǿ�����ĺ�̵�
}EdgeNode;
typedef struct {
	int no;
	EdgeNode* firstedge;
}CNode;

typedef CNode component[VERTEX_NUM + 1];

typedef struct {
	int num;
	component c;
}comp;
typedef struct Tom_ancestors{
	int id;
	key_type *kv;
	struct Tom_ancestors *next;
}LinkList;

typedef struct
{
	btree_node* root;
	index_head* infor;

}BPTree_Index ;

//typedef struct
//{
	vector<int> result_x;
	vector<int> result_y;

//}Result_Array;



nstack ns;
cstack cs;
set1stack set1s;
set2stack set2s;
set3stack set3s;
/**********************************************************/
//the interface for using
index_head *create_index(char *database_name, char *table_name, char *key_name, int key_type, int count, char *file_name);
void *load_index(char *file_name);
btree_node *btree_insert(btree_node *root, key_type *key, int value, index_head *infor);//insert 
int btree_update(btree_node *root, key_type *key, int value, index_head *infor);//update
blist_t *btree_search_range(btree_node *root, key_type *low, key_type *high, index_head *infor);//range search  
btree_node *find_leaf(btree_node *root, key_type *key, index_head *infor);
btree_record *make_new_btree_record(int value, index_head *infor, key_type *key);
btree_node *make_new_btree_node(index_head *infor, key_type *key);
btree_node *make_new_leaf(index_head *infor, key_type *key);
btree_node *make_new_tree(key_type *key, int value, index_head *infor);
btree_node *make_new_root(btree_node *left, btree_node *right, key_type *key, index_head *infor, key_type *tkey);
btree_node *insert_into_parent(btree_node *root, btree_node *left, btree_node *right, key_type *key, index_head *infor);
void insert_into_btree_node(btree_node *nd, btree_node *right, int index, key_type *key, index_head *infor);
btree_node *insert_into_btree_node_after_splitting(btree_node *root, btree_node *nd, btree_node *right, int index, key_type *key, index_head *infor);
btree_node *insert_into_leaf_after_splitting(btree_node *root, btree_node *leaf, int index, key_type *key, btree_record *rec, index_head *infor);
void insert_into_leaf(btree_node *leaf, int index, key_type *key, btree_record *rec, index_head *infor);
int get_btree_node_index(btree_node *nd, void *base);
void distribute_btree_nodes(btree_node *nd, btree_node *neighbor, int nd_index, index_head *inforype, key_type *key);
int get_key_len(key_type *key, int type);
void *alloc_add(int length, int type, index_head *infor, key_type *key); //allocate the free address 
void initialize_keys(key_type *keys, key_type *key_input, int keys_type, key_type *tkey);
int str_cmp(key_type *a, key_type *b, int keys_type);
void init_head_leaf(index_head *infor);
void transitive_closure(btree_node *root, index_head *infor, double btime);
void Forward_transitive(btree_node *root, index_head *infor);
void Create_IndexTable(btree_node *root, index_head *infor);
void SetA(btree_node *root, index_head *infor);
key_type *Search_block(int x, btree_node *root, index_head *infor);


#endif
