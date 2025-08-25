#include "tp_boj/ReadBOJ.h"

#include "tp_math_utils/Geometry3D.h"
#include "tp_math_utils/materials/OpenGLMaterial.h"
#include "tp_math_utils/materials/LegacyMaterial.h"

#include "tp_utils/FileUtils.h"
#include "tp_utils/DebugUtils.h"
#include "tp_utils/JSONUtils.h"

#include <stdexcept>

namespace tp_boj
{

//##################################################################################################
std::vector<tp_math_utils::Geometry3D> readObjectAndTexturesFromFile(const std::string& filePath,
                                                                     std::unordered_map<tp_utils::StringID, std::string>& texturePaths,
                                                                     const tp_math_utils::ExtractTextureIDs& extractTextureIDs)
{
  std::string directory = getAssociatedFilePath(filePath);

  std::vector<tp_math_utils::Geometry3D> geometry = deserializeObject(tp_utils::readBinaryFile(filePath));


  std::unordered_set<tp_utils::StringID> textures;
  for(const auto& mesh : geometry)
    mesh.material.allTextureIDs(textures, extractTextureIDs);

  for(const auto& name : textures)
    texturePaths[name] = directory + name.toString() + ".png";

  return geometry;
}

//##################################################################################################
std::vector<tp_math_utils::Geometry3D> deserializeObject(const std::string& data)
{
  uint32_t maxVersion=20;

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

      if(version<20)
      {
        auto openGLMaterial = mesh.material.findOrAddOpenGL();
        auto legacyMaterial = mesh.material.findOrAddLegacy();

        if(version>16)
        {
          legacyMaterial->shaderType = tp_math_utils::ShaderType(readInt());
        }

        openGLMaterial->albedo.x = readFloat();
        openGLMaterial->albedo.y = readFloat();
        openGLMaterial->albedo.z = readFloat();

        if(version<3)
        {
          openGLMaterial->albedo.x = readFloat();
          openGLMaterial->albedo.y = readFloat();
          openGLMaterial->albedo.z = readFloat();
        }

        if(version<6)
        {
          readFloat(); // specular
          readFloat(); // specular
          readFloat(); // specular
        }

        if(version<3)
          readFloat();

        openGLMaterial->alpha = readFloat();

        if(version>2)
        {
          openGLMaterial->roughness       = readFloat();
          openGLMaterial->metalness       = readFloat();

          if(version>4)
          {
            openGLMaterial->transmission  = readFloat();
            if(version>7)
              openGLMaterial->transmissionRoughness  = readFloat();

            legacyMaterial->ior           = readFloat();

            if(version>6)
            {
              legacyMaterial->sheen              = readFloat();
              legacyMaterial->sheenTint          = readFloat();
              legacyMaterial->clearCoat          = readFloat();
              legacyMaterial->clearCoatRoughness = readFloat();

              if(version>9)
              {
                legacyMaterial->   iridescentFactor = readFloat();
                legacyMaterial->   iridescentOffset = readFloat();
                legacyMaterial->iridescentFrequency = readFloat();

                if(version>10)
                {
                  legacyMaterial->  specular        = readFloat();
                }
              }
            }

            legacyMaterial->sssScale      = readFloat();

            legacyMaterial->sssRadius.x   = readFloat();
            legacyMaterial->sssRadius.y   = readFloat();
            legacyMaterial->sssRadius.z   = readFloat();

            if(version>11)
            {
              if(version>15)
              {
                legacyMaterial->sssMethod = tp_math_utils::SSSMethod(readInt());
                legacyMaterial->normalStrength = readFloat();
              }

              openGLMaterial->albedoBrightness = readFloat();
              openGLMaterial->albedoContrast   = readFloat();
              openGLMaterial->albedoGamma      = readFloat();
              openGLMaterial->albedoHue        = readFloat();
              openGLMaterial->albedoSaturation = readFloat();
              openGLMaterial->albedoValue      = readFloat();
              openGLMaterial->albedoFactor     = readFloat();
            }

            legacyMaterial->sss.x         = readFloat();
            legacyMaterial->sss.y         = readFloat();
            legacyMaterial->sss.z         = readFloat();

            legacyMaterial->emission.x    = readFloat();
            legacyMaterial->emission.y    = readFloat();
            legacyMaterial->emission.z    = readFloat();

            legacyMaterial->emissionScale = readFloat();

            if(version>6)
            {
              legacyMaterial->velvet.x    = readFloat();
              legacyMaterial->velvet.y    = readFloat();
              legacyMaterial->velvet.z    = readFloat();

              legacyMaterial->velvetScale = readFloat();
            }

            if(version>5)
            {
              legacyMaterial->heightScale    = readFloat();
              legacyMaterial->heightMidlevel = readFloat();
            }
          }

          openGLMaterial->useAmbient     = readFloat();
          openGLMaterial->useDiffuse     = readFloat();
          openGLMaterial->useNdotL       = readFloat();
          openGLMaterial->useAttenuation = readFloat();
          openGLMaterial->useShadow      = readFloat();
          openGLMaterial->useLightMask   = readFloat();
          openGLMaterial->useReflection  = readFloat();
        }


        if(version>0)
        {
          if(version<3)
            readFloat();

          openGLMaterial->albedoScale   = readFloat();
          if(version<6)
            readFloat(); //specularScale
        }

        if(version>1)
        {
          openGLMaterial->tileTextures = readInt();

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
              legacyMaterial->rayVisibilityCamera       = readInt();
              legacyMaterial->rayVisibilityDiffuse      = readInt();
              legacyMaterial->rayVisibilityGlossy       = readInt();
              legacyMaterial->rayVisibilityTransmission = readInt();
              legacyMaterial->rayVisibilityScatter      = readInt();
              legacyMaterial->rayVisibilityShadow       = readInt();

              if(version>14)
                openGLMaterial->rayVisibilityShadowCatcher = readInt();
            }
          }
        }

