#include "OpenRelTable.h"

#include <cstring>
#include <stdlib.h>
#include <stdio.h>

OpenRelTable::OpenRelTable()
{
	for (int i = 0; i < MAX_OPEN; ++i)
	{
		RelCacheTable::relCache[i] = nullptr;
		AttrCacheTable::attrCache[i] = nullptr;
		// tableMetaInfo[i].free = true;
	}
	/* RELATION CACHE */
	/* -------------- */

	RecBuffer relCatBlock(RELCAT_BLOCK);

	// Relation Catalog setup
	Attribute relCatRecord[RELCAT_NO_ATTRS];
	relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);
	// printf("-> %s\n", relCatRecord[0].sVal);

	struct RelCacheEntry relCacheEntry;
	RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);

	relCacheEntry.recId.block = RELCAT_BLOCK;
	relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

	RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry *)malloc(sizeof(RelCacheEntry));
	*(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

	// Attribute Catalog setup
	Attribute relCatRecord2[RELCAT_NO_ATTRS];
	relCatBlock.getRecord(relCatRecord2, RELCAT_SLOTNUM_FOR_ATTRCAT);
	// printf("-> %s\n", relCatRecord2[0].sVal);

	struct RelCacheEntry relCacheEntry2;
	RelCacheTable::recordToRelCatEntry(relCatRecord2, &relCacheEntry2.relCatEntry);

	relCacheEntry2.recId.block = RELCAT_BLOCK;
	relCacheEntry2.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;

	RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry *)malloc(sizeof(RelCacheEntry));
	*(RelCacheTable::relCache[ATTRCAT_RELID]) = relCacheEntry2;

	printf("Relation Cache: SUCCESS\n");

	/* ATTRIBUTE CACHE */
	/* --------------- */

	RecBuffer attrCatBlock(ATTRCAT_BLOCK);

	// Relation Catalog setup
	Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

	AttrCacheEntry *head = (struct AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
	head = nullptr;
	AttrCacheEntry *tail = (struct AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));

	for (int i = 0; i < 6; i++)
	{
		AttrCacheEntry *newNode = (struct AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));

		attrCatBlock.getRecord(attrCatRecord, i);
		AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &newNode->attrCatEntry);

		newNode->recId.block = ATTRCAT_BLOCK;
		newNode->recId.slot = i;
		newNode->attrCatEntry.offset = i;
		newNode->next = nullptr;

		if (head == nullptr)
		{
			head = newNode;
			tail = newNode;
		}
		else
		{
			tail->next = newNode;
			tail = newNode;
		}
	}
	AttrCacheTable::attrCache[RELCAT_RELID] = head;

	// Attribute Catalog setup
	Attribute attrCatRecord2[ATTRCAT_NO_ATTRS];

	AttrCacheEntry *head2 = (struct AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
	head2 = nullptr;
	AttrCacheEntry *tail2 = (struct AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));

	for (int i = 6; i < 12; i++)
	{
		AttrCacheEntry *newNode = (struct AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));

		attrCatBlock.getRecord(attrCatRecord2, i);
		AttrCacheTable::recordToAttrCatEntry(attrCatRecord2, &newNode->attrCatEntry);

		newNode->recId.block = ATTRCAT_BLOCK;
		newNode->recId.slot = i;
		newNode->attrCatEntry.offset = i - 6;
		newNode->next = nullptr;

		if (head2 == nullptr)
		{
			head2 = newNode;
			tail2 = newNode;
		}
		else
		{
			tail2->next = newNode;
			tail2 = newNode;
		}
	}
	AttrCacheTable::attrCache[ATTRCAT_RELID] = head2;

	 //Student Relation setup
	/*Attribute attrCatRecord3[ATTRCAT_NO_ATTRS];

	AttrCacheEntry* head3 = (struct AttrCacheEntry*) malloc(sizeof(AttrCacheEntry));
	head3 = nullptr;
	AttrCacheEntry* tail3 = (struct AttrCacheEntry*) malloc(sizeof(AttrCacheEntry));

	for (int i=12; i<16; i++) {
		AttrCacheEntry* newNode = (struct AttrCacheEntry*) malloc(sizeof(AttrCacheEntry));

		attrCatBlock.getRecord(attrCatRecord2, i);
		AttrCacheTable::recordToAttrCatEntry(attrCatRecord2, &newNode->attrCatEntry);

		newNode->recId.block = ATTRCAT_BLOCK;
		newNode->recId.slot = i;
		//newNode->attrCatEntry.offset = i-6;
		newNode->next = nullptr;

		if (head3 == nullptr) {
			head3 = newNode;
			tail3 = newNode;
		} else {
			tail3->next = newNode;
			tail3 = newNode;
		}
	}
	AttrCacheTable::attrCache[2] = head3;
	*/

	//printf("Attribute Cache: SUCCESS\n\n");

	// TableMetaInfo entries
	// tableMetaInfo[RELCAT_RELID].free = false;
	// strcpy(tableMetaInfo[RELCAT_RELID].relName, RELCAT_RELNAME);

	// tableMetaInfo[ATTRCAT_RELID].free = false;
	// strcpy(tableMetaInfo[ATTRCAT_RELID].relName, ATTRCAT_RELNAME);
}

OpenRelTable::~OpenRelTable()
{
	// free all the memory that you allocated in the constructor
}
/* This function will open a relation having name `relName`.
Since we are currently only working with the relation and attribute catalog, we
will just hardcode it. In subsequent stages, we will loop through all the relations
and open the appropriate one.
*/
/*int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

  // if relname is RELCAT_RELNAME, return RELCAT_RELID
  if(strcmp(relName , RELCAT_ATTR_RELNAME)==0){
	return RELCAT_RELID;
  }
  // if relname is ATTRCAT_RELNAME, return ATTRCAT_RELID
   if(strcmp(relName , ATTRCAT_ATTR_RELNAME)==0){
	return ATTRCAT_RELID;
  }
  return E_RELNOTOPEN;
}*/