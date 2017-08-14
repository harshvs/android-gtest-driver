//
// Created by harsh on 9/8/17.
//

#ifndef TESTCPP_JSONPATHFINDER_H
#define TESTCPP_JSONPATHFINDER_H

#include <string>
#include <vector>
#include <stack>

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

class JSONPathFinder {
public:
    JSONPathFinder(std::string jsonPath);
    std::vector<std::string> GetPaths(std::string doc);
private:
    // std::vector<std::string> m_pathTokens;
    void WalkNode(const rapidjson::Value &node,
                  std::vector<std::string> &nodesNames,
                  std::vector<std::string> &paths);
    std::string BuildPath(std::vector<std::string>& pathNames);
};

#endif //TESTCPP_JSONPATHFINDER_H
