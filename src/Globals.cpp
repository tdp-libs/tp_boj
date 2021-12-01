#include "tp_boj/Globals.h"

namespace tp_boj
{

//##################################################################################################
std::string cleanTextureName(const tp_utils::StringID& name)
{
  std::string r = name.keyString();

  for(size_t i=0; i<r.size(); i++)
    r[i] = char(std::tolower(r[i]));

  auto getBack = [&](char del)
  {
    std::vector<std::string> parts;
    tpSplit(parts, r, del);
    r = parts.back();
  };

  auto getFront = [&](const std::string& del)
  {
    std::vector<std::string> parts;
    tpSplit(parts, r, del);
    r = parts.front();
  };

  getBack('/');
  getBack('\\');

  getFront(".png");
  getFront(".jpg");
  getFront(".jpeg");
  getFront(".bmp");
  getFront(".tga");

  for(char& c : r)
  {
    if(c>='a' && c<='z')
      continue;

    if(c>='0' && c<='9')
      continue;

    c = '_';
  }

  return r;
}

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
