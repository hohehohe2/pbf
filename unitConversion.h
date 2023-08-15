#ifndef HOHE2_unitConversion_H
#define HOHE2_unitConversion_H

/*
(mass unit) => 1 particle
(length unit) => 1 mass water equivalent
(time unit)

(mass unit) and (length unit) must satisfy the confition "water density must be 1".
i.e. (mass unit) / (length unit)^3 = 1

(density unit) => (mass unit) / (length unit)^3 = 1
(velocity unit) => (length unit) / (time unit)
(acceleration unit) => (length unit) / (time unit)^2
(force unit) => (mass unit)(length unit) / (time unit)^2 => (length unit)^4 / (time unit)^2
(pressure unit) => (mass unit) / (length unit)(time unit)^2 =>  (length unit)^2 / (time unit)^2
(gas constant unit:density to pressure) => (pressure unit) / (density unit) => (pressure unit) => (length unit)^2 / (time unit)^2
(viscosity unit) => (force unit)(time unit) / (length unit)^2 => (mass unit) / ((time unit)(length unit)) => (length unit)^2 / (time unit)

Example. Gravity:

(length unit) = 1m
(mass unit) = 1t
(time unit) = 1s
(acceleration unit) = (length unit) / (time unit) ^ 2 = 1m/s^2
G = -9.8 m/s

(length unit) = 1cm
(mass unit) = 1kg
(time unit) = 1s
G = 9.8m/s = -9.8 (100cm)/s^2 = -980 cm/s^2

If the user set the length unit 0.01 in meter (i.e. cm), acceleration value gets 100 times.
 */

namespace hohe2
{

///Unit conversion class. MTS stands for meter, ton, second.
class UnitConversion
{
    ///Unit length in meters.
    float unitLength_;

    ///Unit time in seconds.
    float unitTime_;

    float convToMtsMass_;
    float convToMtsVelocity_;
    float convToMtsAcceleration_;
    float convToMtsDensity_;
    float convToMtsForce_;
    float convToMtsPressure_;
    float convToMtsGasConst_;
    float convToMtsViscosity_;

public:
    ///Constructor.
    /**
       @param [in] unitLengthInMeters Unit length you use for input and output data, in meters.\n
       e.g. if you use cm as a unit, set 0.01.
       @param [in] unitTimeInSeconds Unit time  you use for input and output data, in seconds.\n
       e.g. if you use milli second as a unit, set 0.001.
    */
    UnitConversion(float unitLengthInMeters=1, float unitTimeInSeconds=1)
        :unitLength_(unitLengthInMeters), unitTime_(unitTimeInSeconds)
    {
        calculateUnits_();
    }

    void setUnitLengthInMeters(float unitLengthInMeters){unitLength_ = unitLengthInMeters; calculateUnits_();}
    void setUnitTimeInSeconds(float unitTimeInSeconds){unitTime_ = unitTimeInSeconds; calculateUnits_();}

    float unitLengthInMts() const{return unitLength_;}
    float unitTimeInMts() const{return unitTime_;}
    float unitWeightInMts() const{return convToMtsMass_;}

    float lengthToMts(float value) const{return value * unitLength_;}
    float timeToMts(float value) const{return value * unitTime_;}
    float weightToMts(float value) const{return value * convToMtsMass_;}
    float velocityToMts(float value) const{return value * convToMtsVelocity_;}
    float accelerationToMts(float value) const{return value * convToMtsAcceleration_;}
    float densityToMts(float value) const{return value;}
    float forceToMts(float value) const{return value * convToMtsForce_;}
    float pressureToMts(float value) const{return value * convToMtsPressure_;}
    float gasConstToMts(float value) const{return value * convToMtsGasConst_;}
    float viscosityToMts(float value) const{return value * convToMtsViscosity_;}

    float lengthFromMts(float value) const{return value / unitLength_;}
    float timeFromMts(float value) const{return value / unitTime_;}
    float weightFromMts(float value) const{return value / convToMtsMass_;}
    float velocityFromMts(float value) const{return value / convToMtsVelocity_;}
    float accelerationFromMts(float value) const{return value / convToMtsAcceleration_;}
    float densityFromMts(float value) const{return value;}
    float forceFromMts(float value) const{return value / convToMtsForce_;}
    float pressureFromMts(float value) const{return value / convToMtsPressure_;}
    float gasConstFromMts(float value) const{return value / convToMtsGasConst_;}
    float viscosityFromMts(float value) const{return value / convToMtsViscosity_;}

    void calculateUnits_()
    {
        convToMtsMass_ = unitLength_ * unitLength_ * unitLength_;
        convToMtsVelocity_ = unitLength_/unitTime_;
        convToMtsAcceleration_ = unitLength_/unitTime_/unitTime_;
        convToMtsDensity_ = 1;
        convToMtsForce_ = unitLength_ * unitLength_ * unitLength_ * unitLength_ / unitTime_ / unitTime_;
        convToMtsPressure_ = unitLength_ * unitLength_ / unitTime_ / unitTime_;
        convToMtsGasConst_ = unitLength_ * unitLength_ / unitTime_ / unitTime_;
        convToMtsViscosity_ = unitLength_ * unitLength_ / unitTime_;
    }
};

}
#endif
