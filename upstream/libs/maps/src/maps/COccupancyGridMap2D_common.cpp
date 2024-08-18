/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2024, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

#include "maps-precomp.h"  // Precomp header
//

// Force size_x being a multiple of 16 cells
//#define		ROWSIZE_MULTIPLE_16

#include <mrpt/maps/COccupancyGridMap2D.h>
#include <mrpt/maps/CSimplePointsMap.h>
#include <mrpt/math/TPose2D.h>
#include <mrpt/poses/CPose3D.h>
#include <mrpt/serialization/CArchive.h>

using namespace mrpt;
using namespace mrpt::math;
using namespace mrpt::maps;
using namespace mrpt::obs;
using namespace mrpt::poses;
using namespace mrpt::tfest;
using namespace std;

//  =========== Begin of Map definition ============
MAP_DEFINITION_REGISTER(
    "mrpt::maps::COccupancyGridMap2D,occupancyGrid", mrpt::maps::COccupancyGridMap2D)

COccupancyGridMap2D::TMapDefinition::TMapDefinition() = default;

void COccupancyGridMap2D::TMapDefinition::loadFromConfigFile_map_specific(
    const mrpt::config::CConfigFileBase& source, const std::string& sectionNamePrefix)
{
  // [<sectionNamePrefix>+"_creationOpts"]
  const std::string sSectCreation = sectionNamePrefix + string("_creationOpts");
  MRPT_LOAD_CONFIG_VAR(min_x, float, source, sSectCreation);
  MRPT_LOAD_CONFIG_VAR(max_x, float, source, sSectCreation);
  MRPT_LOAD_CONFIG_VAR(min_y, float, source, sSectCreation);
  MRPT_LOAD_CONFIG_VAR(max_y, float, source, sSectCreation);
  MRPT_LOAD_CONFIG_VAR(resolution, float, source, sSectCreation);

  // [<sectionName>+"_occupancyGrid_##_insertOpts"]
  insertionOpts.loadFromConfigFile(source, sectionNamePrefix + string("_insertOpts"));

  // [<sectionName>+"_occupancyGrid_##_likelihoodOpts"]
  likelihoodOpts.loadFromConfigFile(source, sectionNamePrefix + string("_likelihoodOpts"));
}

void COccupancyGridMap2D::TMapDefinition::dumpToTextStream_map_specific(std::ostream& out) const
{
  LOADABLEOPTS_DUMP_VAR(min_x, float);
  LOADABLEOPTS_DUMP_VAR(max_x, float);
  LOADABLEOPTS_DUMP_VAR(min_y, float);
  LOADABLEOPTS_DUMP_VAR(max_y, float);
  LOADABLEOPTS_DUMP_VAR(resolution, float);

  this->insertionOpts.dumpToTextStream(out);
  this->likelihoodOpts.dumpToTextStream(out);
}

mrpt::maps::CMetricMap::Ptr COccupancyGridMap2D::internal_CreateFromMapDefinition(
    const mrpt::maps::TMetricMapInitializer& _def)
{
  const COccupancyGridMap2D::TMapDefinition& def =
      *dynamic_cast<const COccupancyGridMap2D::TMapDefinition*>(&_def);
  auto obj =
      COccupancyGridMap2D::Create(def.min_x, def.max_x, def.min_y, def.max_y, def.resolution);
  obj->insertionOptions = def.insertionOpts;
  obj->likelihoodOptions = def.likelihoodOpts;
  return obj;
}
//  =========== End of Map definition Block =========

IMPLEMENTS_SERIALIZABLE(COccupancyGridMap2D, CMetricMap, mrpt::maps)

std::vector<float> COccupancyGridMap2D::entropyTable;

static const float MAX_H = 0.69314718055994531f;  // ln(2)

// Static lookup tables for log-odds
static CLogOddsGridMapLUT<COccupancyGridMap2D::cellType> logodd_lut;
CLogOddsGridMapLUT<COccupancyGridMap2D::cellType>& COccupancyGridMap2D::get_logodd_lut()
{
  return logodd_lut;
}

/*---------------------------------------------------------------
            Constructor
  ---------------------------------------------------------------*/
COccupancyGridMap2D::COccupancyGridMap2D(
    float min_x, float max_x, float min_y, float max_y, float res) :
    m_map(),

    m_precomputedLikelihood(),

    m_basis_map(),
    m_voronoi_diagram(),

    updateInfoChangeOnly(),
    insertionOptions(),
    likelihoodOptions(),
    likelihoodOutputs(),
    CriticalPointsList()
{
  MRPT_START
  setSize(min_x, max_x, min_y, max_y, res, 0.5f);
  MRPT_END
}

