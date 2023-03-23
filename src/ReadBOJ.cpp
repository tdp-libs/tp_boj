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
    for(const auto& name : mesh.material.allTextures())
      texturePaths[name] = directory + name.toString() + ".png";

  return geometry;
}

//##################################################################################################
std::vector<tp_math_utils::Geometry3D> deserializeObject(const std::string& data)
{
  uint32_t maxVersion=18;

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
    uint32_t version=0;

    for(uint32_t v=maxVersion; v; v--)
    {
      if(objCount == (uint32_t(0)-v))
      {
        version = v;
        objCount = readInt();
        break;
      }
    }

    if(version==0)
    {
      uint32_t fileVersion = uint32_t(0)-objCount;
      if(fileVersion<10000)
      {
        tpWarning() << "Failed to deserialize model, BOJ file version: " << fileVersion << " max supported version: " << maxVersion;
        return std::vector<tp_math_utils::Geometry3D>();
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

        if(version<18)
        {
          readFloat(); // vert.color.x
          readFloat(); // vert.color.y
          readFloat(); // vert.color.z
          readFloat(); // vert.color.w
        }

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

      if(version>16)
      {
        mesh.material.shaderType = tp_math_utils::ShaderType(readInt());
      }

      mesh.material.albedo.x = readFloat();
      mesh.material.albedo.y = readFloat();
      mesh.material.albedo.z = readFloat();

      if(version<3)
      {
        mesh.material.albedo.x = readFloat();
        mesh.material.albedo.y = readFloat();
        mesh.material.albedo.z = readFloat();
      }

      if(version<6)
      {
        readFloat(); // specular
        readFloat(); // specular
        readFloat(); // specular
      }

      if(version<3)
        readFloat();

      mesh.material.alpha = readFloat();

      if(version>2)
      {
        mesh.material.roughness       = readFloat();
        mesh.material.metalness       = readFloat();

        if(version>4)
        {
          mesh.material.transmission  = readFloat();          
          if(version>7)
            mesh.material.transmissionRoughness  = readFloat();

          mesh.material.ior           = readFloat();

          if(version>6)
          {
            mesh.material.sheen              = readFloat();
            mesh.material.sheenTint          = readFloat();
            mesh.material.clearCoat          = readFloat();
            mesh.material.clearCoatRoughness = readFloat();

            if(version>9)
            {
              mesh.material.   iridescentFactor = readFloat();
              mesh.material.   iridescentOffset = readFloat();
              mesh.material.iridescentFrequency = readFloat();

              if(version>10)
              {
                mesh.material.  specular        = readFloat();
              }
            }
          }

          mesh.material.sssScale      = readFloat();

          mesh.material.sssRadius.x   = readFloat();
          mesh.material.sssRadius.y   = readFloat();
          mesh.material.sssRadius.z   = readFloat();

          if(version>11)
          {
            if(version>15)
            {
              mesh.material.sssMethod = tp_math_utils::SSSMethod(readInt());
              mesh.material.normalStrength = readFloat();
            }

            mesh.material.albedoBrightness = readFloat();
            mesh.material.albedoContrast   = readFloat();
            mesh.material.albedoGamma      = readFloat();
            mesh.material.albedoHue        = readFloat();
            mesh.material.albedoSaturation = readFloat();
            mesh.material.albedoValue      = readFloat();
            mesh.material.albedoFactor     = readFloat();
          }

          mesh.material.sss.x         = readFloat();
          mesh.material.sss.y         = readFloat();
          mesh.material.sss.z         = readFloat();

          mesh.material.emission.x    = readFloat();
          mesh.material.emission.y    = readFloat();
          mesh.material.emission.z    = readFloat();

          mesh.material.emissionScale = readFloat();

          if(version>6)
          {
            mesh.material.velvet.x    = readFloat();
            mesh.material.velvet.y    = readFloat();
            mesh.material.velvet.z    = readFloat();

            mesh.material.velvetScale = readFloat();
          }

          if(version>5)
          {
            mesh.material.heightScale    = readFloat();
            mesh.material.heightMidlevel = readFloat();
          }
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
        if(version<6)
          readFloat(); //specularScale
      }

      if(version>1)
      {
        mesh.material.tileTextures = readInt();

        if(version>12)
        {
          mesh.material.uvTransformation.skewUV.x      = readFloat();
          mesh.material.uvTransformation.skewUV.y      = readFloat();
          mesh.material.uvTransformation.scaleUV.x     = readFloat();
          mesh.material.uvTransformation.scaleUV.y     = readFloat();
          mesh.material.uvTransformation.translateUV.x = readFloat();
          mesh.material.uvTransformation.translateUV.y = readFloat();
          mesh.material.uvTransformation.rotateUV      = readFloat();

          if(version>13)
          {
            mesh.material.rayVisibilityCamera       = readInt();
            mesh.material.rayVisibilityDiffuse      = readInt();
            mesh.material.rayVisibilityGlossy       = readInt();
            mesh.material.rayVisibilityTransmission = readInt();
            mesh.material.rayVisibilityScatter      = readInt();
            mesh.material.rayVisibilityShadow       = readInt();

            if(version>14)
              mesh.material.rayVisibilityShadowCatcher = readInt();
          }
        }
      }

      if(version<3)
        readString();

      mesh.material.albedoTexture  = readString();
      if(version<6)
        readString(); //specularTexture
      mesh.material.alphaTexture    = readString();
      mesh.material.normalsTexture  = readString();

      if(version>2)
      {
        mesh.material.roughnessTexture = readString();
        mesh.material.metalnessTexture = readString();
        if(version<6)
          readString(); //aoTexture
        else
        {
          mesh.material.emissionTexture = readString();
          mesh.material.     sssTexture = readString();
          mesh.material.  heightTexture = readString();
          if(version>6)
          {
            mesh.material.         transmissionTexture = readString();
            mesh.material.transmissionRoughnessTexture = readString();
            mesh.material.                sheenTexture = readString();
            mesh.material.            sheenTintTexture = readString();
            mesh.material.            clearCoatTexture = readString();
            mesh.material.   clearCoatRoughnessTexture = readString();
            mesh.material.               velvetTexture = readString();
            mesh.material.         velvetFactorTexture = readString();

            if(version>8)
            {
              mesh.material.           sssScaleTexture = readString();
              mesh.material.   iridescentFactorTexture = readString();
              mesh.material.   iridescentOffsetTexture = readString();
              mesh.material.iridescentFrequencyTexture = readString();

              if(version>10)
              {
                mesh.material.         specularTexture = readString();
              }
            }
          }
        }
      }
    }

    return object;
  }
  catch(...)
  {
    return std::vector<tp_math_utils::Geometry3D>();
  }
}

}
