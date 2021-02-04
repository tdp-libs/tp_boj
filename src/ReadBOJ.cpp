#include "tp_boj/ReadBOJ.h"

#include "tp_math_utils/Geometry3D.h"

#include "tp_utils/FileUtils.h"
#include "tp_utils/DebugUtils.h"

#include <stdexcept>

namespace tp_boj
{

//##################################################################################################
std::vector<tp_math_utils::Geometry3D> readObjectAndTexturesFromFile(const std::string& filePath,
                                                                     std::unordered_map<tp_utils::StringID, std::string>& texturePaths)
{
  std::string directory = getAssociatedFilePath(filePath);

  std::vector<tp_math_utils::Geometry3D> geometry = deserializeObject(tp_utils::readBinaryFile(filePath));
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

    addTexture(mesh.material.   albedoTexture);  //!< mtl: map_Kd or map_Ka
    addTexture(mesh.material. specularTexture);  //!< mtl: map_Ks
    addTexture(mesh.material.    alphaTexture);  //!< mtl: map_d
    addTexture(mesh.material.  normalsTexture);  //!< mtl: map_Bump
    addTexture(mesh.material.roughnessTexture);  //!<
    addTexture(mesh.material.metalnessTexture);  //!<
    addTexture(mesh.material.       aoTexture);  //!<
  }

  return geometry;
}

//##################################################################################################
std::vector<tp_math_utils::Geometry3D> deserializeObject(const std::string& data)
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

    for(uint32_t v=6; v; v--)
    {
      if(objCount == (uint32_t(0)-v))
      {
        version = v;
        objCount = readInt();
        break;
      }
    }

    std::vector<tp_math_utils::Geometry3D> object;
    object.resize(size_t(objCount));
    for(auto& mesh : object)
    {
      mesh.comments.resize(size_t(readInt()));
      for(auto& comment : mesh.comments)
        comment = readString();

      mesh.verts.resize(size_t(readInt()));
      for(auto& vert : mesh.verts)
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

        if(version<4)
        {
          readFloat();
          readFloat();
          readFloat();

          readFloat();
          readFloat();
          readFloat();
        }
      }

      mesh.indexes.resize(size_t(readInt()));
      for(auto& index : mesh.indexes)
      {
        switch(readInt())
        {
        case 1:  index.type = mesh.triangleFan;   break;
        case 2:  index.type = mesh.triangleStrip; break;
        default: index.type = mesh.triangles;     break;
        }

        index.indexes.resize(size_t(readInt()));
        for(int& i : index.indexes)
          i = int(readInt());
      }

      mesh.material.name = readString();

      mesh.material.albedo.x = readFloat();
      mesh.material.albedo.y = readFloat();
      mesh.material.albedo.z = readFloat();

      if(version<3)
      {
        mesh.material.albedo.x = readFloat();
        mesh.material.albedo.y = readFloat();
        mesh.material.albedo.z = readFloat();
      }

      mesh.material.specular.x = readFloat();
      mesh.material.specular.y = readFloat();
      mesh.material.specular.z = readFloat();

      if(version<3)
        readFloat();

      mesh.material.alpha = readFloat();

      if(version>2)
      {
        mesh.material.roughness      = readFloat();
        mesh.material.metalness      = readFloat();

        if(version>4)
        {
          mesh.material.transmission = readFloat();
          mesh.material.ior          = readFloat();
        }

        mesh.material.useAmbient     = readFloat();
        mesh.material.useDiffuse     = readFloat();
        mesh.material.useNdotL       = readFloat();
        mesh.material.useAttenuation = readFloat();
        mesh.material.useShadow      = readFloat();
        mesh.material.useLightMask   = readFloat();
        mesh.material.useReflection  = readFloat();
      }


      if(version>0)
      {
        if(version<3)
          readFloat();

        mesh.material.albedoScale   = readFloat();
        mesh.material.specularScale = readFloat();
      }

      if(version>1)
      {
        mesh.material.tileTextures = readInt();
      }


      if(version<3)
        readString();

      mesh.material.albedoTexture  = readString();
      mesh.material.specularTexture = readString();
      mesh.material.alphaTexture    = readString();
      mesh.material.normalsTexture  = readString();

      if(version>2)
      {
        mesh.material.roughnessTexture = readString();
        mesh.material.metalnessTexture = readString();
        mesh.material.aoTexture        = readString();
      }
    }

    return object;
  }
  catch(...)
  {
    tpWarning() << "Failed to deserialize model.";
    return std::vector<tp_math_utils::Geometry3D>();
  }
}

}