void COccupancyGridMap2D::copyMapContentFrom(const COccupancyGridMap2D& o)
{
  freeMap();
  m_resolution = o.m_resolution;
  m_xMin = o.m_xMin;
  m_xMax = o.m_xMax;
  m_yMin = o.m_yMin;
  m_yMax = o.m_yMax;
  m_size_x = o.m_size_x;
  m_size_y = o.m_size_y;
  m_map = o.m_map;

  m_basis_map.clear();
  m_voronoi_diagram.clear();

  m_likelihoodCacheOutDated = true;
  m_is_empty = o.m_is_empty;
}

void COccupancyGridMap2D::setSize(
    float xmin, float xmax, float ymin, float ymax, float res, float default_value)
{
  MRPT_START

  ASSERT_GT_(res, 0.0f);
  ASSERT_GT_(xmax, xmin);
  ASSERT_GT_(ymax, ymin);
  ASSERT_GE_(default_value, 0.0f);
  ASSERT_LE_(default_value, 1.0f);

  freeMap();
  m_likelihoodCacheOutDated = true;

  // Adjust sizes to adapt them to full sized cells acording to the
  // resolution:
  xmin = res * round(xmin / res);
  ymin = res * round(ymin / res);
  xmax = res * round(xmax / res);
  ymax = res * round(ymax / res);

  // Set parameters:
  this->m_resolution = res;
  this->m_xMin = xmin;
  this->m_xMax = xmax;
  this->m_yMin = ymin;
  this->m_yMax = ymax;

  // Now the number of cells should be integers:
  m_size_x = round((m_xMax - m_xMin) / m_resolution);
  m_size_y = round((m_yMax - m_yMin) / m_resolution);

#ifdef ROWSIZE_MULTIPLE_16
  // map rows must be 16 bytes aligned:
  if (0 != (size_x % 16))
  {
    size_x = ((size_x >> 4) + 1) << 4;
    x_max = x_min + size_x * resolution;
  }
  size_x = round((x_max - x_min) / resolution);
  ASSERT_(0 == (size_x % 16));
#endif

  // Cells memory:
  m_map.resize(m_size_x * m_size_y, p2l(default_value));

  // Free these buffers also:
  m_basis_map.clear();
  m_voronoi_diagram.clear();

  m_is_empty = true;

  MRPT_END
}

