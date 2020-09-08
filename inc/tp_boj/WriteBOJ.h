#ifndef tp_boj_WriteBOJ_h
#define tp_boj_WriteBOJ_h

#include "tp_boj/Globals.h"
#include "tp_maps/layers/Geometry3DLayer.h"

#include <iosfwd>

namespace tp_boj
{
//##################################################################################################
std::string cleanTextureName(const tp_utils::StringID& name);

//##################################################################################################
void writeObjectAndTexturesToFile(const std::vector<tp_maps::Geometry3D>& object,
                                  const std::string& filePath,
                                  const std::function<void(const tp_utils::StringID&, const std::string&)>& saveTexture);

//##################################################################################################
std::string serializeObject(const std::vector<tp_maps::Geometry3D>& object,
                            const std::function<void(const tp_utils::StringID&)>& saveTexture);

}

#endif
