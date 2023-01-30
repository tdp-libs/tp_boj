#ifndef tp_boj_Globals_h
#define tp_boj_Globals_h

#include "tp_utils/StringID.h"

#if defined(TP_BOJ_LIBRARY)
#  define TP_BOJ_EXPORT TP_EXPORT
#else
#  define TP_BOJ_EXPORT TP_IMPORT
#endif

//##################################################################################################
//! Load 3D models from .boj files.
namespace tp_boj
{
//##################################################################################################
std::string cleanTextureName(const tp_utils::StringID& name);

//##################################################################################################
std::string getAssociatedFilePath(const std::string& filePath, const std::string& filename=std::string());

}

#endif