void COccupancyGridMap2D::resizeGrid(
    float new_x_min,
    float new_x_max,
    float new_y_min,
    float new_y_max,
    float new_cells_default_value,
    bool additionalMargin) noexcept
{
  unsigned int extra_x_izq = 0, extra_y_arr = 0, new_size_x = 0, new_size_y = 0;
  std::vector<cellType> new_map;

  if (new_x_min > new_x_max)
  {
    printf(
        "[COccupancyGridMap2D::resizeGrid] Warning!! Ignoring call, since: "
        "x_min=%f  x_max=%f\n",
        new_x_min, new_x_max);
    return;
  }
  if (new_y_min > new_y_max)
  {
    printf(
        "[COccupancyGridMap2D::resizeGrid] Warning!! Ignoring call, since: "
        "y_min=%f  y_max=%f\n",
        new_y_min, new_y_max);
    return;
  }

  // Required?
  if (new_x_min >= m_xMin && new_y_min >= m_yMin && new_x_max <= m_xMax && new_y_max <= m_yMax)
    return;

  // For the precomputed likelihood trick:
  m_likelihoodCacheOutDated = true;

  // Add an additional margin:
  if (additionalMargin)
  {
    if (new_x_min < m_xMin) new_x_min = floor(new_x_min - 4);
    if (new_x_max > m_xMax) new_x_max = ceil(new_x_max + 4);
    if (new_y_min < m_yMin) new_y_min = floor(new_y_min - 4);
    if (new_y_max > m_yMax) new_y_max = ceil(new_y_max + 4);
  }

  // We do not support grid shrinking... at least stay the same:
  new_x_min = min(new_x_min, m_xMin);
  new_x_max = max(new_x_max, m_xMax);
  new_y_min = min(new_y_min, m_yMin);
  new_y_max = max(new_y_max, m_yMax);

  // Adjust sizes to adapt them to full sized cells acording to the
  // resolution:
  if (fabs(new_x_min / m_resolution - round(new_x_min / m_resolution)) > 0.05f)
    new_x_min = m_resolution * round(new_x_min / m_resolution);
  if (fabs(new_y_min / m_resolution - round(new_y_min / m_resolution)) > 0.05f)
    new_y_min = m_resolution * round(new_y_min / m_resolution);
  if (fabs(new_x_max / m_resolution - round(new_x_max / m_resolution)) > 0.05f)
    new_x_max = m_resolution * round(new_x_max / m_resolution);
  if (fabs(new_y_max / m_resolution - round(new_y_max / m_resolution)) > 0.05f)
    new_y_max = m_resolution * round(new_y_max / m_resolution);

  // Change size: 4 sides extensions:
  extra_x_izq = round((m_xMin - new_x_min) / m_resolution);
  extra_y_arr = round((m_yMin - new_y_min) / m_resolution);

  new_size_x = round((new_x_max - new_x_min) / m_resolution);
  new_size_y = round((new_y_max - new_y_min) / m_resolution);

  assert(new_size_x >= m_size_x + extra_x_izq);

#ifdef ROWSIZE_MULTIPLE_16
  // map rows must be 16 bytes aligned:
  size_t old_new_size_x = new_size_x;  // Debug
  if (0 != (new_size_x % 16))
  {
    int size_x_incr = 16 - (new_size_x % 16);
    // new_x_max = new_x_min + new_size_x * resolution;
    new_x_max += size_x_incr * resolution;
  }
  new_size_x = round((new_x_max - new_x_min) / resolution);
  assert(0 == (new_size_x % 16));
#endif

  // Reserve new mem block
  new_map.resize(new_size_x * new_size_y, p2l(new_cells_default_value));

  // Copy all the old map rows into the new map:
  {
    cellType* dest_ptr = &new_map[extra_x_izq + extra_y_arr * new_size_x];
    cellType* src_ptr = &m_map[0];
    size_t row_size = m_size_x * sizeof(cellType);

    for (size_t y = 0; y < m_size_y; y++)
    {
#if defined(_DEBUG) || (MRPT_ALWAYS_CHECKS_DEBUG)
      assert(dest_ptr + row_size - 1 <= &new_map[new_map.size() - 1]);
      assert(src_ptr + row_size - 1 <= &m_map[m_map.size() - 1]);
#endif
      memcpy(dest_ptr, src_ptr, row_size);
      dest_ptr += new_size_x;
      src_ptr += m_size_x;
    }
  }

  // Move new values into the new map:
  m_xMin = new_x_min;
  m_xMax = new_x_max;
  m_yMin = new_y_min;
  m_yMax = new_y_max;

  m_size_x = new_size_x;
  m_size_y = new_size_y;

  // Free old map, replace by new one:
  m_map.swap(new_map);

  // Free the other buffers:
  m_basis_map.clear();
  m_voronoi_diagram.clear();
}

/*---------------------------------------------------------------
            freeMap
  ---------------------------------------------------------------*/
void COccupancyGridMap2D::freeMap()
{
  MRPT_START

  // Free map and sectors
  m_map.clear();

  m_basis_map.clear();
  m_voronoi_diagram.clear();

  m_size_x = m_size_y = 0;

  // For the precomputed likelihood trick:
  m_likelihoodCacheOutDated = true;

  m_is_empty = true;

  MRPT_END
}

/*---------------------------------------------------------------
  Computes the entropy and related values of this grid map.
  out_H The target variable for absolute entropy, computed
 as:<br><center>H(map)=Sum<sub>x,y</sub>{ -p(x,y)ln(p(x,y))
 -(1-p(x,y))ln(1-p(x,y)) }</center><br><br>
  out_I The target variable for absolute "information", defining I(x) = 1 -
 H(x)
  out_mean_H The target variable for mean entropy, defined as entropy per
 square meter: mean_H(map) = H(map) / (Map length x (meters))(Map length y
 (meters))
  out_mean_I The target variable for mean information, defined as information
 per square meter: mean_I(map) = I(map) / (Map length x (meters))(Map length y
 (meters))
 ---------------------------------------------------------------*/
