#include <stdint.h>
#include <map>
#include <cstddef>
#include <iostream>



#ifndef CALLPATHREE_H
#define CALLPATHREE_H

using namespace std;

struct MessageData;
struct FunctionData;

struct CallPathTreeNode{

	uint64_t function_id;
	uint64_t parent_id;

	CallPathTreeNode(){}

	CallPathTreeNode( uint64_t _function_id, uint64_t _parent_id ) {

		function_id = _function_id;
		parent_id = _parent_id; //node id NOT function id!

	}

};


struct CallPathTree{

	std::map<uint64_t, CallPathTreeNode> tree; //node_id|node
	std::map<std::pair<uint64_t, uint64_t>, uint64_t> mapping; //map< pair<old NodeId, Rank> , NodeId in merged Tree >

    std::map<std::pair<uint64_t, uint64_t>, MessageData> mData;
    std::map< std::pair< uint64_t, std::pair< uint64_t, uint64_t > >, FunctionData> fData;

	//Function so you can insert nodes into an !UNMERGED! tree
	uint64_t insertNode(CallPathTreeNode aNode, uint64_t myRank){

		std::map<uint64_t, CallPathTreeNode>::iterator it = this->tree.find(aNode.parent_id);
		std::map<uint64_t, CallPathTreeNode>::iterator it_e = this->tree.end();

		if(aNode.parent_id != 0 && it == it_e){

			return 0; //no parent - should be error -> inside the file or the mpi communication

		}

		if(aNode.parent_id == 0 && it == it_e) {

			it = this->tree.begin(); //additional cost for every new root node

		}
	
		for(; it != it_e; it++){

			if(it->second.function_id == aNode.function_id && it->second.parent_id == aNode.parent_id) {

					this->mapping.insert(std::pair<std::pair<uint64_t, uint64_t>, uint64_t >(std::pair<uint64_t, uint64_t>(it->first, myRank), it->first));

				return it->first; //node already exists -> same function + same parent

			}

		}

		this->tree.insert(std::pair<uint64_t, CallPathTreeNode>(this->tree.size() + 1, aNode));
		this->mapping.insert(std::pair<std::pair<uint64_t, uint64_t>, uint64_t >(std::pair<uint64_t, uint64_t>(this->tree.size(), myRank), this->tree.size()));
		
		return this->tree.size();

	}
	

    /* function to insert mapping-data (from one mapping) into another - all with the same local node_id */
	void intoMapping(std::map<std::pair<uint64_t, uint64_t>, uint64_t>rhs_mapping, uint64_t rhs_nodeID, uint64_t nNodeId){

		std::map<std::pair<uint64_t, uint64_t>, uint64_t>::iterator it_map = rhs_mapping.begin();
		std::map<std::pair<uint64_t, uint64_t>, uint64_t>::iterator it_map_e = rhs_mapping.end();

		for(; it_map != it_map_e; it_map++){

			if(it_map->second == rhs_nodeID){

                this->mapping.insert(std::pair<std::pair<uint64_t, uint64_t>, uint64_t>(std::pair<uint64_t, uint64_t>(it_map->first.first, it_map->first.second), nNodeId ) );

			}

		}


	}
    
};



#endif

