#ifndef tp_boj_ReadBOJ_h
#define tp_boj_ReadBOJ_h

#include "tp_boj/Globals.h" // IWYU pragma: keep

#include "tp_math_utils/Geometry3D.h"

#include <iosfwd>
#include <unordered_map>

namespace tp_boj
{

//##################################################################################################
std::vector<tp_math_utils::Geometry3D> readObjectAndTexturesFromFile(const std::string& filePath,
                                                                     std::unordered_map<tp_utils::StringID, std::string>& texturePaths);

//##################################################################################################
std::vector<tp_math_utils::Geometry3D> deserializeObject(const std::string& data);
}

#endif
