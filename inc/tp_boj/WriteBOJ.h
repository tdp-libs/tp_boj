#pragma once

#include "tp_boj/Globals.h" // IWYU pragma: keep
#include "tp_math_utils/Geometry3D.h"

#include <iosfwd>

namespace tp_boj
{

//##################################################################################################
void writeObjectAndResourcesToFile(const std::vector<tp_math_utils::Geometry3D>& object,
                                   const std::string& filePath,
                                   const std::function<void(const tp_utils::StringID&, const std::string&)>& saveTexture,
                                   const std::function<void(const tp_utils::StringID&, const tp_utils::StringID&, const std::string&)>& saveExternalFile,
                                   const tp_math_utils::ExtractTextureIDs& extractTextureIDs);

//##################################################################################################
void writeObjectAndResourcesToData(const std::vector<tp_math_utils::Geometry3D>& object,
                                   const std::string& filePath,
                                   const std::function<void(const tp_utils::StringID&, const std::string&)>& saveTexture,
                                   const std::function<void(const tp_utils::StringID&, const tp_utils::StringID&, const std::string&)>& saveExternalFile,
                                   const std::function<void(const std::string& path, const std::string& data, bool binary)>& saveFile,
                                   const tp_math_utils::ExtractTextureIDs& extractTextureIDs);

//##################################################################################################
std::string serializeObject(const std::vector<tp_math_utils::Geometry3D>& object,
                            const std::function<void(const tp_utils::StringID&)>& saveTexture,
                            const std::function<void(const tp_utils::StringID&, const tp_utils::StringID&)>& saveExternalFile,
                            const tp_math_utils::ExtractTextureIDs& extractTextureIDs);

}