void COccupancyGridMap2D::computeEntropy(TEntropyInfo& info) const
{
#ifdef OCCUPANCY_GRIDMAP_CELL_SIZE_8BITS
  size_t N = 256;
#else
  size_t N = 65536;
#endif

  // Compute the entropy table: The entropy for each posible cell value
  // ----------------------------------------------------------------------
  if (entropyTable.size() != N)
  {
    entropyTable.resize(N, 0);
    for (size_t i = 0; i < N; i++)
    {
      const auto p = l2p(static_cast<cellType>(i));
      auto h = d2f(H(p) + H(1 - p));

      // Cell's probabilities rounding problem fixing:
      if (i == 0 || i == (N - 1)) h = 0;
      if (h > (MAX_H - 1e-4)) h = MAX_H;

      entropyTable[i] = h;
    }
  }

  // Initialize the global results:
  info.H = info.I = 0;
  info.effectiveMappedCells = 0;

  info.H = info.I = 0;
  info.effectiveMappedCells = 0;
  for (signed char it : m_map)
  {
    auto ctu = static_cast<cellTypeUnsigned>(it);
    auto h = entropyTable[ctu];
    info.H += h;
    if (h < (MAX_H - 0.001f))
    {
      info.effectiveMappedCells++;
      info.I -= h;
    }
  }

  // The info: (See ref. paper EMMI in IROS 2006)
  info.I /= MAX_H;
  info.I += info.effectiveMappedCells;

  // Mean values:
  // ------------------------------------------
  info.effectiveMappedArea = info.effectiveMappedCells * m_resolution * m_resolution;
  if (info.effectiveMappedCells)
  {
    info.mean_H = info.H / info.effectiveMappedCells;
    info.mean_I = info.I / info.effectiveMappedCells;
  }
  else
  {
    info.mean_H = 0;
    info.mean_I = 0;
  }
}

/*---------------------------------------------------------------
              clear
 ---------------------------------------------------------------*/
void COccupancyGridMap2D::internal_clear()
{
  setSize(-10, 10, -10, 10, getResolution());
  // For the precomputed likelihood trick:
  m_likelihoodCacheOutDated = true;
}

/*---------------------------------------------------------------
              fill
 ---------------------------------------------------------------*/
void COccupancyGridMap2D::fill(float default_value)
{
  cellType defValue = p2l(default_value);
  for (auto it = m_map.begin(); it < m_map.end(); ++it) *it = defValue;
  // For the precomputed likelihood trick:
  m_likelihoodCacheOutDated = true;
}

/*---------------------------------------------------------------
          updateCell
 ---------------------------------------------------------------*/
void COccupancyGridMap2D::updateCell(int x, int y, float v)
{
  // Tip: if x<0, (unsigned)(x) will also be >>> size_x ;-)
  if (static_cast<unsigned int>(x) >= m_size_x || static_cast<unsigned int>(y) >= m_size_y) return;

  // Get the current contents of the cell:
  cellType& theCell = m_map[x + y * m_size_x];

  // Compute the new Bayesian-fused value of the cell:
  if (updateInfoChangeOnly.enabled)
  {
    float old = l2p(theCell);
    float new_v = 1 / (1 + (1 - v) * (1 - old) / (old * v));
    updateInfoChangeOnly.cellsUpdated++;
    updateInfoChangeOnly.I_change += 1 - (H(new_v) + H(1 - new_v)) / MAX_H;
  }
  else
  {
    cellType obs = p2l(v);  // The observation: will be >0 for free, <0 for occupied.
    if (obs > 0)
    {
      if (theCell > (OCCGRID_CELLTYPE_MAX - obs))
        theCell = OCCGRID_CELLTYPE_MAX;  // Saturate
      else
        theCell += obs;
    }
    else
    {
      if (theCell < (OCCGRID_CELLTYPE_MIN - obs))
        theCell = OCCGRID_CELLTYPE_MIN;  // Saturate
      else
        theCell += obs;
    }
  }
}

/*---------------------------------------------------------------
              subSample
 ---------------------------------------------------------------*/
void COccupancyGridMap2D::subSample(int downRatio)
{
  std::vector<cellType> newMap;

  ASSERT_(downRatio > 0);

  m_resolution *= downRatio;

  int newSizeX = round((m_xMax - m_xMin) / m_resolution);
  int newSizeY = round((m_yMax - m_yMin) / m_resolution);

  newMap.resize(newSizeX * newSizeY);

  for (int x = 0; x < newSizeX; x++)
  {
    for (int y = 0; y < newSizeY; y++)
    {
      float newCell = 0;

      for (int xx = 0; xx < downRatio; xx++)
        for (int yy = 0; yy < downRatio; yy++)
          newCell += getCell(x * downRatio + xx, y * downRatio + yy);

      newCell /= (downRatio * downRatio);

      newMap[x + y * newSizeX] = p2l(newCell);
    }
  }

  setSize(m_xMin, m_xMax, m_yMin, m_yMax, m_resolution);
  m_map = newMap;
}

