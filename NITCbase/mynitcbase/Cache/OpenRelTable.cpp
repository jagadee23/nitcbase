#include "OpenRelTable.h"

#include <cstring>
#include <cstdlib>

OpenRelTable::OpenRelTable() {

  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
  }

  /************ Setting up Relation Cache entries ************/
  // (we need to populate relation cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Relation Cache Table ****/
  RecBuffer relCatBlock(RELCAT_BLOCK);

  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

  /**** setting up Attribute Catalog relation in the Relation Cache Table ****/

  // set up the relation cache entry for the attribute catalog similarly
  // from the record at RELCAT_SLOTNUM_FOR_ATTRCAT
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);

  // set the value at RelCacheTable::relCache[ATTRCAT_RELID]
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;

  RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[ATTRCAT_RELID]) = relCacheEntry;

  /**** setting up Student Catalog relation in the Relation Cache Table ****/

  relCatBlock.getRecord(relCatRecord, ATTRCAT_RELID + 1);

  // set the value at RelCacheTable::relCache[ATTRCAT_RELID+1]
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = ATTRCAT_RELID + 1;

  RelCacheTable::relCache[ATTRCAT_RELID + 1] = (struct RelCacheEntry *)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[ATTRCAT_RELID + 1]) = relCacheEntry;

  /************ Setting up Attribute cache entries ************/
  // (we need to populate attribute cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Attribute Cache Table ****/
  RecBuffer attrCatBlock(ATTRCAT_BLOCK);

  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

  // iterate through all the attributes of the relation catalog and create a linked
  // list of AttrCacheEntry (slots 0 to 5)
  // for each of the entries, set
  //    attrCacheEntry.recId.block = ATTRCAT_BLOCK;
  //    attrCacheEntry.recId.slot = i   (0 to 5)
  //    and attrCacheEntry.next appropriately
  // NOTE: allocate each entry dynamically using malloc
  AttrCacheEntry * head , * curr;
  head = curr = (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
  int i;
  for (i = 0; i < RELCAT_NO_ATTRS - 1; i++) {
    attrCatBlock.getRecord(attrCatRecord, i);
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &curr->attrCatEntry);
    curr->recId.slot = i;
    curr->recId.block = ATTRCAT_BLOCK;
    curr->next = (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    curr = curr->next;
  }
  attrCatBlock.getRecord(attrCatRecord, i);
  AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &curr->attrCatEntry);
  curr->next = nullptr;
  curr->recId.slot = RELCAT_NO_ATTRS - 1;
  curr->recId.block = ATTRCAT_BLOCK;
  i++;

  // set the next field in the last entry to nullptr

  AttrCacheTable::attrCache[RELCAT_RELID] = head;

  /**** setting up Attribute Catalog relation in the Attribute Cache Table ****/

  // set up the attributes of the attribute cache similarly.
  // read slots 6-11 from attrCatBlock and initialise recId appropriately
  head = curr = (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
  for (; i < RELCAT_NO_ATTRS + ATTRCAT_NO_ATTRS - 1; i++) {
    attrCatBlock.getRecord(attrCatRecord, i);
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &curr->attrCatEntry);
    curr->recId.slot = i;
    curr->recId.block = ATTRCAT_BLOCK;
    curr->next = (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
    curr = curr->next;
  }
  attrCatBlock.getRecord(attrCatRecord, i);
  AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &curr->attrCatEntry);
  curr->next = nullptr;
  curr->recId.slot = i;
  curr->recId.block = ATTRCAT_BLOCK;
  i++;

  // set the value at AttrCacheTable::attrCache[ATTRCAT_RELID]
  AttrCacheTable::attrCache[ATTRCAT_RELID] = head;

  /**** setting up Student Catalog relation in the Attribute Cache Table ****/
  i = 12;
  head = curr = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
  for (; i < RELCAT_NO_ATTRS + ATTRCAT_NO_ATTRS + relCacheEntry.relCatEntry.numAttrs - 1; i++)
  {
      attrCatBlock.getRecord(attrCatRecord, i);
      AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &curr->attrCatEntry);
      curr->recId.slot = i;
      curr->recId.block = ATTRCAT_BLOCK;
      curr->next = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
      curr = curr->next;
  }
  attrCatBlock.getRecord(attrCatRecord, i);
  AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &curr->attrCatEntry);
  curr->next = nullptr;
  curr->recId.slot = i;
  curr->recId.block = ATTRCAT_BLOCK;
  i++;

  // set the value at AttrCacheTable::attrCache[ATTRCAT_RELID+1]
  AttrCacheTable::attrCache[ATTRCAT_RELID + 1] = head;
  
}

OpenRelTable::~OpenRelTable() {
  // free all the memory that you allocated in the constructor
}

/* This function will open a relation having name `relName`.
Since we are currently only working with the relation and attribute catalog, we
will just hardcode it. In subsequent stages, we will loop through all the relations
and open the appropriate one.
*/
int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

  // if relname is RELCAT_RELNAME, return RELCAT_RELID
  if (strcmp(relName, RELCAT_RELNAME) == 0)
  {
    return RELCAT_RELID;
  }
  
  // if relname is ATTRCAT_RELNAME, return ATTRCAT_RELID
  if(strcmp(relName, ATTRCAT_RELNAME) == 0) {
    return ATTRCAT_RELID;
  }
  if (strcmp(relName, "Students") == 0) {
    return 2;
  }

  return E_RELNOTOPEN;
}
