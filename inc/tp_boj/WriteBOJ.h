#ifndef tp_boj_WriteBOJ_h
#define tp_boj_WriteBOJ_h

#include "tp_boj/Globals.h"
#include "tp_math_utils/Geometry3D.h"

#include <iosfwd>

namespace tp_boj
{

//##################################################################################################
void writeObjectAndResourcesToFile(const std::vector<tp_math_utils::Geometry3D>& object,
                                  const std::string& filePath,
                                  const std::function<void(const tp_utils::StringID&, const std::string&)>& saveTexture,
                                  const std::function<void(const tp_utils::StringID&, const tp_utils::StringID&, const std::string&)>& saveExternalFile);

//##################################################################################################
void writeObjectAndResourcesToData(const std::vector<tp_math_utils::Geometry3D>& object,
                                  const std::string& filePath,
                                  const std::function<void(const tp_utils::StringID&, const std::string&)>& saveTexture,
                                  const std::function<void(const tp_utils::StringID&, const tp_utils::StringID&, const std::string&)>& saveExternalFile,
                                  const std::function<void(const std::string& path, const std::string& data, bool binary)>& saveFile);

//##################################################################################################
std::string serializeObject(const std::vector<tp_math_utils::Geometry3D>& object,
                            const std::function<void(const tp_utils::StringID&)>& saveTexture,
                            const std::function<void(const tp_utils::StringID&, const tp_utils::StringID&)>& saveExternalFile);

}

#endif