/*---------------------------------------------------------------
              computeMatchingWith
 ---------------------------------------------------------------*/
void COccupancyGridMap2D::determineMatching2D(
    const mrpt::maps::CMetricMap* otherMap2,
    const CPose2D& otherMapPose_,
    TMatchingPairList& correspondences,
    const TMatchingParams& params,
    TMatchingExtraResults& extraResults) const
{
  MRPT_START

  extraResults = TMatchingExtraResults();

  ASSERT_GT_(params.decimation_other_map_points, 0);
  ASSERT_LT_(params.offset_other_map_points, params.decimation_other_map_points);

  ASSERT_(otherMap2->GetRuntimeClass()->derivedFrom(CLASS_ID(CPointsMap)));
  const auto* otherMap = static_cast<const CPointsMap*>(otherMap2);

  const TPose2D otherMapPose = otherMapPose_.asTPose();

  const size_t nLocalPoints = otherMap->size();
  std::vector<float> x_locals(nLocalPoints), y_locals(nLocalPoints), z_locals(nLocalPoints);

  const float sin_phi = sin(otherMapPose.phi);
  const float cos_phi = cos(otherMapPose.phi);

  size_t nOtherMapPointsWithCorrespondence = 0;  // Number of points with one corrs. at least
  size_t nTotalCorrespondences = 0;              // Total number of corrs
  float _sumSqrDist = 0;

  // The number of cells to look around each point:
  const int cellsSearchRange = round(params.maxDistForCorrespondence / m_resolution);

  // Initially there are no correspondences:
  correspondences.clear();

  // Hay mapa local?
  if (!nLocalPoints) return;  // No

  // Solo hacer matching si existe alguna posibilidad de que
  //  los dos mapas se toquen:
  // -----------------------------------------------------------
  float local_x_min = std::numeric_limits<float>::max();
  float local_x_max = -std::numeric_limits<float>::max();
  float local_y_min = std::numeric_limits<float>::max();
  float local_y_max = -std::numeric_limits<float>::max();

  const auto& otherMap_pxs = otherMap->getPointsBufferRef_x();
  const auto& otherMap_pys = otherMap->getPointsBufferRef_y();
  const auto& otherMap_pzs = otherMap->getPointsBufferRef_z();

  // Translate all local map points:
  for (unsigned int localIdx = params.offset_other_map_points; localIdx < nLocalPoints;
       localIdx += params.decimation_other_map_points)
  {
    // Girar y desplazar cada uno de los puntos del local map:
    const float xx = x_locals[localIdx] =
        otherMapPose.x + cos_phi * otherMap_pxs[localIdx] - sin_phi * otherMap_pys[localIdx];
    const float yy = y_locals[localIdx] =
        otherMapPose.y + sin_phi * otherMap_pxs[localIdx] + cos_phi * otherMap_pys[localIdx];
    z_locals[localIdx] = /* otherMapPose.z +*/ otherMap_pzs[localIdx];

    // mantener el max/min de los puntos:
    local_x_min = min(local_x_min, xx);
    local_x_max = max(local_x_max, xx);
    local_y_min = min(local_y_min, yy);
    local_y_max = max(local_y_max, yy);
  }

  // If the local map is entirely out of the grid,
  //   do not even try to match them!!
  if (local_x_min > m_xMax || local_x_max < m_xMin || local_y_min > m_yMax || local_y_max < m_yMin)
    return;  // Matching is NULL!

  const cellType thresholdCellValue = p2l(0.5f);

  // For each point in the other map:
  for (unsigned int localIdx = params.offset_other_map_points; localIdx < nLocalPoints;
       localIdx += params.decimation_other_map_points)
  {
    // Starting value:
    float maxDistForCorrespondenceSquared = square(params.maxDistForCorrespondence);

    // For speed-up:
    const float x_local = x_locals[localIdx];
    const float y_local = y_locals[localIdx];
    const float z_local = z_locals[localIdx];

    // Look for the occupied cell closest from the map point:
    float min_dist = 1e6;
    TMatchingPair closestCorr;

    // Get the indexes of cell where the point falls:
    const int cx0 = x2idx(x_local);
    const int cy0 = y2idx(y_local);

    // Get the rectangle to look for into:
    const int cx_min = max(0, cx0 - cellsSearchRange);
    const int cx_max = min(static_cast<int>(m_size_x) - 1, cx0 + cellsSearchRange);
    const int cy_min = max(0, cy0 - cellsSearchRange);
    const int cy_max = min(static_cast<int>(m_size_y) - 1, cy0 + cellsSearchRange);

    // Will be set to true if a corrs. is found:
    bool thisLocalHasCorr = false;

    // Look in nearby cells:
    for (int cx = cx_min; cx <= cx_max; cx++)
    {
      for (int cy = cy_min; cy <= cy_max; cy++)
      {
        // Is an occupied cell?
        if (m_map[cx + cy * m_size_x] < thresholdCellValue)
        {
          const float residual_x = idx2x(cx) - x_local;
          const float residual_y = idx2y(cy) - y_local;

          // Compute max. allowed distance:
          maxDistForCorrespondenceSquared = square(
              params.maxAngularDistForCorrespondence *
                  params.angularDistPivotPoint.distanceTo(TPoint3D(x_local, y_local, 0)) +
              params.maxDistForCorrespondence);

          // Square distance to the point:
          const float this_dist = square(residual_x) + square(residual_y);

          if (this_dist < maxDistForCorrespondenceSquared)
          {
            if (!params.onlyKeepTheClosest)
            {
              // save the correspondence:
              nTotalCorrespondences++;
              TMatchingPair mp;
              mp.globalIdx = cx + cy * m_size_x;
              mp.global.x = idx2x(cx);
              mp.global.y = idx2y(cy);
              mp.global.z = z_local;
              mp.localIdx = localIdx;
              mp.local.x = otherMap_pxs[localIdx];
              mp.local.y = otherMap_pys[localIdx];
              mp.local.z = otherMap_pzs[localIdx];
              correspondences.push_back(mp);
            }
            else
            {
              // save the closest only:
              if (this_dist < min_dist)
              {
                min_dist = this_dist;

                closestCorr.globalIdx = cx + cy * m_size_x;
                closestCorr.global.x = idx2x(cx);
                closestCorr.global.y = idx2y(cy);
                closestCorr.global.z = z_local;
                closestCorr.localIdx = localIdx;
                closestCorr.local.x = otherMap_pxs[localIdx];
                closestCorr.local.y = otherMap_pys[localIdx];
                closestCorr.local.z = otherMap_pzs[localIdx];
              }
            }

            // At least one:
            thisLocalHasCorr = true;
          }
        }
      }
    }  // End of find closest nearby cell

    // save the closest correspondence:
    if (params.onlyKeepTheClosest && (min_dist < maxDistForCorrespondenceSquared))
    {
      nTotalCorrespondences++;
      correspondences.push_back(closestCorr);
    }

    // At least one corr:
    if (thisLocalHasCorr)
    {
      nOtherMapPointsWithCorrespondence++;

      // Accumulate the MSE:
      _sumSqrDist += min_dist;
    }

  }  // End "for each local point"...

  extraResults.correspondencesRatio =
      nOtherMapPointsWithCorrespondence / d2f(nLocalPoints / params.decimation_other_map_points);
  extraResults.sumSqrDist = _sumSqrDist;

  MRPT_END
}

