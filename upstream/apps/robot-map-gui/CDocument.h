/* +---------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)               |
   |                          https://www.mrpt.org/                            |
   |                                                                           |
   | Copyright (c) 2005-2024, Individual contributors, see AUTHORS file        |
   | See: https://www.mrpt.org/Authors - All rights reserved.                  |
   | Released under BSD License. See details in https://www.mrpt.org/License   |
   +---------------------------------------------------------------------------+
   */
#pragma once
#include <string>

#include "TypeOfConfig.h"
#include "gui/configWidget/CConfigWidget.h"
#include "mrpt/config/CConfigFile.h"
#include "mrpt/maps/CMultiMetricMap.h"
#include "mrpt/maps/CSimpleMap.h"
#include "mrpt/opengl/CRenderizable.h"

/** This class gets *.simplemap and *.ini, and convert it to name of the map -
CSetOfObjects.
Also, it provides work with *.simplemap.
*/

using MetricPtr = mrpt::maps::CMetricMap::Ptr;
using RenderizableMaps = std::map<SType, mrpt::opengl::CSetOfObjects::Ptr>;
using TypeConfig = std::map<TypeOfConfig, std::vector<MetricPtr>>;

class CDocument
{
 public:
  CDocument() = default;
  ~CDocument() = default;

  bool isFileChanged() const;

  void loadSimpleMap(const std::string& fileName);
  void saveSimpleMap();

  void saveMetricMapRepresentationToFile(
      const std::string& fileName, const std::string& mapName) const;

  void saveMetricmapInBinaryFormat(const std::string& fileName, const std::string& mapName) const;

  void saveAsPng(const std::string& fileName) const;

  bool hasPointsMap() const;
  void saveAsText(const std::string& fileName) const;

  const std::string& getFileName() const;

  void setListOfMaps(mrpt::maps::TSetOfMetricMapInitializers& mapCfg);
  void setConfig(const std::string& config);

  const RenderizableMaps renderizableMaps() const;

  const mrpt::maps::CSimpleMap& simplemap() const;
  const mrpt::maps::CMultiMetricMap::TListMaps& config() const;

  const TypeConfig& typeConfig() const;

  std::vector<size_t> remove(const std::vector<size_t>& indexes);

  void move(
      const std::vector<size_t>& indexes,
      const mrpt::maps::CSimpleMap::KeyframeList& posesObsPairs);
  void move(
      size_t index,
      const mrpt::maps::CSimpleMap::Keyframe& posesObsPair,
      bool disableUpdateMetricMap = false);

  void insert(const std::vector<size_t>& idx, mrpt::maps::CSimpleMap::KeyframeList& posesObsPairs);

  mrpt::maps::CSimpleMap::KeyframeList get(const std::vector<size_t>& idx);

  const mrpt::maps::CSimpleMap::Keyframe& get(size_t idx) const { return m_simplemap.get(idx); }

  mrpt::maps::CSimpleMap::Keyframe& get(size_t idx) { return m_simplemap.get(idx); }

  mrpt::maps::CSimpleMap::KeyframeList getReverse(const std::vector<size_t>& idx);

 private:
  void addMapToRenderizableMaps(TypeOfConfig type, RenderizableMaps& renderMaps) const;
  void updateMetricMap();

  mrpt::maps::CSimpleMap m_simplemap;
  mrpt::maps::CMultiMetricMap m_metricmap;
  TypeConfig m_typeConfigs;
  std::string m_fileName;
  bool m_changedFile{false};
  bool m_hasPointsMap{false};
};