        if(version<3)
          readString();

        openGLMaterial->albedoTexture  = readString();
        if(version<6)
          readString(); //specularTexture
        openGLMaterial->alphaTexture    = readString();
        openGLMaterial->normalsTexture  = readString();

        if(version>2)
        {
          openGLMaterial->roughnessTexture = readString();
          openGLMaterial->metalnessTexture = readString();
          if(version<6)
            readString(); //aoTexture
          else
          {
            legacyMaterial->emissionTexture = readString();
            legacyMaterial->     sssTexture = readString();
            legacyMaterial->  heightTexture = readString();
            if(version>6)
            {
              openGLMaterial->         transmissionTexture = readString();
              openGLMaterial->transmissionRoughnessTexture = readString();
              legacyMaterial->                sheenTexture = readString();
              legacyMaterial->            sheenTintTexture = readString();
              legacyMaterial->            clearCoatTexture = readString();
              legacyMaterial->   clearCoatRoughnessTexture = readString();
              legacyMaterial->               velvetTexture = readString();
              legacyMaterial->         velvetFactorTexture = readString();

              if(version>8)
              {
                legacyMaterial->           sssScaleTexture = readString();
                legacyMaterial->   iridescentFactorTexture = readString();
                legacyMaterial->   iridescentOffsetTexture = readString();
                legacyMaterial->iridescentFrequencyTexture = readString();

                if(version>10)
                {
                  legacyMaterial->         specularTexture = readString();

                  if(version>18)
                  {
                    openGLMaterial->           rgbaTexture = readString();
                    openGLMaterial->          rmttrTexture = readString();
                  }
                }
              }
            }
          }
        }
      }
      else
      {
        // Version 20+
        mesh.material.loadState(tp_utils::jsonFromString(readString()));

        mesh.material.uvTransformation.skewUV.x      = readFloat();
        mesh.material.uvTransformation.skewUV.y      = readFloat();
        mesh.material.uvTransformation.scaleUV.x     = readFloat();
        mesh.material.uvTransformation.scaleUV.y     = readFloat();
        mesh.material.uvTransformation.translateUV.x = readFloat();
        mesh.material.uvTransformation.translateUV.y = readFloat();
        mesh.material.uvTransformation.rotateUV      = readFloat();
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