/*---------------------------------------------------------------
          isEmpty
  ---------------------------------------------------------------*/
bool COccupancyGridMap2D::isEmpty() const { return m_is_empty; }
/*---------------------------------------------------------------
        operator <
  ---------------------------------------------------------------*/
bool mrpt::maps::operator<(
    const COccupancyGridMap2D::TPairLikelihoodIndex& e1,
    const COccupancyGridMap2D::TPairLikelihoodIndex& e2)
{
  return e1.first > e2.first;
}

/*---------------------------------------------------------------
        computePathCost
  ---------------------------------------------------------------*/
float COccupancyGridMap2D::computePathCost(float x1, float y1, float x2, float y2) const
{
  float sumCost = 0;

  float dist = sqrt(square(x1 - x2) + square(y1 - y2));
  int nSteps = round(1.5f * dist / m_resolution);

  for (int i = 0; i < nSteps; i++)
  {
    float x = x1 + (x2 - x1) * i / d2f(nSteps);
    float y = y1 + (y2 - y1) * i / d2f(nSteps);
    sumCost += getPos(x, y);
  }

  if (nSteps)
    return sumCost / d2f(nSteps);
  else
    return 0;
}

float COccupancyGridMap2D::compute3DMatchingRatio(
    [[maybe_unused]] const mrpt::maps::CMetricMap* otherMap,
    [[maybe_unused]] const mrpt::poses::CPose3D& otherMapPose,
    [[maybe_unused]] const TMatchingRatioParams& params) const
{
  return 0;
}

