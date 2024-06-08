# Mini DataBase Management System (NITCBase)

## Overview
NITCBase is a mini DataBase Management System developed as part of a university laboratory course. It is a lightweight database system designed for educational purposes, allowing users to perform basic database operations. This README file provides essential information for setting up and running NITCBase.

## Table of Contents
- [Prerequisites](#prerequisites)
- [Getting Started](#getting-started)
- [Installation](#installation)
- [Running NITCBase](#running-nitcbase)
- [Usage](#usage)
- [XFS-Documentation](#xfs-documentation)
- [Contributing](#contributing)
- [License](#license)

## Prerequisites
Before running NITCBase, ensure that you have the following prerequisites installed on your system:
- Linux-based operating system (tested on Ubuntu)
- C/C++ compiler (gcc/g++)
- Git

## Getting Started
Follow these steps to set up and run NITCBase on your system:

### Installation
1. Clone the NITCBase repository to your local machine:
    ```bash
    git clone https://github.com/jagadee23/nitcbase
    ```

2. Navigate to the project directory:
    ```bash
    cd nitcbase/mynitcbase
    ```

3. Build the NITCBase application using the make command:
    ```bash
    make
    ```

4. In case you run into any issues while creating builds, install the necessary packages.

### Running NITCBase
Once you have successfully built NITCBase, you can run it with the following command:
```bash
./nitcbase
```

## Usage
NITCBase provides a simple command-line interface for executing basic database operations. You can interact with the database using various SQL commands. For detailed information on available commands and their usage, refer to the XFS-Documentation provided at the link.

## NITCBase Documentation
For detailed information on architecture, methodology, design, and roadmap, please refer to the NITCBase Documentation.

## Algebra-Layer
The Front End parses SQL-Like queries and converts them into a sequence of algebra layer and schema layer method calls. The algebra layer functions process the basic insert and retrieval requests to and from the database. Retrieval functions will create a target relation into which the retrieved data will be stored.

### Return values
For all functions, output will be similar to this in the algebra layer:
- `SUCCESS`: On successful insert of the given record into the relation
- `E_RELNOTOPEN`: If the relation is not open
- `E_NATTRMISMATCH`: If the actual number of attributes in the relation is different from the provided number of attributes
- `E_ATTRTYPEMISMATCH`: If the actual type of the attribute in the relation is different from the type of the provided attribute in the record
- `E_DISKFULL`: If disk space is not sufficient for inserting the record / index
- `E_NOTPERMITTED`: If `relName` is either `RELATIONCAT` or `ATTRIBUTECAT` (i.e., when the user tries to insert a record into any of the catalogs)

#### Functions

1. **Insert**
    ```c
    int insert(char relName[ATTR_SIZE], int numberOfAttributes, char record[][ATTR_SIZE]);
    ```

2. **Project Specified Attributes**
    ```c
    int project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], int tar_nAttrs, char tar_Attrs[][ATTR_SIZE]);
    ```

3. **Project All Attributes (Copy Relation)**
    ```c
    int project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE]);
    ```

4. **Select**
    ```c
    int select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE]);
    ```

5. **Join**
    ```c
    int join(char srcRelOne[ATTR_SIZE], char srcRelTwo[ATTR_SIZE], char targetRel[ATTR_SIZE], char attrOne[ATTR_SIZE], char attrTwo[ATTR_SIZE]);
    ```

## Block Access Layer
The Block Access layer processes the requests for update/retrieval from the algebra and schema layers and works with disk blocks that are buffered by the Buffer layer.

### Functions

1. **linearSearch**
    ```c
    RecId linearSearch(int relId, char *attrName, Attribute attrVal, int op);
    ```

2. **search**
    ```c
    int search(int relId, Attribute *record, char *attrName, Attribute attrVal, int op);
    ```

3. **insert**
    ```c
    int insert(int relId, union Attribute *record);
    ```

4. **renameRelation**
    ```c
    int renameRelation(char *oldName, char *newName);
    ```

5. **renameAttribute**
    ```c
    int renameAttribute(char *relName, char *oldName, char *newName);
    ```

6. **deleteRelation**
    ```c
    int deleteRelation(char *relName);
    ```

7. **project**
    ```c
    int project(int relId, Attribute *record);
    ```

## B+ Tree Layer
The B+ Tree Layer provides indexing functionality for attributes.

### Functions

1. **bPlusCreate**
    ```c
    int bPlusCreate(int relId, char attrName[ATTR_SIZE]);
    ```

2. **bPlusSearch**
    ```c
    RecId bPlusSearch(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, int op);
    ```

3. **bPlusDestroy**
    ```c
    int bPlusDestroy(int rootBlockNum);
    ```

4. **bPlusInsert**
    ```c
    int bPlusInsert(int relId, char attrName[ATTR_SIZE], union Attribute attrVal, RecId recordId);
    ```

5. **findLeafToInsert**
    ```c
    int findLeafToInsert(int rootBlock, Attribute attrVal, int attrType);
    ```

6. **insertIntoLeaf**
    ```c
    int insertIntoLeaf(int relId, char attrName[ATTR_SIZE], int blockNum, Index entry);
    ```

7. **splitLeaf**
    ```c
    int splitLeaf(int leafBlockNum, Index indices[]);
    ```

8. **insertIntoInternal**
    ```c
    int insertIntoInternal(int relId, char attrName[ATTR_SIZE], int blockNum, Index entry);
    ```

9. **splitInternal**
    ```c
    int splitInternal(int intBlockNum, InternalEntry internalEntries[]);
    ```

10. **createNewRoot**
    ```c
    int createNewRoot(int relId, char attrName[ATTR_SIZE], Attribute attrVal, int lChild, int rChild);
    ```

## Contributing
We welcome contributions to NITCBase! If you would like to contribute to this project, please create a pull request and make sure to follow decent naming conventions. Upon review, your request will be merged if found useful.

## License
NITCBase is licensed under the MIT License.

Thank you for using NITCBase. If you encounter any issues or have questions, please don't hesitate to create an issue on our GitHub repository.
