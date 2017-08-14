//
// Created by harsh on 9/8/17.
//

#include "JSONPathFinder.h"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"


#include <stack>
#include <string>
#include <sstream>
#include <cstdlib>

using namespace std;
using namespace rapidjson;

JSONPathFinder::JSONPathFinder(std::string jsonPath) {

}

std::vector<std::string>
JSONPathFinder::GetPaths(std::string doc) {

    Document document;
    document.Parse(doc);

    vector<string> paths;
    vector<string> pathNames;
    WalkNode(document, pathNames, paths);
    return paths;
}

void
JSONPathFinder::WalkNode(const rapidjson::Value &node,
                         std::vector<std::string> &nodesNames,
                         std::vector<std::string> &paths)
{
    if (node.IsArray())
    {
        int size = node.Size();
        for(int i = 0; i < size; i++) {
            nodesNames.emplace_back(/*string(itoa(i)*/ "0");
            paths.emplace_back(BuildPath(nodesNames));
            const Value& childValue = node[i];
            WalkNode(childValue, nodesNames, paths);
            nodesNames.pop_back();
        }

    } else if (node.IsObject())
    {
        for(const auto& child : node.GetObject())
        {
            string childName(child.name.GetString());
            nodesNames.emplace_back(childName);
            paths.emplace_back(BuildPath(nodesNames));
            WalkNode(child.value, nodesNames, paths);
            nodesNames.pop_back();
        }

    } else
    {
        return;
    }
}

std::string JSONPathFinder::BuildPath(std::vector<std::string> &pathNames) {
    stringstream pathStm;
    for (auto name : pathNames) {
        pathStm << "/" << name;
    }
    return pathStm.str();
}