bool COccupancyGridMap2D::nn_single_search(
    const mrpt::math::TPoint3Df& query,
    mrpt::math::TPoint3Df& result,
    float& out_dist_sqr,
    uint64_t& resultIndexOrID) const
{
  // delegate to the 2D version:
  mrpt::math::TPoint2Df r;
  bool res = nn_single_search({query.x, query.y}, r, out_dist_sqr, resultIndexOrID);
  result = {r.x, r.y, .0f};
  return res;
}
void COccupancyGridMap2D::nn_multiple_search(
    const mrpt::math::TPoint3Df& query,
    const size_t N,
    std::vector<mrpt::math::TPoint3Df>& results,
    std::vector<float>& out_dists_sqr,
    std::vector<uint64_t>& resultIndicesOrIDs) const
{
  // delegate to the 2D version:
  std::vector<mrpt::math::TPoint2Df> r;
  nn_multiple_search({query.x, query.y}, N, r, out_dists_sqr, resultIndicesOrIDs);
  results.resize(r.size());
  for (size_t i = 0; i < r.size(); i++) results[i] = {r[i].x, r[i].y, .0f};
}
void COccupancyGridMap2D::nn_radius_search(
    const mrpt::math::TPoint3Df& query,
    const float search_radius_sqr,
    std::vector<mrpt::math::TPoint3Df>& results,
    std::vector<float>& out_dists_sqr,
    std::vector<uint64_t>& resultIndicesOrIDs,
    size_t maxPoints) const
{
  // delegate to the 2D version:
  std::vector<mrpt::math::TPoint2Df> r;
  nn_radius_search(
      {query.x, query.y}, search_radius_sqr, r, out_dists_sqr, resultIndicesOrIDs, maxPoints);
  results.resize(r.size());
  for (size_t i = 0; i < r.size(); i++) results[i] = {r[i].x, r[i].y, .0f};
}

bool COccupancyGridMap2D::nn_single_search(
    const mrpt::math::TPoint2Df& query,
    mrpt::math::TPoint2Df& result,
    float& out_dist_sqr,
    uint64_t& resultIndexOrID) const
{
  std::vector<mrpt::math::TPoint2Df> r;
  std::vector<float> dist_sqr;
  std::vector<uint64_t> resultIndices;
  nn_multiple_search(query, 1, r, dist_sqr, resultIndices);
  if (r.empty()) return false;  // none found
  result = r[0];
  out_dist_sqr = dist_sqr[0];
  resultIndexOrID = resultIndices[0];
  return true;
}

void COccupancyGridMap2D::nn_multiple_search(
    const mrpt::math::TPoint2Df& query,
    const size_t N,
    std::vector<mrpt::math::TPoint2Df>& results,
    std::vector<float>& out_dists_sqr,
    std::vector<uint64_t>& resultIndicesOrIDs) const
{
  results.clear();
  results.reserve(N);
  out_dists_sqr.clear();
  out_dists_sqr.reserve(N);
  resultIndicesOrIDs.clear();
  resultIndicesOrIDs.reserve(N);

  int cx_query = x2idx(query.x), cy_query = y2idx(query.y);

  mrpt::saturate<int>(cx_query, 0, getSizeX() - 1);
  mrpt::saturate<int>(cy_query, 0, getSizeY() - 1);

  const cellType thresholdCellValue = p2l(0.5f);
  const float resolutionSqr = mrpt::square(m_resolution);

  for (int searchRadiusInCells = 0;
       results.size() < N && searchRadiusInCells < std::max<int>(getSizeX(), getSizeY());
       searchRadiusInCells++)
  {
    int cx0 = cx_query - searchRadiusInCells;
    int cx1 = cx_query + searchRadiusInCells;
    if (cx1 < 0 || cx0 >= static_cast<int>(getSizeX())) continue;

    int cy0 = cy_query - searchRadiusInCells;
    int cy1 = cy_query + searchRadiusInCells;
    if (cy1 < 0 || cy0 >= static_cast<int>(getSizeY())) continue;

    mrpt::saturate<int>(cx0, 0, getSizeX() - 1);
    mrpt::saturate<int>(cy0, 0, getSizeY() - 1);
    mrpt::saturate<int>(cx1, 0, getSizeX() - 1);
    mrpt::saturate<int>(cy1, 0, getSizeY() - 1);

    std::map<int, std::pair<int, int>> dists2cells;

    auto lambdaAddCell = [&dists2cells, cx_query, cy_query](int cx, int cy)
    {
      int distSqr = mrpt::square(cx - cx_query) + mrpt::square(cy - cy_query);
      dists2cells[distSqr] = {cx, cy};
    };

    for (int cx = cx0; cx <= cx1; cx++)
    {
      if (int cy = cy0; m_map[cx + cy * m_size_x] < thresholdCellValue) lambdaAddCell(cx, cy);
      if (int cy = cy1; cy1 != cy0 && m_map[cx + cy * m_size_x] < thresholdCellValue)
        lambdaAddCell(cx, cy);
    }
    for (int cy = cy0 + 1; cy < cy1; cy++)
    {
      if (int cx = cx0; m_map[cx + cy * m_size_x] < thresholdCellValue) lambdaAddCell(cx, cy);
      if (int cx = cx1; m_map[cx + cy * m_size_x] < thresholdCellValue) lambdaAddCell(cx, cy);
    }

    // Add the top best "N" neighbors:
    for (auto it = dists2cells.begin(); it != dists2cells.end() && results.size() < N; ++it)
    {
      const int cx = it->second.first;
      const int cy = it->second.second;
      out_dists_sqr.push_back(it->first * resolutionSqr);
      results.push_back({idx2x(cx), idx2y(cy)});
      resultIndicesOrIDs.push_back(cx + cy * m_size_x);
    }
  }
}

