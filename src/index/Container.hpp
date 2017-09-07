/*
 * Created by brigbaby on 8/15/17.
 */
 
#ifndef REVERSEDINDEXSERVICE_CONTAINER_HPP
#define REVERSEDINDEXSERVICE_CONTAINER_HPP

#include <iostream>

typedef struct{
	std::string id;
	int size;
} DOC;

typedef struct{
    int docIndex;
	int size;
} CONJUNCTION;

typedef struct{
	int conjIndex;
    std::string name;
	std::string value;
	int relation;
	int conj_size;
} ASSIGNMENT;

#endif //REVERSEDINDEXSERVICE_CONTAINER_HPP
