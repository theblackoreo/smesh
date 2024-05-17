

#ifndef SAHARA_MOBILITY_H
#define SAHARA_MOBILITY_H

#include "ns3/constant-acceleration-mobility-model.h"
#include "ns3/mobility-model.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"

namespace ns3
{

class SaharaMobility : public MobilityModel
{
public:
  
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId() const;
 
  SaharaMobility ();
  virtual ~SaharaMobility ();


  void SetPosition(const Vector &position);
  void SetVelocity(const Vector &velocity);



  void SetVelocityAndAcceleration (const Vector &velocity, const Vector &acceleration);
  /** \brief Change the acceleration. If the value is greater than the max limit, then it's set to that limit. If it's less than the lowest limit, it's set to the lower limit.
   */
  void SetAccelerationValue (double value);

  /** \brief Returns the acceleration as a ns3::Vector form
   */ 
  Vector GetAcceleration();

  /** \brief Returns the acceleration as a double
   */
  double GetAccelerationValue ();

  /** \brief Sets the minimum and maximum acceleration values in m/s^2. This depends on the vehicle.
   * \param min the minimum acceleration value. This is a negative value for deceleration.
   * \param max the maximum acceleration value.
   */
  void SetAccelerationLimits (double min, double max);
  /** \brief Sets the maximum speed. This is usually the speed limit of the road.
   */
  void SetMaxVelocity (double max);

  



private:

  /* \brief A function that is scheduled to set acceleration to zero when max velocity is reached.
  */
  void SetAccelerationToZeroMaxVelocity ();
  /* \brief A function that is scheduled to set acceleration to zero when zero velocity is reached. 
  */
  void SetAccelerationToZeroZeroVelocity ();

  virtual Vector DoGetPosition (void) const;
  virtual void DoSetPosition (const Vector &position);
  virtual Vector DoGetVelocity (void) const;

  Time m_baseTime;  //!< the base time
  Vector m_basePosition; //!< the base position
  Vector m_baseVelocity; //!< the base velocity
  Vector m_acceleration;  //!< the acceleration

  Time m_updatePeriod; //!< How often to update
  Time m_timeToSpeedlimit; //!< When acceleration is adjusted, we compute the time at which the vehicle would reach max allowed velocity. We do this so that we would set the acceleration to 0 as vehicles must not exceed the speed limit


  EventId m_limit_event; //!< An event that's scheduled in anticipation of reaching max speed. If acceleration changes before then, it's cancelled, and a new one is scheduled
  double m_theta; //!< Heading angle. If vehicle is moving along x-axis, forward it would be 0, if opposite direction, it's PI (3.14)
  double m_max_velocity;
  double m_min_velocity;
  double m_min_acceleration;
  double m_max_acceleration;
  
};

}


#endif /* SAHARA_MOBILITY_H */