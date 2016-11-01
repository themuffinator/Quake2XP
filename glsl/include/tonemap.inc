// tonemap algortim from http://filmicgames.com/archives/75
float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
float W = 11.2;

float exposureBias = 0.666;
float power = 0.5;

vec3 Tonemap(vec3 x){
  return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 performTonemap(vec3 tmp){
  vec3 tmpMap  = tmp;
  vec3 cur = Tonemap(u_glareParams.z * tmpMap);
  
  vec3 newcur;
  vec3 whitescale;

  whitescale = 1.0 / Tonemap(vec3(W, W, W));
  newcur = cur * whitescale;
  
  return vec3(pow(newcur.r, power), pow(newcur.g, u_glareParams.y), pow(newcur.b, u_glareParams.y));
}