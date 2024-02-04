#include "Schema.h"

#include <cmath>
#include <cstring>

int Schema::openRel(char relName[ATTR_SIZE])
{
  int ret = OpenRelTable::openRel(relName);

  // the OpenRelTable::openRel() function returns the rel-id if successful
  // a valid rel-id will be within the range 0 <= relId < MAX_OPEN and any
  // error codes will be negative
  if (ret >= 0)
  {
    return SUCCESS;
  }

  // otherwise it returns an error message
  return ret;
}

int Schema::closeRel(char relName[ATTR_SIZE])
{
  /* relation is relation catalog or attribute catalog */
  if (strcmp(relName, RELCAT_RELNAME) == 0 || strcmp(relName, ATTRCAT_RELNAME) == 0)
  {
    return E_NOTPERMITTED;
  }

  // this function returns the rel-id of a relation if it is open or
  // E_RELNOTOPEN if it is not. we will implement this later.
  int relId = OpenRelTable::getRelId(relName);

  if (relId == E_RELNOTOPEN)
  {
    return E_RELNOTOPEN;
  }

  return OpenRelTable::closeRel(relId);
}

/* This method changes the relation name of specified relation to new name as specified in arguments. */
int Schema::renameRel(char oldRelName[ATTR_SIZE], char newRelName[ATTR_SIZE])
{
  // if the oldRelName or newRelName is either Relation Catalog or Attribute Catalog,
  // return E_NOTPERMITTED
  // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
  // you may use the following constants: RELCAT_NAME and ATTRCAT_NAME)
  if (
      strcmp(newRelName, RELCAT_RELNAME) == 0 ||
      strcmp(newRelName, ATTRCAT_RELNAME) == 0 ||
      strcmp(oldRelName, RELCAT_RELNAME) == 0 ||
      strcmp(oldRelName, ATTRCAT_RELNAME) == 0)
  {
    return E_NOTPERMITTED;
  }

  // if the relation is open
  //    (check if OpenRelTable::getRelId() returns E_RELNOTOPEN)
  //    return E_RELOPEN
  int retVal = OpenRelTable::getRelId(oldRelName);
  if (retVal != E_RELNOTOPEN)
  {
    return E_RELOPEN;
  }

  // retVal = BlockAccess::renameRelation(oldRelName, newRelName);
  retVal = BlockAccess::renameRelation(oldRelName, newRelName);
  // return retVal
  return retVal;
}

/* This method changes the name of an attribute/column present in
 a specified relation, to new name as specified in arguments. */
int Schema::renameAttr(char *relName, char *oldAttrName, char *newAttrName)
{
  // if the relName is either Relation Catalog or Attribute Catalog,
  // return E_NOTPERMITTED
  // (check if the relation names are either "RELATIONCAT" and "ATTRIBUTECAT".
  // you may use the following constants: RELCAT_NAME and ATTRCAT_NAME)
  if (
      strcmp(relName, RELCAT_RELNAME) == 0 ||
      strcmp(relName, ATTRCAT_RELNAME) == 0)
  {
    return E_NOTPERMITTED;
  }

  // if the relation is open
  //    (check if OpenRelTable::getRelId() returns E_RELNOTOPEN)
  //    return E_RELOPEN
  int retVal = OpenRelTable::getRelId(relName);
  if (retVal == E_RELOPEN)
  {
    return E_RELOPEN;
  }

  // Call BlockAccess::renameAttribute with appropriate arguments.

  // return the value returned by the above renameAttribute() call
  return BlockAccess::renameAttribute(relName, oldAttrName, newAttrName);
}