#include "tp_boj/Globals.h"

namespace tp_boj
{

//##################################################################################################
std::string getAssociatedFilePath(const std::string& filePath, const std::string& fileName)
{
  std::string directory;

  {
    std::vector<std::string> parts;
    std::vector<std::string> partsA;
    tpSplit(partsA, filePath, '/');
    for(const auto& a : partsA)
    {
      std::vector<std::string> partsB;
      tpSplit(partsB, a, '/');
      for(const auto& b : partsB)
        parts.push_back(b);
    }

    if(!parts.empty())
    {
      parts.pop_back();
      for(const auto& part : parts)
        directory += part + '/';
    }
  }

  return directory + fileName;
}

}
