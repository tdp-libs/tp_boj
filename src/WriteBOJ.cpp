#include "tp_boj/WriteBOJ.h"

#include "tp_utils/FileUtils.h"

#include <cctype>

namespace tp_boj
{

//##################################################################################################
void writeObjectAndResourcesToFile(const std::vector<tp_math_utils::Geometry3D>& object,
                                   const std::string& filePath,
                                   const std::function<void(const tp_utils::StringID&, const std::string&)>& saveTexture,
                                   const std::function<void(const tp_utils::StringID&, const tp_utils::StringID&, const std::string&)>& saveExternalFile,
                                   const tp_math_utils::ExtractTextureIDs& extractTextureIDs)
{
  std::string directory = getAssociatedFilePath(filePath);

  std::string objectData = serializeObject(object, [&](const tp_utils::StringID& name)
  {
    if(name.isValid())
      saveTexture(name, directory + cleanTextureName(name) + ".png");
  },
  [&](const tp_utils::StringID& type, const tp_utils::StringID& name)
  {
    if(type.isValid() && name.isValid())
      saveExternalFile(type, name, directory + cleanTextureName(name));
  },
  extractTextureIDs);
  tp_utils::writeBinaryFile(filePath, objectData);
}

//##################################################################################################
void writeObjectAndResourcesToData(const std::vector<tp_math_utils::Geometry3D>& object,
                                   const std::string& filePath,
                                   const std::function<void(const tp_utils::StringID&, const std::string&)>& saveTexture,
                                   const std::function<void(const tp_utils::StringID&, const tp_utils::StringID&, const std::string&)>& saveExternalFile,
                                   const std::function<void(const std::string& path, const std::string& data, bool binary)>& saveFile,
                                   const tp_math_utils::ExtractTextureIDs& extractTextureIDs)
{
  std::string directory = getAssociatedFilePath(filePath);

  std::string objectData = serializeObject(object, [&](const tp_utils::StringID& name)
  {
    if(name.isValid())
      saveTexture(name, directory + cleanTextureName(name) + ".png");
  },
  [&](const tp_utils::StringID& type, const tp_utils::StringID& name)
  {
    if(type.isValid() && name.isValid())
      saveExternalFile(type, name, directory + cleanTextureName(name));
  },
  extractTextureIDs);
  saveFile(filePath, objectData, true);
}

//##################################################################################################
std::string serializeObject(const std::vector<tp_math_utils::Geometry3D>& object,
                            const std::function<void(const tp_utils::StringID&)>& saveTexture,
                            const std::function<void(const tp_utils::StringID&, const tp_utils::StringID&)>& saveExternalFile,
                            const tp_math_utils::ExtractTextureIDs& extractTextureIDs)
{
  auto run = [&object](const auto& addInt, const auto& addFloat, const auto& addString)
  {
    uint32_t maxVersion=20;

    addInt(uint32_t(0)-maxVersion);
    addInt(uint32_t(object.size()));
    for(const auto& mesh : object)
    {
      addInt(uint32_t(mesh.comments.size()));
      for(const auto& comment : mesh.comments)
        addString(comment);

      addInt(uint32_t(mesh.verts.size()));
      for(const auto& vert : mesh.verts)
      {
        addFloat(vert.vert.x);
        addFloat(vert.vert.y);
        addFloat(vert.vert.z);

        addFloat(vert.texture.x);
        addFloat(vert.texture.y);

        addFloat(vert.normal.x);
        addFloat(vert.normal.y);
        addFloat(vert.normal.z);
      }

      addInt(uint32_t(mesh.indexes.size()));
      for(const auto& index : mesh.indexes)
      {
        if(index.type == mesh.triangleFan)
          addInt(1);
        else if(index.type == mesh.triangleStrip)
          addInt(2);
        else
          addInt(3);

        addInt(uint32_t(index.indexes.size()));
        for(int i : index.indexes)
          addInt(uint32_t(i));
      }

      addString(mesh.material.name.toString());

      {
        nlohmann::json j;
        mesh.material.saveState(j);
        addString(j.dump());
      }

      addFloat(mesh.material.uvTransformation.skewUV.x);
      addFloat(mesh.material.uvTransformation.skewUV.y);
      addFloat(mesh.material.uvTransformation.scaleUV.x);
      addFloat(mesh.material.uvTransformation.scaleUV.y);
      addFloat(mesh.material.uvTransformation.translateUV.x);
      addFloat(mesh.material.uvTransformation.translateUV.y);
      addFloat(mesh.material.uvTransformation.rotateUV);
    }
  };

  size_t resultSize=0;
  {
    auto addInt = [&](uint32_t)
    {
      resultSize+=4;
    };

    auto addFloat = [&](float)
    {
      resultSize+=4;
    };

    auto addString = [&](const std::string& s)
    {
      resultSize+=4;
      resultSize+=s.size();
    };
    run(addInt, addFloat, addString);
  }

  std::string result;
  result.resize(resultSize);
  {
    char* data = result.data();
    auto addInt = [&](uint32_t n)
    {
      memcpy(data, &n, 4);
      data+=4;
    };

    auto addFloat = [&](float n)
    {
      memcpy(data, &n, 4);
      data+=4;
    };

    auto addString = [&](const std::string& s)
    {
      uint32_t n = uint32_t(s.size());
      memcpy(data, &n, 4);
      data+=4;

      memcpy(data, s.data(), s.size());
      data+=s.size();
    };
    run(addInt, addFloat, addString);
  }

  std::unordered_set<tp_utils::StringID> textures;
  std::vector<std::pair<tp_utils::StringID, tp_utils::StringID>> files;
  for(const auto& mesh : object)
  {
    mesh.material.allTextureIDs(textures, extractTextureIDs);
    mesh.material.appendFileIDs(files);
  }

  for(const auto& texture : textures)
    if(texture.isValid())
      saveTexture(texture);
  
  for(const auto& file : files)
    if(file.first.isValid() && file.second.isValid())
      saveExternalFile(file.first, file.second);

  return result;
}

}
