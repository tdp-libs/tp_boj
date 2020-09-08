#ifndef tp_boj_ReadBOJ_h
#define tp_boj_ReadBOJ_h

#include "tp_boj/Globals.h"
#include "tp_maps/layers/Geometry3DLayer.h"

#include <iosfwd>

namespace tp_boj
{

//##################################################################################################
std::vector<tp_maps::Geometry3D> readObjectAndTexturesFromFile(const std::string& filePath,
                                                               std::unordered_map<tp_utils::StringID, std::string>& texturePaths,
                                                               int triangleFan,
                                                               int triangleStrip,
                                                               int triangles);

//##################################################################################################
std::vector<tp_maps::Geometry3D> deserializeObject(const std::string& data,
                                                   int triangleFan,
                                                   int triangleStrip,
                                                   int triangles);
}

#endif
