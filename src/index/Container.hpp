/*
 * Created by brigbaby on 8/15/17.
 */

#ifndef REVERSEDINDEXSERVICE_CONTAINER_HPP
#define REVERSEDINDEXSERVICE_CONTAINER_HPP

#include <iostream>

typedef struct{
	std::string id;
	int size;//maybe it's useless
} DOC;

typedef struct{
	long doc_belong;//address value of corresponding doc
	int size;
} CONJUNCTION;

typedef struct{
	long conj_belong;//address value of corresponding conj
	std::string name;
	std::string value;
	int relation;
	int conj_size;
} ASSIGNMENT;

#endif //REVERSEDINDEXSERVICE_CONTAINER_HPP
