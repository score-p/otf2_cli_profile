#pragma once

using namespace std;

#include "datastructs.h"

struct Functioncords
{
    uint64_t start;
    uint64_t center;
    uint64_t end;
};

map <uint64_t, Functioncords> FunctionCoordinateMap;

SystemTreeClass baseSystemClass;
SystemTreeClass groupSystemClass;
SystemTreeClass topSystemClass;
uint64_t baseSystemMembers;
uint64_t groupSystemMembers;
uint64_t topSystemMembers;
map<SystemTreeClass, uint64_t > systemClasses;

typedef map<uint64_t, uint64_t> ParentIdMap;
typedef map<uint64_t, SystemTreeNode* > ChildrenMap;

ParentIdMap parentMap;

void getChildrenByClass(
        AllData& allData,
        SystemTreeNode &node,
        SystemTreeClass treeClassType,
        ChildrenMap &resultMap)
{
    for (set<uint64_t>::const_iterator iter = node.children.begin();
            iter != node.children.end(); ++iter)
    {
        uint64_t childId = *iter;
        SystemTreeNode* childTreeNode = &(allData.systemTreeIdToSystemTreeNodeMap[childId]);
        if (childTreeNode->class_id == treeClassType)
        {
          resultMap.insert(make_pair(childId, childTreeNode));
          continue;
        }
        else
        {
          getChildrenByClass(allData, *childTreeNode, treeClassType, resultMap);
        }
    }
}

uint64_t getParentIdByClass(
        AllData& allData,
        uint64_t nodeId,
        SystemTreeClass treeClassType)
{
  if (treeClassType == SYSTEMTREE_UNKNOWN || treeClassType == SYSTEMTREE_OTHER)
    return 0;

  SystemTreeNode& node = allData.systemTreeIdToSystemTreeNodeMap[nodeId];
  if (node.class_id == treeClassType)
    return nodeId;
  else
    return getParentIdByClass(allData, node.parent, treeClassType);
}

static string getSystemTreeClassName(SystemTreeClass treeClass)
{
  switch (treeClass)
  {
    case SYSTEMTREE_BLADE:
      return "blade";
    case SYSTEMTREE_CABINET:
      return "cabinet";
    case SYSTEMTREE_CABINET_ROW:
      return "cabinet row";
    case SYSTEMTREE_CAGE:
      return "cage";
    case SYSTEMTREE_LOCATION:
      return "location";
    case SYSTEMTREE_LOCATION_GROUP:
      return "location group";
    case SYSTEMTREE_MACHINE:
      return "machine";
    case SYSTEMTREE_NODE:
      return "node";
    case SYSTEMTREE_OTHER:
      return "other";
    default:
      return "unknown";
  }
}

