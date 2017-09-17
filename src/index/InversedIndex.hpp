/*
 * Created by brigbaby on 8/10/17.
 */

#ifndef REVERSEDINDEXSERVICE_REVERSEDINDEX_HPP
#define REVERSEDINDEXSERVICE_REVERSEDINDEX_HPP

#include <iostream>
#include <set>
#include <unordered_map>
#include <json/json.h>
#include <json/value.h>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "Container.hpp"

typedef std::unordered_map<std::string, std::vector<int>> assignmentsInOrder;
class InversedIndex{

private:

	std::vector<DOC*> docs;
	std::vector<CONJUNCTION*> conjunctions;
	std::vector<ASSIGNMENT*> assignments;
	std::unordered_map<int, assignmentsInOrder*> assignmentIndexesMap;

	void MatchingAssignment(const ASSIGNMENT &qry, std::vector<int> &matchingAssignmentIndexes);
	void CountingEachConjunction(std::vector<int>& matchingAssignmentIndexes, std::unordered_map<int, int>& ConjunctionByCount);
	void FilteringConjByEachQueryAssi(std::unordered_map<int, int>& ConjunctionByCount, int sizeofAssi, std::unordered_map<int, int>& matchingConjunction);
	void MatchingDoc(std::unordered_map<int, int>& matchingConjunction, std::vector<std::string>& matchingDocsAddress);

public:

	InversedIndex(){}

	void Insert(const Json::Value& doc);
	void Select(const Json::Value& query, Json::Value& result);
	void makeAssignmentMap();
};

void InversedIndex::MatchingAssignment(const ASSIGNMENT &qry, std::vector<int> &matchingAssignmentIndexes) {
    std::string assignmentKey = qry.name + "_" + qry.value;
    for(int i = qry.conj_size; i >= 0; --i){
		if(assignmentIndexesMap.find(i) != assignmentIndexesMap.end()){
			if((*assignmentIndexesMap[i]).find(assignmentKey) != (*assignmentIndexesMap[i]).end()){
				for(auto idx:(*assignmentIndexesMap[i])[assignmentKey]){
					if(assignments[idx]->relation >= qry.relation)
						matchingAssignmentIndexes.push_back(idx);
				}
			}
		}
	}
	return;
}

void InversedIndex::CountingEachConjunction(std::vector<int>& matchingAssignmentIndexes, std::unordered_map<int, int>& ConjunctionByCount) {
	//ConjunctionByCount's key is index of conjunction,CountingEachConjunction's value is the times that each conjunction's
    // times of appearance in this single loop of matching one attribute of query
    for (auto &idx : matchingAssignmentIndexes) {
        int conjIndex = (*assignments[idx]).conjIndex;
        auto it = ConjunctionByCount.find(conjIndex);
        if (it != ConjunctionByCount.end()) {
            it->second++;
        }
        else {
            ConjunctionByCount.insert(std::pair<int, int>(conjIndex, 1));
        }
    }
    return;
}

void InversedIndex::FilteringConjByEachQueryAssi(std::unordered_map<int, int>& ConjunctionByCount, int sizeofAttibute, std::unordered_map<int, int>& matchingConjunction){
    for(auto it : ConjunctionByCount){
        if(it.second == sizeofAttibute){
            if(matchingConjunction.find(it.first) != matchingConjunction.end()){
                ++matchingConjunction[it.first];
            }
            else matchingConjunction.insert(std::pair<int ,int>(it.first, 1));
        }
    }
}

void InversedIndex::MatchingDoc(std::unordered_map<int, int>& matchingConjunction, std::vector<std::string>& matchingDocsIDs){
	for(auto it : matchingConjunction){
        if(it.second == conjunctions[it.first]->size){
			int docIndex = conjunctions[it.first]->docIndex;
            matchingDocsIDs.push_back(docs[docIndex]->id);
		}
	}
}

void InversedIndex::makeAssignmentMap(){
    if(assignments.size()>0){
        std::cout << "The assignmentIndexesMap is already existed" << std::endl;
        return;
    }
    for(int i=0; i<assignments.size(); ++i){
        std::string assignmentKey = assignments[i]->name + "_" + assignments[i]->value;
        int conj_size = assignments[i]->conj_size;
        if(assignmentIndexesMap.find(conj_size) != assignmentIndexesMap.end()){
            if((*assignmentIndexesMap[conj_size]).find(assignmentKey) != (*assignmentIndexesMap[conj_size]).end())
                (*assignmentIndexesMap[conj_size])[assignmentKey].push_back(i);
            else
                (*assignmentIndexesMap[conj_size])[assignmentKey] = {i};
        }
        else{
            assignmentIndexesMap[conj_size] = new assignmentsInOrder;
            (*assignmentIndexesMap[conj_size])[assignmentKey] = {i};
        }
    }
}

