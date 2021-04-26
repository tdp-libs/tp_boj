#include "tp_boj/WriteBOJ.h"

#include "tp_utils/FileUtils.h"

#include <cctype>

namespace tp_boj
{

//##################################################################################################
void writeObjectAndTexturesToFile(const std::vector<tp_math_utils::Geometry3D>& object,
                                  const std::string& filePath,
                                  const std::function<void(const tp_utils::StringID&, const std::string&)>& saveTexture)
{
  std::string directory = getAssociatedFilePath(filePath);

  std::string objectData = serializeObject(object, [&](const tp_utils::StringID& name)
  {
    if(name.isValid())
      saveTexture(name, directory + cleanTextureName(name) + ".png");
  });
  tp_utils::writeBinaryFile(filePath, objectData);
}

//##################################################################################################
std::string serializeObject(const std::vector<tp_math_utils::Geometry3D>& object,
                            const std::function<void(const tp_utils::StringID&)>& saveTexture)
{
  auto run = [&object](const auto& addInt, const auto& addFloat, const auto& addString)
  {
    addInt(uint32_t(0)-9); // Version 9
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

        addFloat(vert.color.x);
        addFloat(vert.color.y);
        addFloat(vert.color.z);
        addFloat(vert.color.w);

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

      addString(mesh.material.name.keyString());

      addFloat(mesh.material.albedo.x);
      addFloat(mesh.material.albedo.y);
      addFloat(mesh.material.albedo.z);

      addFloat(mesh.material.alpha);

      addFloat(mesh.material.roughness);
      addFloat(mesh.material.metalness);
      addFloat(mesh.material.transmission);
      addFloat(mesh.material.transmissionRoughness);
      addFloat(mesh.material.ior);

      addFloat(mesh.material.sheen);
      addFloat(mesh.material.sheenTint);
      addFloat(mesh.material.clearCoat);
      addFloat(mesh.material.clearCoatRoughness);

      addFloat(mesh.material.sssScale);

      addFloat(mesh.material.sssRadius.x);
      addFloat(mesh.material.sssRadius.y);
      addFloat(mesh.material.sssRadius.z);

      addFloat(mesh.material.sss.x);
      addFloat(mesh.material.sss.y);
      addFloat(mesh.material.sss.z);

      addFloat(mesh.material.emission.x);
      addFloat(mesh.material.emission.y);
      addFloat(mesh.material.emission.z);

      addFloat(mesh.material.emissionScale);

      addFloat(mesh.material.velvet.x);
      addFloat(mesh.material.velvet.y);
      addFloat(mesh.material.velvet.z);

      addFloat(mesh.material.velvetScale);

      addFloat(mesh.material.heightScale);
      addFloat(mesh.material.heightMidlevel);

      addFloat(mesh.material.useAmbient);
      addFloat(mesh.material.useDiffuse);
      addFloat(mesh.material.useNdotL);
      addFloat(mesh.material.useAttenuation);
      addFloat(mesh.material.useShadow);
      addFloat(mesh.material.useLightMask);
      addFloat(mesh.material.useReflection);

      addFloat(mesh.material.albedoScale);

      addInt(mesh.material.tileTextures?1:0);

      addString(cleanTextureName(mesh.material.               albedoTexture));
      addString(cleanTextureName(mesh.material.                alphaTexture));
      addString(cleanTextureName(mesh.material.              normalsTexture));
      addString(cleanTextureName(mesh.material.            roughnessTexture));
      addString(cleanTextureName(mesh.material.            metalnessTexture));
      addString(cleanTextureName(mesh.material.             emissionTexture));
      addString(cleanTextureName(mesh.material.                  sssTexture));
      addString(cleanTextureName(mesh.material.               heightTexture));
      addString(cleanTextureName(mesh.material.         transmissionTexture));
      addString(cleanTextureName(mesh.material.transmissionRoughnessTexture));
      addString(cleanTextureName(mesh.material.                sheenTexture));
      addString(cleanTextureName(mesh.material.            sheenTintTexture));
      addString(cleanTextureName(mesh.material.            clearCoatTexture));
      addString(cleanTextureName(mesh.material.   clearCoatRoughnessTexture));
      addString(cleanTextureName(mesh.material.               velvetTexture));
      addString(cleanTextureName(mesh.material.         velvetFactorTexture));
      addString(cleanTextureName(mesh.material.             sssScaleTexture));
      addString(cleanTextureName(mesh.material.     iridescentFactorTexture));
      addString(cleanTextureName(mesh.material.     iridescentOffsetTexture));
      addString(cleanTextureName(mesh.material.  iridescentFrequencyTexture));
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
  for(const auto& mesh : object)
    for(const auto& id : mesh.material.allTextures())
      textures.insert(id);

  for(const auto& texture : textures)
    if(texture.isValid())
      saveTexture(texture);

  return result;
}

}
