/*=====================================
Frostbite style physically based camera
from https://placeholderart.wordpress.com/2014/12/15/implementing-a-physically-based-camera-automatic-exposure/
=======================================*/

#define MIN_ISO         100.0
#define MAX_ISO         6400.0
#define MIN_APERTURE    1.8
#define MAX_APERTURE    22.0
#define MIN_SHUTTER     1.0 / 4000.0
#define MAX_SHUTTER     1.0 / 30.0

float ComputeISO(float aperture, float shutterSpeed, float ev){
    return (sqrt(aperture) * 100.0f) / (shutterSpeed * pow(2.0f, ev));
} 
// Given the camera settings compute the current exposure value
float ComputeEV(float aperture, float shutterSpeed, float iso){
    return log2((sqrt(aperture) * 100.0) / (shutterSpeed * iso));
} 
// Using the light metering equation compute the target exposure value
float ComputeTargetEV(float averageLuminance){
    // K is a light meter calibration constant
    float K = 12.5;
    return log2(averageLuminance * 100.0f / K);
}
/*
* Get an exposure using the Saturation-based Speed method.
*/
float getSaturationBasedExposure(float aperture, float shutterSpeed, float iso){
    float l_max = (7800.0f / 65.0f) * sqrt(aperture) / (iso * shutterSpeed);
    return 1.0f / l_max;
} 

/*
* Get an exposure using the Standard Output Sensitivity method.
* Accepts an additional parameter of the target middle grey.
*/
float getStandardOutputBasedExposure(float aperture, float shutterSpeed, float iso, float middleGrey){
    float l_avg = (1000.0f / 65.0f) * sqrt(aperture) / (iso * shutterSpeed);
    return middleGrey / l_avg;
}