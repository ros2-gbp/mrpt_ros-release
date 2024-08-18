/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2024, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */
#pragma once

#include <mrpt/math/CMatrixDynamic.h>
#include <mrpt/obs/CObservation.h>
#include <mrpt/obs/CSinCosLookUpTableFor2DScans.h>
#include <mrpt/poses/CPose3D.h>
#include <mrpt/serialization/CSerializable.h>

#include <variant>
#include <vector>

namespace mrpt::obs
{
class CObservationVelodyneScan;
class CObservation2DRangeScan;

/** \addtogroup mrpt_obs_grp
 * @{ */

/** A `CObservation`-derived class for raw range data from a 2D or 3D
 * rotating scanner. This class is the preferred alternative to
 * CObservationVelodyneScan and CObservation2DRangeScan in MRPT 2.x, since it
 * exposes range data as an organized matrix, more convenient for feature
 * detection directly on "range images" and on points stored as a matrix in the
 * member organizedPoints.
 *
 * Check out the main data fields in the list of members below.
 *
 *  Note that this object has \b two timestamp fields:
 *  - The standard `CObservation::timestamp` field in the base class, which
 * should contain the accurate satellite-based UTC timestamp if available,
 * and
 *  - the field originalReceivedTimestamp, with the
 * local computer-based timestamp based on the reception of the message in
 * the computer.
 *
 * Both timestamps correspond to the firing of the **first** laser in
 * the scan, i.e. the first column in organizedPoints.
 *
 * The reference frame for the 3D LIDAR is with +X pointing forward, +Z up.
 *
 * \note New in MRPT 2.0.0
 * \sa CObservation, mrpt::hwdrivers::CVelodyneScanner
 */
class CObservationRotatingScan : public CObservation
{
  DEFINE_SERIALIZABLE(CObservationRotatingScan, mrpt::obs)

 public:
  enum class ExternalStorageFormat : uint8_t
  {
    None = 0,            //!< is always stored in memory
    MRPT_Serialization,  //!< Uses mrpt-serialization binary file
    /// Plain text, format explained in saveToTextFile()
    PlainTextFile
  };

 protected:
  ExternalStorageFormat m_externally_stored = ExternalStorageFormat::None;
  std::string m_external_file;

 public:
  /** @name Scan range data
    @{ */

  /** Number of "Lidar rings" (e.g. 16 for a Velodyne VLP16, etc.). This
   * should be constant for a given LiDAR scanner.
   * All matrices defined below have this number of rows.
   */
  uint16_t rowCount{0};

  /** Number of lidar "firings" for this scan. It is assumed
   * that firings occur at a fixed rate. Consecutive scans ("scan"=instance of
   * this class) may have different number of firings, and different start and
   * end azimuth. All matrices defined below have this number of columns.
   */
  uint16_t columnCount{0};

  /** The NxM matrix of distances (ranges) for each direction
   * (columns) and for each laser "ring" (rows). Matrix element are integers
   * for efficiency of post-processing filters, etc. Zero means no return
   * (i.e. invalid range). This member must be always provided, containing the
   * ranges for the STRONGEST ray returns.
   * To obtain ranges in meters, multiply this matrix by `rangeResolution`.
   *
   * \sa organizedPoints
   */
  mrpt::math::CMatrix_u16 rangeImage{0, 0};

  /** If present, it contains all 3D points, in local coordinates wrt the
   * sensor, for all !=0 entries in \a rangeImage.
   */
  mrpt::math::CMatrixDynamic<mrpt::math::TPoint3Df> organizedPoints{0, 0};

  /** Optionally, an intensity channel. Matrix with a 0x0 size if not
   * provided. */
  mrpt::math::CMatrix_u8 intensityImage{0, 0};

  /** Optional additional layers, e.g. LAST return, etc. for lidars with
   * multiple returns per firing. A descriptive name of what the alternative
   * range means as std::map Key, e.g. `FIRST`, `SECOND`. */
  std::map<std::string, mrpt::math::CMatrix_u16> rangeOtherLayers;

