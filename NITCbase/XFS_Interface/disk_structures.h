#ifndef NITCBASE_DISK_STRUCTURES_H
#define NITCBASE_DISK_STRUCTURES_H

#include <cstdint>
#include "define/constants.h"
#include "define/errors.h"

typedef int relId;
typedef struct recId {
	int block;
	int slot;
} recId;

typedef struct indexId {
	int block;
	int index;
} indexId;

typedef struct RelCatEntry {
	char rel_name[ATTR_SIZE];
	int num_attr;
	int num_rec;
	int first_blk;
	int last_blk;           //Head of linked list of blocks
	int num_slots_blk;      //Number of slots in a block
} RelCatEntry;

typedef struct AttrCatEntry {
	char rel_name[ATTR_SIZE];
	char attr_name[ATTR_SIZE];
	int attr_type;          // can be INT/FLOAT/STR
	bool primary_flag;      // Currently unused
	int root_block;         // root block# of the B+ tree if exists, else -1
	int offset;             // offset of the attribute in the relation
} AttrCatEntry;

typedef struct RecBlock {
	int32_t blockType;
	int32_t pblock;
	int32_t lblock;
	int32_t rblock;
	int32_t numEntries;
	int32_t numAttrs;
	int32_t numSlots;
	unsigned char reserved[4];
	unsigned char slotMap_Records[BLOCK_SIZE - 104];
	unsigned char unused[72];
} RecBlock;

typedef struct HeadInfo {
	int32_t blockType;
	int32_t pblock;
	int32_t lblock;
	int32_t rblock;
	int32_t numEntries;
	int32_t numAttrs;
	int32_t numSlots;
	unsigned char reserved[4];
} HeadInfo;

typedef union Attribute {
	double nval;
	char sval[ATTR_SIZE];
} Attribute;

typedef struct InternalEntry {
	int32_t lChild;
	union Attribute attrVal;
	int32_t rChild;
} InternalEntry;

typedef struct Index {
	union Attribute attrVal;
	int32_t block;
	int32_t slot;
	unsigned char unused[8];
} Index;

#endif //NITCBASE_DISK_STRUCTURES_H
