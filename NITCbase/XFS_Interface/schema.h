#ifndef NITCBASE_SCHEMA_H
#define NITCBASE_SCHEMA_H
#include "define/constants.h"
#include "disk_structures.h"

int createRel(char relname[16], int nAttrs, char attrs[][ATTR_SIZE], int attrtypes[]);
int deleteRel(char relname[ATTR_SIZE]);
int renameRel(char oldRelName[ATTR_SIZE],char newRelName[ATTR_SIZE]);
int renameAtrribute(char relName[ATTR_SIZE], char oldAttrName[ATTR_SIZE],char newAttrName[ATTR_SIZE]);
int openRel(char relationName[ATTR_SIZE]);
int closeRel(char rel_name[ATTR_SIZE]);
int closeRel(int relid);
int createIndex(char *relationName, char *attrName);
int dropIndex(char *relationName, char *attrName);

Attribute *make_relcatrec(char relname[16], int nAttrs, int nRecords, int firstBlock, int lastBlock);
Attribute* make_attrcatrec(char relname[ATTR_SIZE], char attrname[ATTR_SIZE], int attrtype, int rootBlock, int offset);

#endif //NITCBASE_SCHEMA_H