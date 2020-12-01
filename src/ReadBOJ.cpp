#include "tp_boj/ReadBOJ.h"

#include "tp_math_utils/Geometry3D.h"

#include "tp_utils/FileUtils.h"
#include "tp_utils/DebugUtils.h"

#include <stdexcept>

namespace tp_boj
{

//##################################################################################################
std::vector<tp_maps::Geometry3D> readObjectAndTexturesFromFile(const std::string& filePath,
                                                               std::unordered_map<tp_utils::StringID, std::string>& texturePaths,
                                                               int triangleFan,
                                                               int triangleStrip,
                                                               int triangles)
{
  std::string directory = getAssociatedFilePath(filePath);

  std::vector<tp_maps::Geometry3D> geometry = deserializeObject(tp_utils::readBinaryFile(filePath),
                                                                triangleFan,
                                                                triangleStrip,
                                                                triangles);
  for(const auto& mesh : geometry)
  {
    auto addTexture = [&](const tp_utils::StringID& name)
    {
      if(!name.isValid())
        return;

      if(tpContainsKey(texturePaths, name))
        return;

      texturePaths[name] = directory + name.keyString() + ".png";
    };

    addTexture(mesh.material. ambientTexture); //!< mtl: map_Ka
    addTexture(mesh.material. diffuseTexture); //!< mtl: map_Kd
    addTexture(mesh.material.specularTexture); //!< mtl: map_Ks
    addTexture(mesh.material.   alphaTexture); //!< mtl: map_d
    addTexture(mesh.material.    bumpTexture); //!< mtl: map_Bump
  }

  return geometry;
}

//##################################################################################################
std::vector<tp_maps::Geometry3D> deserializeObject(const std::string& data,
                                                   int triangleFan,
                                                   int triangleStrip,
                                                   int triangles)
{
  auto p = data.data();
  auto pMax = p + data.size();

  auto readInt = [&]()
  {
    if((pMax-p) < 4)
      throw std::logic_error("BOJ readInt buffer overflow.");

    uint32_t n;
    memcpy(&n, p, 4);
    p+=4;
    return n;
  };

  auto readFloat = [&]()
  {
    if((pMax-p) < 4)
      throw std::logic_error("BOJ readFloat buffer overflow.");

    float n;
    memcpy(&n, p, 4);
    p+=4;
    return n;
  };

  auto readString = [&]()
  {
    if((pMax-p) < 4)
      throw std::logic_error("BOJ readString buffer overflow.");

    uint32_t n;
    memcpy(&n, p, 4);
    p+=4;

    std::string str;
    str.resize(n);

    if((pMax-p) < n)
      throw std::logic_error("BOJ readString buffer overflow.");

    memcpy(str.data(), p, n);
    p+=n;

    return str;
  };

  try
  {
    uint32_t objCount = readInt();
    int version=0;
    if(objCount == (uint32_t(0)-1))
    {
      version = 1;
      objCount = readInt();
    }
    if(objCount == (uint32_t(0)-2))
    {
      version = 2;
      objCount = readInt();
    }
    if(objCount == (uint32_t(0)-3))
    {
      version = 3;
      objCount = readInt();
    }

    std::vector<tp_maps::Geometry3D> object;
    object.resize(size_t(objCount));
    for(auto& mesh : object)
    {
      mesh.geometry.triangleFan   = triangleFan;
      mesh.geometry.triangleStrip = triangleStrip;
      mesh.geometry.triangles     = triangles;
      mesh.geometry.comments.resize(size_t(readInt()));
      for(auto& comment : mesh.geometry.comments)
        comment = readString();

      mesh.geometry.verts.resize(size_t(readInt()));
      for(auto& vert : mesh.geometry.verts)
      {
        vert.vert.x = readFloat();
        vert.vert.y = readFloat();
        vert.vert.z = readFloat();

        vert.color.x = readFloat();
        vert.color.y = readFloat();
        vert.color.z = readFloat();
        vert.color.w = readFloat();

        vert.texture.x = readFloat();
        vert.texture.y = readFloat();

        vert.normal.x = readFloat();
        vert.normal.y = readFloat();
        vert.normal.z = readFloat();

        vert.tangent.x = readFloat();
        vert.tangent.y = readFloat();
        vert.tangent.z = readFloat();

        vert.bitangent.x = readFloat();
        vert.bitangent.y = readFloat();
        vert.bitangent.z = readFloat();
      }

      mesh.geometry.indexes.resize(size_t(readInt()));
      for(auto& index : mesh.geometry.indexes)
      {
        switch(readInt())
        {
        case 1:  index.type = triangleFan;   break;
        case 2:  index.type = triangleStrip; break;
        default: index.type = triangles;     break;
        }

        index.indexes.resize(size_t(readInt()));
        for(int& i : index.indexes)
          i = int(readInt());
      }

      mesh.material.name = readString();

      mesh.material.ambient.x = readFloat();
      mesh.material.ambient.y = readFloat();
      mesh.material.ambient.z = readFloat();

      mesh.material.diffuse.x = readFloat();
      mesh.material.diffuse.y = readFloat();
      mesh.material.diffuse.z = readFloat();

      mesh.material.specular.x = readFloat();
      mesh.material.specular.y = readFloat();
      mesh.material.specular.z = readFloat();

      mesh.material.shininess = readFloat();
      mesh.material.alpha = readFloat();

      if(version>2)
      {
        mesh.material.roughness      = readFloat();
        mesh.material.metalness      = readFloat();

        mesh.material.useDiffuse     = readFloat();
        mesh.material.useNdotL       = readFloat();
        mesh.material.useAttenuation = readFloat();
        mesh.material.useShadow      = readFloat();
        mesh.material.useLightMask   = readFloat();
        mesh.material.useReflection  = readFloat();
      }

      if(version>0)
      {
        mesh.material.ambientScale = readFloat();
        mesh.material.diffuseScale = readFloat();
        mesh.material.specularScale = readFloat();
      }

      if(version>1)
      {
        mesh.material.tileTextures = readInt();
      }

      mesh.material.ambientTexture  = readString();
      mesh.material.diffuseTexture  = readString();
      mesh.material.specularTexture = readString();
      mesh.material.alphaTexture    = readString();
      mesh.material.bumpTexture     = readString();
    }

    return object;
  }
  catch(...)
  {
    tpWarning() << "Failed to deserialize model.";
    return std::vector<tp_maps::Geometry3D>();
  }
}

}
