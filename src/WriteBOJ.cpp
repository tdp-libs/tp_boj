#include "tp_boj/WriteBOJ.h"

#include "tp_utils/FileUtils.h"

#include <cctype>

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

  return r;
}

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
    addInt(uint32_t(0)-3); // Version 3
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

        addFloat(vert.tangent.x);
        addFloat(vert.tangent.y);
        addFloat(vert.tangent.z);

        addFloat(vert.bitangent.x);
        addFloat(vert.bitangent.y);
        addFloat(vert.bitangent.z);
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

      addFloat(mesh.material.specular.x);
      addFloat(mesh.material.specular.y);
      addFloat(mesh.material.specular.z);

      addFloat(mesh.material.alpha);

      addFloat(mesh.material.roughness);
      addFloat(mesh.material.metalness);

      addFloat(mesh.material.useAmbient);
      addFloat(mesh.material.useDiffuse);
      addFloat(mesh.material.useNdotL);
      addFloat(mesh.material.useAttenuation);
      addFloat(mesh.material.useShadow);
      addFloat(mesh.material.useLightMask);
      addFloat(mesh.material.useReflection);

      addFloat(mesh.material.albedoScale);
      addFloat(mesh.material.specularScale);

      addInt(mesh.material.tileTextures?1:0);

      addString(cleanTextureName(mesh.material.albedoTexture   ));
      addString(cleanTextureName(mesh.material.specularTexture ));
      addString(cleanTextureName(mesh.material.alphaTexture    ));
      addString(cleanTextureName(mesh.material.normalsTexture  ));
      addString(cleanTextureName(mesh.material.roughnessTexture));
      addString(cleanTextureName(mesh.material.metalnessTexture));
      addString(cleanTextureName(mesh.material.aoTexture       ));
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
  {
    textures.insert(mesh.material. albedoTexture   );
    textures.insert(mesh.material. specularTexture );
    textures.insert(mesh.material. alphaTexture    );
    textures.insert(mesh.material. normalsTexture  );
    textures.insert(mesh.material. roughnessTexture);
    textures.insert(mesh.material. metalnessTexture);
    textures.insert(mesh.material. aoTexture       );
  }

  for(const auto& texture : textures)
    if(texture.isValid())
      saveTexture(texture);

  return result;
}

}
