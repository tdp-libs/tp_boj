#include "tp_boj/WriteBOJ.h"

#include "tp_utils/FileUtils.h"


namespace tp_boj
{

//##################################################################################################
std::string cleanTextureName(const tp_utils::StringID& name)
{
  std::string r = name.keyString();

  for(size_t i=0; i<r.size(); i++)
    r[i] = std::tolower(r[i]);

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
void writeObjectAndTexturesToFile(const std::vector<tp_maps::Geometry3D>& object,
                                  const std::string& filePath,
                                  const std::function<void(const tp_utils::StringID&, const std::string&)>& saveTexture)
{
  std::string directory = getAssociatedFilePath(filePath);

  std::string objectData = serializeObject(object, [&](const tp_utils::StringID& name)
  {
    saveTexture(name, directory + cleanTextureName(name) + ".png");
  });
  tp_utils::writeBinaryFile(filePath, objectData);
}

//##################################################################################################
std::string serializeObject(const std::vector<tp_maps::Geometry3D>& object,
                            const std::function<void(const tp_utils::StringID&)>& saveTexture)
{
  auto run = [&object](const auto& addInt, const auto& addFloat, const auto& addString)
  {
    addInt(object.size());
    for(const auto& mesh : object)
    {
      addInt(mesh.geometry.comments.size());
      for(const auto& comment : mesh.geometry.comments)
        addString(comment);

      addInt(mesh.geometry.verts.size());
      for(const auto& vert : mesh.geometry.verts)
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

      addInt(mesh.geometry.indexes.size());
      for(const auto& index : mesh.geometry.indexes)
      {
        if(index.type == mesh.geometry.triangleFan)
          addInt(1);
        else if(index.type == mesh.geometry.triangleStrip)
          addInt(2);
        else
          addInt(3);

        addInt(index.indexes.size());
        for(int i : index.indexes)
          addInt(uint32_t(i));
      }

      addString(mesh.material.name.keyString());

      addFloat(mesh.material.ambient.x);
      addFloat(mesh.material.ambient.y);
      addFloat(mesh.material.ambient.z);

      addFloat(mesh.material.diffuse.x);
      addFloat(mesh.material.diffuse.y);
      addFloat(mesh.material.diffuse.z);

      addFloat(mesh.material.specular.x);
      addFloat(mesh.material.specular.y);
      addFloat(mesh.material.specular.z);

      addFloat(mesh.material.shininess);
      addFloat(mesh.material.alpha);

      addString(cleanTextureName(mesh.material. ambientTexture));
      addString(cleanTextureName(mesh.material. diffuseTexture));
      addString(cleanTextureName(mesh.material.specularTexture));
      addString(cleanTextureName(mesh.material.   alphaTexture));
      addString(cleanTextureName(mesh.material.    bumpTexture));
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
      uint32_t n = s.size();
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
    textures.insert(mesh.material. ambientTexture); //!< mtl: map_Ka
    textures.insert(mesh.material. diffuseTexture); //!< mtl: map_Kd
    textures.insert(mesh.material.specularTexture); //!< mtl: map_Ks
    textures.insert(mesh.material.   alphaTexture); //!< mtl: map_d
    textures.insert(mesh.material.    bumpTexture); //!< mtl: map_Bump
  }

  for(const auto& texture : textures)
    saveTexture(texture);

  return result;
}

}