  /**  Real-world scale (in meters) of integer units in range images (e.g.
   * 0.002 means 1 range unit is 2 millimeters) */
  double rangeResolution;

  /** Azimuth of the first and last columns in `ranges`, with respect to the
   * *sensor* forward direction.
   * Note that startAzimuth may be possitive or negative, and azimuthSpan can
   * be too to reflect the direction of rotation of the scanner:
   * >0 is CCW, <0 is CW.
   */
  double startAzimuth{-M_PI}, azimuthSpan{2 * M_PI};

  /** Time (in seconds) that passed since `startAzimuth` (first column) to
   * `endAzimuth` (last column). */
  double sweepDuration{.0};

  /** The driver should fill in this observation */
  std::string lidarModel{"UNKNOWN_SCANNER"};

  /** The maximum range allowed by the device, in meters (e.g. 100m).
   * Stored
   * here by the driver while capturing based on the sensor model. */
  double minRange{1.0}, maxRange{130.0};

  /** The SE(3) pose of the sensor on the robot/vehicle frame
   * of reference */
  mrpt::poses::CPose3D sensorPose;

  /** The local computer-based timestamp based on the
   * reception of the message in the computer. \sa
   * has_satellite_timestamp, CObservation::timestamp in the
   * base class, which should contain the accurate
   * satellite-based UTC timestamp.  */
  mrpt::system::TTimeStamp originalReceivedTimestamp{INVALID_TIMESTAMP};

  /** If true, CObservation::timestamp has been generated
   * from accurate satellite clock. Otherwise, no GPS data
   * is available and timestamps are based on the local
   * computer clock. */
  bool has_satellite_timestamp{false};

  /** @} */

  /** @name "Convert from" API
   * @{ */

  void fromVelodyne(const mrpt::obs::CObservationVelodyneScan& o);
  void fromScan2D(const mrpt::obs::CObservation2DRangeScan& o);

  /** Will convert from another observation if it's any of the supported
   * source types (see fromVelodyne(), fromScan2D()) and
   * return true, or will return false otherwise if there is no known way to
   * convert from the passed object. */
  bool fromGeneric(const mrpt::obs::CObservation& o);
  /** @} */

  /** @name Delayed-load manual control methods.
    @{ */
  // See base class docs.
  void load_impl() const override;
  void unload() const override;
  /** @} */

  /** \name Point cloud external storage functions
   * @{ */
  inline bool isExternallyStored() const
  {
    return m_externally_stored != ExternalStorageFormat::None;
  }
  inline const std::string& getExternalStorageFile() const { return m_external_file; }
  void setAsExternalStorage(const std::string& fileName, const ExternalStorageFormat fmt);

  void overrideExternalStorageFormatFlag(const ExternalStorageFormat fmt)
  {
    m_externally_stored = fmt;
  }

  /** Write scan data to a plain text, each line has:
   *   `x y z range intensity row_idx col_idx`
   *
   * For each point in the organized point cloud.
   * Invalid points (e.g. no lidar return) are stored as (x,y,z)=(0,0,0) and
   * range=0.
   *
   * \return true on success
   */
  bool saveToTextFile(const std::string& filename) const;

  /** Loads the range, intensity, and organizedPoints members from a plain
   * text file in the format describd in saveToTextFile()
   */
  bool loadFromTextFile(const std::string& filename);

  /** @} */
  // See base class docs
  mrpt::system::TTimeStamp getOriginalReceivedTimeStamp() const override;

  // See base class docs
  void getSensorPose(mrpt::poses::CPose3D& out_sensorPose) const override
  {
    out_sensorPose = sensorPose;
  }
  void setSensorPose(const mrpt::poses::CPose3D& newSensorPose) override
  {
    sensorPose = newSensorPose;
  }
  void getDescriptionAsText(std::ostream& o) const override;

};  // End of class def.

/** @} */

}  // namespace mrpt::obs

namespace mrpt::typemeta
{  // Specialization must occur in the same namespace
MRPT_DECLARE_TTYPENAME_PTR_NAMESPACE(CObservationRotatingScan, ::mrpt::obs)
}