void COccupancyGridMap2D::nn_radius_search(
    const mrpt::math::TPoint2Df& query,
    const float search_radius_sqr,
    std::vector<mrpt::math::TPoint2Df>& results,
    std::vector<float>& out_dists_sqr,
    std::vector<uint64_t>& resultIndicesOrIDs,
    size_t maxPoints) const
{
  results.clear();
  out_dists_sqr.clear();
  resultIndicesOrIDs.clear();

  if (search_radius_sqr == 0) return;

  int cx_query = x2idx(query.x), cy_query = y2idx(query.y);

  const cellType thresholdCellValue = p2l(0.5f);
  const float resolutionSqr = mrpt::square(m_resolution);
  const int maxSearchRadiusInCells =
      static_cast<int>(std::ceil(std::sqrt(search_radius_sqr) / m_resolution));
  const int maxSearchRadiusSqrInCells = mrpt::square(maxSearchRadiusInCells);

  for (int searchRadiusInCells = 0; searchRadiusInCells <= maxSearchRadiusInCells;
       searchRadiusInCells++)
  {
    int cx0 = cx_query - searchRadiusInCells;
    int cx1 = cx_query + searchRadiusInCells;
    if (cx1 < 0 || cx0 >= static_cast<int>(getSizeX())) continue;

    int cy0 = cy_query - searchRadiusInCells;
    int cy1 = cy_query + searchRadiusInCells;
    if (cy1 < 0 || cy0 >= static_cast<int>(getSizeY())) continue;

    mrpt::saturate<int>(cx0, 0, getSizeX() - 1);
    mrpt::saturate<int>(cy0, 0, getSizeY() - 1);
    mrpt::saturate<int>(cx1, 0, getSizeX() - 1);
    mrpt::saturate<int>(cy1, 0, getSizeY() - 1);

    auto lambdaAddCell = [&](int cx, int cy)
    {
      int distSqr = mrpt::square(cx - cx_query) + mrpt::square(cy - cy_query);
      if (distSqr > maxSearchRadiusSqrInCells) return;

      out_dists_sqr.push_back(distSqr * resolutionSqr);
      results.push_back({idx2x(cx), idx2y(cy)});
      resultIndicesOrIDs.push_back(cx + cy * m_size_x);
    };

    for (int cx = cx0; cx <= cx1; cx++)
    {
      if (int cy = cy0; m_map[cx + cy * m_size_x] < thresholdCellValue) lambdaAddCell(cx, cy);
      if (int cy = cy1; cy1 != cy0 && m_map[cx + cy * m_size_x] < thresholdCellValue)
        lambdaAddCell(cx, cy);
    }
    for (int cy = cy0 + 1; cy < cy1; cy++)
    {
      if (int cx = cx0; m_map[cx + cy * m_size_x] < thresholdCellValue) lambdaAddCell(cx, cy);
      if (int cx = cx1; m_map[cx + cy * m_size_x] < thresholdCellValue) lambdaAddCell(cx, cy);
    }

    if (maxPoints && results.size() >= maxPoints) break;
  }
}