// insert
void InversedIndex::Insert(const Json::Value& doc){
    // doc
    DOC* ptr_doc = new DOC;
    ptr_doc->id = doc["id"].asString();
    ptr_doc->size = doc["conditions"].size();
    docs.push_back(ptr_doc);

    for(int i = 0;i < doc["conditions"].size();i++) {
        Json::Value ConjJson = doc["conditions"][i];
        // conjunction
        CONJUNCTION* ptr_conj = new CONJUNCTION;
        ptr_conj->docIndex = docs.size()-1;
        ptr_conj->size = ConjJson.size();
        conjunctions.push_back(ptr_conj);

        for(auto& FieldName : ConjJson.getMemberNames()){
            for(auto& FieldValue : ConjJson[FieldName]){
                // assignment
                ASSIGNMENT* ptr_assi = new ASSIGNMENT;
                ptr_assi->name = FieldName;
                ptr_assi->value = FieldValue.asString();
                ptr_assi->relation = ConjJson[FieldName].size();
                ptr_assi->conj_size = ptr_conj->size;
                ptr_assi->conjIndex = conjunctions.size()-1;
                assignments.push_back(ptr_assi);
                std::string assignmentKey = ptr_assi->name + "_" + ptr_assi->value;

                if(assignmentIndexesMap.find(ptr_assi->conj_size) != assignmentIndexesMap.end()){
                  if((*assignmentIndexesMap[ptr_assi->conj_size]).find(assignmentKey) != (*assignmentIndexesMap[ptr_assi->conj_size]).end())
                      (*assignmentIndexesMap[ptr_assi->conj_size])[assignmentKey].push_back(assignments.size()-1);
                  else
                      (*assignmentIndexesMap[ptr_assi->conj_size])[assignmentKey] = {assignments.size()-1};
                }
                else{
                    assignmentIndexesMap[ptr_assi->conj_size] = new assignmentsInOrder;
                    (*assignmentIndexesMap[ptr_assi->conj_size])[assignmentKey] = {assignments.size()-1};
                }
            }
        }
    }

    return;
}


// select
void InversedIndex::Select(const Json::Value& query, Json::Value& result){
  // check
  std::cout << std::endl << "----- 0 ------" << std::endl;
  std::cout << "doc size " << docs.size() << std::endl;
  std::cout << "conjunction size " << conjunctions.size() << std::endl;
  std::cout << "assignment size " << assignments.size() << std::endl;
  Json::Value condition = query;
  std::cout << "condition's size: " << condition.size() << std::endl;

  //match the assignment and the conjunction
  std::unordered_map<int, int> matchingConjunction;
  for(auto& FieldName : condition.getMemberNames()) {
      //The times of this loop is the numbers of attribute, same as the numbers of query's assignment
      std::vector<int> matchingAssignmentIndexes;
      for (auto &FieldValue : condition[FieldName]) {
          // The times of this loop is the numbers of each assignment's value
          ASSIGNMENT qry_assi;
          qry_assi.name = FieldName;
          qry_assi.value = FieldValue.asString();
          qry_assi.relation = condition[FieldName].size();
          qry_assi.conj_size = condition.size();
          // match assignment precisely
          MatchingAssignment(qry_assi, matchingAssignmentIndexes);
      }
      std::unordered_map<int, int> ConjunctionByCount;
      CountingEachConjunction(matchingAssignmentIndexes, ConjunctionByCount);
      FilteringConjByEachQueryAssi(ConjunctionByCount, condition[FieldName].size(), matchingConjunction);

  }

  //match the doc
  std::vector<std::string> matchingDocsIDs;
  MatchingDoc(matchingConjunction, matchingDocsIDs);

  Json::Value DocList;
  //for(auto& it : matchingDocsIDs){
  //    DocList.append(it);
  //}
  //result['doc'] = DocList;

  for(int i=0; i<matchingDocsIDs.size(); ++i){
      DocList.append(matchingDocsIDs[i]);
  }
  result = DocList;

  return;
}
#endif //REVERSEDINDEXSERVICE_REVERSEDINDEX_HPP
