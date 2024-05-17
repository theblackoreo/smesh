#include "saharaMobility.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <iomanip>      // std::setprecision

#include <cmath>
namespace ns3
{
NS_LOG_COMPONENT_DEFINE ("SaharaMobility");
NS_OBJECT_ENSURE_REGISTERED (SaharaMobility);

TypeId ns3::SaharaMobility::GetTypeId(void) {
	static TypeId tid = TypeId("ns3::SaharaMobility")
							.SetParent<MobilityModel>()
							.SetGroupName ("caac")
							.AddConstructor<SaharaMobility>()
							;
	return tid;
}

TypeId ns3::SaharaMobility::GetInstanceTypeId() const
{
	return SaharaMobility::GetTypeId();
}

ns3::SaharaMobility::SaharaMobility() {

	// default value where it is initialized 

	m_theta =0 ; //heading is initialized to zero
	m_min_velocity = 0; //Cannot go backward. Minimum speed is stand still.
	m_max_velocity = 30;
	m_min_acceleration = -3.6576; // m/s^2
	m_max_acceleration = 3.6576;
}
ns3::SaharaMobility::~SaharaMobility() {
}


void
SaharaMobility::SetPosition(const Vector &position){

	DoSetPosition(position);
}

void
 ns3::SaharaMobility::SetVelocity(const Vector &velocity){
	NS_LOG_DEBUG (  "Setting new values: " << "Vel:"<<velocity );

	m_baseVelocity = velocity;
	NotifyCourseChange ();	
}



void SaharaMobility::SetAccelerationValue (double value)
{
	double newValue;
	if (value>m_max_acceleration)
	{
		newValue = m_max_acceleration;
	}
	if (value<m_min_acceleration)
	{
		newValue = m_min_acceleration;
	}
	else
	{
		newValue = value;
	}
	
	Vector newAccerl (newValue, 0,0);
	SetVelocityAndAcceleration (GetVelocity(), newAccerl);
}


inline Vector
SaharaMobility::DoGetVelocity (void) const
{
	NS_LOG_FUNCTION (this);
	//NS_LOG_DEBUG ("baseTime=" << m_baseTime << "\tBase Velocity=" << m_baseVelocity);
	double t = (Simulator::Now () - m_baseTime).GetSeconds ();

	return Vector (m_baseVelocity.x + m_acceleration.x*t,
			m_baseVelocity.y + m_acceleration.y*t,
			m_baseVelocity.z + m_acceleration.z*t);
}

inline Vector
SaharaMobility::DoGetPosition (void) const
{
	NS_LOG_FUNCTION (this << m_baseTime);
	double t = (Simulator::Now () - m_baseTime).GetSeconds ();

	double half_t_square = t*t*0.5;

	/*
    NS_LOG_DEBUG ( RED ("Vital Info\n\t") << BOLD("TimeNow: ") << Now()<< BOLD(" BasePosX: ") << m_basePosition.x  << BOLD (" Accel=") << m_acceleration 
                << BOLD(" HalfTSq=") << half_t_square << BOLD(" t=") << t 
                << BOLD ("\n\tDoGetPosition() ") << m_basePosition.x + m_baseVelocity.x*t + m_acceleration.x*half_t_square );
				*/


	return Vector (m_basePosition.x + m_baseVelocity.x*t + m_acceleration.x*half_t_square,
			m_basePosition.y + m_baseVelocity.y*t + m_acceleration.y*half_t_square,
			m_basePosition.z + m_baseVelocity.z*t + m_acceleration.z*half_t_square);
}

void
SaharaMobility::DoSetPosition (const Vector &position)
{
	NS_LOG_FUNCTION (this << position);
	m_baseVelocity = DoGetVelocity ();
	m_baseTime = Simulator::Now ();
	m_basePosition = position;
	NotifyCourseChange ();
}



void ns3::SaharaMobility::SetVelocityAndAcceleration(const Vector &velocity, const Vector &acceleration)
{
    
	NS_LOG_FUNCTION (this << velocity << acceleration);
	
	NS_LOG_DEBUG (  "Setting new values: " << "Vel:"<<velocity << " Accl:" << acceleration );
	m_basePosition = DoGetPosition ();
	m_baseTime = Simulator::Now ();

  //NS_LOG_DEBUG ("\t" << "\tnewBasePOS: " << m_basePosition << "\tBaseTime: " << m_baseTime);
	m_baseVelocity = velocity;
  m_acceleration = acceleration;

	

    //SanityCheck(); //Used this for debugging.

    NotifyCourseChange ();
		//Check if we're going to violate speed limits (0 to m_max_velocity)
    if (acceleration.x > 0 && m_baseVelocity.x == m_max_velocity) //trying to speedup when velocity is max
    {
				//Set acceleration to zero
        m_acceleration = Vector (0,0,0);
       // NS_LOG_DEBUG ( BLUE ("\tSpeed limit hit!") );
        // no further changes are needed. Change in acceleration has on effect on velocity
        
        return; 
    }
    else if (acceleration.x < 0 && m_baseVelocity.x == 0) //Trying to slow down when velocity is zero
    {
        m_acceleration = Vector (0,0,0);
       // NS_LOG_DEBUG ( BLUE ("\tZERO speed limit hit!") );
        // no further changes are needed. Change in acceleration has on effect on velocity
        return;
    }
    if (acceleration.x == 0)
    {
       // NS_LOG_DEBUG(YELLOW ("COURSE CHANGED ACCELERATION IS ZERO!"));
        //NotifyCourseChange();
        return;
    }
    //Okay, so we are either "speedig up, but not at max speed" or "slowing down but not stand-still"
	/*
		A node maintains a certain acceleration until it hits top speed, when it hit top speed, acceleration is set to 0
		An event is created & sechudled to fire when max speed is reached to set the acceleration to 0.
		We're cancelling that event because acceleration has changed
	*/
	NS_LOG_DEBUG(Simulator::Now() << " : Cancelling speedlimit event. Vel=" << m_baseVelocity << " Acceleration=" << acceleration);
	m_limit_event.Cancel();

	//If a node maintains this acceleration, when will it hit its speed limit?

    if (m_acceleration.x < 0) //we're slowing down, let's find the time it takes to reach 0
    {
        //Scheduling an event to set acceleration to zero
        m_timeToSpeedlimit = Seconds( m_baseVelocity.x / std::abs(m_acceleration.x) );
    }
    else if (m_acceleration.x > 0) //We're speeding up, let's find the time it takes to reach max velocity
    {
        m_timeToSpeedlimit = Seconds( (m_max_velocity - m_baseVelocity.x) / m_acceleration.x );
    }
		else
		{
			m_timeToSpeedlimit = Seconds(0); //for completion purposes
		}
		
	NS_LOG_DEBUG ("\tCurrent Velocity= " << m_baseVelocity);
	NS_LOG_DEBUG ("\tCurrent Acceleration= " << m_acceleration);
  NS_LOG_DEBUG ("\tLimit event in : " << m_timeToSpeedlimit);


	/*
	We will schedule an event for when will the velocity limit be reached
	*/
	//Let's check if the calculated time to reach the limit is not zero
	if (m_timeToSpeedlimit.GetSeconds() > 0 )
	{
			NS_LOG_DEBUG (Simulator::Now() <<" -- Scheduled setting ACCELERATION to ZERO in " << m_timeToSpeedlimit );
			//If we're speeding up...
			if (m_acceleration.x >0)
					m_limit_event = Simulator::Schedule(m_timeToSpeedlimit, &SaharaMobility::SetAccelerationToZeroMaxVelocity, this);
			else
			{		//If we're slowing down...
					m_limit_event = Simulator::Schedule(m_timeToSpeedlimit, &SaharaMobility::SetAccelerationToZeroZeroVelocity, this);
			}
			
    }
	else	//If the calculated time to speed limit is zero, set acceleration to zero. 
	{
		//NS_LOG_LOGIC (Simulator::Now() << " Max Velocity reached. Setting Acceleration to ZERO");
		m_acceleration = Vector (0,0,0);
	}

	//NotifyCourseChange ();
}
void SaharaMobility::SetAccelerationToZeroZeroVelocity()
{
    NS_LOG_FUNCTION(this);
    SetVelocityAndAcceleration ( Vector(0,0,0) , Vector(0,0,0));
}
/*
Vector SaharaMobility::GetPosition (void) const
{
	return DoGetPosition ();
}
*/
void SaharaMobility::SetAccelerationToZeroMaxVelocity ()
{
    NS_LOG_FUNCTION (this);
    SetVelocityAndAcceleration ( Vector(m_max_velocity,0,0) , Vector(0,0,0));

}


Vector ns3::SaharaMobility::GetAcceleration() {
	return m_acceleration;
}

void ns3::SaharaMobility::SetAccelerationLimits(double min,
		double max) {

	m_min_acceleration = min;
	m_max_acceleration = max;
}

void ns3::SaharaMobility::SetMaxVelocity(double max) {
	this->m_max_velocity = max;
}

double SaharaMobility::GetAccelerationValue ()
{
	return m_acceleration.x;
}



}