// Copyright 2013 Sean McKenna
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//

// a ray tracer in C++


// libraries, namespace
#include <thread>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <random>
#include "library/loadXML.cpp"
#include "library/scene.cpp"
using namespace std;


// scene to load (project #) + all ray tracing options & settings
string xml = "scenes/prj12.xml";
bool printXML = false;
bool zBuffer = false;
bool sampleCount = false;
int bounceCount = 5;
int sampleMin = 4;
int sampleMax = 32;
float sampleThreshold = 0.001;
int shadowMin = 8;
int shadowMax = 32;
bool gammaCorr = true;
bool globalIllum = false;
bool irradCache = false;
int samplesGI = 128;
bool invSqFO = true;
bool photonMap = true;
int samplesPM = 1000000;
int bounceCountPM = 5;
float photonRad = 2.0;
int maxPhotons = 100;


// variables for ray tracing
int w;
int h;
int size;
Color24* img;
float* zImg;
float* sampleImg;
IrradianceMap im;
BalancedPhotonMap *pm;


// variables for anti-aliasing brightness calculations (XYZ, Lab)
float perR = 0.2126;
float perG = 0.7152;
float perB = 0.0722;
float Ycutoff = pow(6.0 / 29.0, 3.0);
float Yprecalc = (1.0 / 3.0) * pow(29.0 / 6.0, 2.0);


// setup threading
static const int numThreads = 8;
void rayTracing(int i);
void irradianceCache(int i, int m, LightList lightCache);


// for camera ray generation
void cameraRayVars();
float imageDistance = 1.0;
Point *imageTopLeftV;
Point *dXV;
Point *dYV;
Point *dVx;
Point *dVy;
Point firstPixel;
Transformation* c;
Point cameraRay(float pX, float pY, Point offset);


// ray tracer
int main(){
  
  // load scene: root node, camera, image (and set shadow casting variables)
  loadScene(xml, printXML, shadowMin, shadowMax, globalIllum, irradCache, samplesGI, invSqFO, photonMap);
  
  // set the scene as the root node
  setScene(rootNode);
  
  // set variables for ray tracing
  w = render.getWidth();
  h = render.getHeight();
  size = render.getSize();
  img = render.getRender();
  zImg = render.getZBuffer();
  sampleImg = render.getSample();
  if(globalIllum && irradCache)
    im.Initialize(w, h);
  
  // set variables for generating camera rays
  cameraRayVars();
  
  // compute an irradiance cache for global illumination
  if(globalIllum && irradCache){
    
    // caching light list
    LightList lightCache;
    lightCache.deleteAll();
    string name = "indirect";
    IrradianceCacheLight *l = new IrradianceCacheLight();
    Light *light = NULL;
    l->setLightList(&lights);
    l->setEnvironment(environment);
    l->setSamples(samplesGI);
    light = l;
    light->setName(name);
    lightCache.push_back(light);
    
    // subdivide our image to compute indirect illumination
    bool subdivide = true;
    while(subdivide){
      
      // check if we are on final subdivide
      if(im.GetSubdivLevel() == 0)
        subdivide = false;
      
      // calculate indirect illumination
      int cnt = 0;
      for(int i = 0; i < im.GetDataCount(); i++){
        
        // grab position on image plane
        float px;
        float py;
        im.GetPosition(i, px, py);
        
        // get the pixel number
        int pixel = px + py * w;
        
        // compute the ray tracing cache (if needs to be set)
        if(!im.IsValid(i))
          irradianceCache(pixel, i, lightCache);
      }
      
      // subdivide (if necessary)
      if(subdivide)
        im.Subdivide();
    }
  }
  
  // compute a photon map for global illumination
  if(photonMap){
    
    // initialize photon map
    PhotonMap *map = createPhotonMap(samplesPM);
    
    // calculate total light power for random selection
    float powTot = 0.0;
    int numLights = lights.size();
    float *lightPow = new float[numLights];
    float *lightProb = new float[numLights];
    for(int i = 0; i < numLights; i++){
      if(lights[i]->isPhotonSource()){
        powTot += lights[i]->getPhotonIntensity().Grey();
        lightPow[i] = powTot;
      }else{
        lightPow[i] = -1.0;
      }
    }
    for(int i = 0; i < numLights; i++)
      lightProb[i] = lights[i]->getPhotonIntensity().Grey() / powTot;
    
    // keep track of generated photons
    int genPhotons = 0;
    
    // setup random generator for photon mapping
    mt19937 rnd;
    uniform_real_distribution<float> dist{0.0, 1.0};
    
    // fill our photon map
    while(map->stored_photons < samplesPM){
      
      // photon variables
      Color pow;
      int bounce = 1;
      bool cont = true;
      
      // select random light
      Light *light;
      float probLight;
      int l = 0;
      bool foundLight = false;
      float randomPow = dist(rnd) * powTot;
      while(!foundLight){
        if(randomPow <= lightPow[l]){
          light = lights[l];
          probLight = lightProb[l];
          foundLight = true;
        }
        l++;
      }
      
      // initialize our photon
      pow = light->getPhotonIntensity() * 4.0 * M_PI / probLight;
      Cone randPhoton = light->randomPhoton();
      
      // ignore first hit (direct lighting) unless using Monte Carlo GI
      bool store = false;
      if(globalIllum)
        store = true;
      
      // loop for tracing a photon
      while(cont){
        
        // trace photon in scene
        HitInfo hi = HitInfo();
        bool hit = traceRay(randPhoton, hi);
        
        // if hit, get the node's material
        if(hit){
          Node *n = hi.node;
          Material *m;
          if(n)
            m = n->getMaterial();
          
          // if there is a material that is a photon surface, calculate probabilities
          if(m){
            
            // first, save our photon hit (only if a photon surface & a front hit!)
            if(m->isPhotonSurface() && hi.front && store){
              float *power, *position, *direction;
              power = new float[3];
              pow.GetValue(power);
              position = new float[3];
              hi.p.GetValue(position);
              direction = new float[3];
              randPhoton.dir.GetValue(direction);
              storePhoton(map, power, position, direction);
            }
            
            // pass our photon hit to the surface to get next photon (if not absorbed)
            cont = m->randomPhotonBounce(randPhoton, pow, hi);
            
            // be sure to store following protons
            if(!store)
              store = true;
          }
          
          // otherwise, terminate photon
          else
            cont = false;
        
        // if we hit nothing, terminate photon
        }else
          cont = false;
          
        // check our photon bounce count
        bounce++;
        if(bounce > bounceCountPM)
          cont = false;
      }
      
      // add to our generated photons
      genPhotons++;
    }
    
    // scale photon map by number of generated photons
    float scale = 1.0 / ((float) genPhotons);
    scalePhotonPower(map, scale);
    
    // balance our photon map
    pm = balancePhotonMap(map);
  }
  
  // start ray tracing loop (in parallel with threads)
  thread t[numThreads];
  for(int i = 0; i < numThreads; i++)
    t[i] = thread(rayTracing, i);
  
  // when finished, join all threads back to main
  for(int i = 0; i < numThreads; i++)
    t[i].join();
  
  // output ray-traced image & z-buffer & sample count image (if set)
  render.save("images/image.ppm");
  if(zBuffer){
    render.computeZImage();
    render.saveZImage("images/imageZ.ppm");
  }
  if(sampleCount){
    render.computeSampleImage();
    render.saveSampleImage("images/imageSample.ppm");
  }
}


// ray tracing loop (for an individual pixel)
void rayTracing(int i){
  
  // initial starting pixel
  int pixel = i;
  
  // setup random generator for anti-aliasing & depth-of-field
  mt19937 rnd;
  uniform_real_distribution<float> dist{0.0, 1.0};
  
  // create new light list for thread
  LightList threadLights;
  threadLights.deleteAll();
  
  // update our thread light list
  threadLights = lights;
  
  // if necessary, add new irradiance map light
  if(globalIllum && irradCache){
    IrradianceMapLight *l = new IrradianceMapLight();
    string name = "irradianceMap";
    Light *light = NULL;
    light = l;
    light->setName(name);
    threadLights.push_back(light);
  }
  
  // if necessary, add a photon map light
  if(photonMap && !globalIllum){
    PhotonMapLight *l = new PhotonMapLight();
    l->setPhotonMap(pm, photonRad, maxPhotons);
    string name = "photonMap";
    Light *light = NULL;
    light = l;
    light->setName(name);
    threadLights.push_back(light);
  }
  
  // if necessary, add a Monte Carlo photon map light
  if(photonMap && globalIllum){
    MonteCarloPhotonMapLight *l = new MonteCarloPhotonMapLight();
    l->setPhotonMap(pm, photonRad, maxPhotons);
    l->setEnvironment(environment);
    l->setSamples(samplesGI);
    string name = "monteCarloPhotonMap";
    Light *light = NULL;
    light = l;
    light->setName(name);
    threadLights.push_back(light);
  }
   
  // thread continuation condition
  while(pixel < size){
    
    // number of samples
    int s = 0;
    
    // establish pixel location (center)
    float pX = pixel % w;
    float pY = pixel / w;
    
    // color values to store across samples
    Color col;
    Color colAvg;
    float zAvg = 0.0;
    float rVar = 0.0;
    float gVar = 0.0;
    float bVar = 0.0;
    float var = sampleThreshold;
    float brightness = 0.0;
    
    // random rotation of Halton sequence on circle of confusion
    float dcR = dist(rnd) * 2.0 * M_PI;
    
    // if necessary, update irradiance map light with indirect color
    if(globalIllum && irradCache){
      Color c;
      float z;
      Point N;
      ColorIM cim;
      cim.c = c;
      cim.z = z;
      cim.N = N;
      im.Eval(cim, pX, pY);
      int index = threadLights.size() - 1;
      Light *light = threadLights[index];
      light->setColor(cim.c);
    }
    
    // compute multi-adaptive sampling for each pixel (anti-aliasing)
    while(s < sampleMin || (s != sampleMax && (rVar * perR > var + brightness * var || gVar * perG > var + brightness * var || bVar * perB > var + brightness * var))){
      
      // grab Halton sequence to shift point by on image plane
      float dpX = centerHalton(Halton(s, 3));
      float dpY = centerHalton(Halton(s, 2));
      
      // grab Halton sequence to shift point along circle of confusion
      float dcS = sqrt(Halton(s, 2)) * camera.dof;
      
      // grab Halton sequence to shift point around circle of confusion
      float dcT = Halton(s, 3) * 2.0 * M_PI;
      
      // compute the offset for depth of field sampling
      Point posOffset = (*dVx * cos(dcR + dcT) + *dVy * sin(dcR + dcT)) * dcS;
      
      // transform ray into world space (offset by Halton seqeunce for sampling)
      Point rayDir = cameraRay(pX + dpX, pY + dpY, posOffset);
      Cone *ray = new Cone();
      ray->pos = camera.pos + c->transformFrom(posOffset);
      ray->dir = c->transformFrom(rayDir);
      ray->radius = 0.0;
      ray->tan = dXV->x / (2.0 * imageDistance);
      
      // traverse through scene DOM
      // transform rays into model space
      // detect ray intersections and get back HitInfo
      HitInfo hi = HitInfo();
      bool hit = traceRay(*ray, hi);
      
      // update z-buffer, if necessary
      if(zBuffer)
        zAvg = (zAvg * s + hi.z) / (float) (s + 1);
      
      // if hit, get the node's material
      if(hit){
        Node *n = hi.node;
        Material *m;
        if(n)
          m = n->getMaterial();
        
        // if there is a material, shade the pixel
        // 5-passes for reflections and refractions
        if(m)
          col = m->shade(*ray, hi, threadLights, bounceCount);
        
        // otherwise color it white (as a hit)
        else
          col.Set(0.929, 0.929, 0.929);
        
      // if we hit nothing, draw the background
      }else{
        Point p = Point((float) pX / w, (float) pY / h, 0.0);
        Color b = background.sample(p);
        col = b;
      }
      
      // compute average color
      float rAvg = (colAvg.r * s + col.r) / (float) (s + 1);
      float gAvg = (colAvg.g * s + col.g) / (float) (s + 1);
      float bAvg = (colAvg.b * s + col.b) / (float) (s + 1);
      colAvg.Set(rAvg, gAvg, bAvg);
      
      // compute color variances
      rVar = (rVar * s + (col.r - rAvg) * (col.r - rAvg)) / (float) (s + 1);
      gVar = (gVar * s + (col.g - gAvg) * (col.g - gAvg)) / (float) (s + 1);
      bVar = (bVar * s + (col.b - bAvg) * (col.b - bAvg)) / (float) (s + 1);
      
      // calculate and update brightness average using XYZ and Lab space
      float Y = perR * rAvg + perG * gAvg + perB * bAvg;
      float Y13 = Y;
      if(Y13 > Ycutoff)
        Y13 = pow(Y13, 1.0 / 3.0);
      else
        Y13 = Yprecalc * Y13 + (4.0 / 29.0);
      brightness = (116.0 * Y13 - 16.0) / 100.0;
      
      // increment sample count
      s++;
      
      // watch for errors at any individual sample, terminate thread if so
      if(colAvg[0] != colAvg[0] || colAvg[1] != colAvg[1] || colAvg[2] != colAvg[2]){
        cout << "ERROR - pixel " << pixel << " & sample " << s << endl;
        s = sampleMax;
        pixel = size;
      }
    }
    
    // gamma correction
    if(gammaCorr){
      colAvg.r = pow(colAvg.r, 1.0 / 2.2);
      colAvg.g = pow(colAvg.g, 1.0 / 2.2);
      colAvg.b = pow(colAvg.b, 1.0 / 2.2);
    }
    
    // color the pixel image
    img[pixel] = Color24(colAvg);
    
    // update the z-buffer image, if necessary
    if(zBuffer)
      zImg[pixel] = zAvg;
    
    // update the sample count image, if necessary
    if(sampleCount)
      sampleImg[pixel] = s;
    
    // re-assign next pixel (naive, but works)
    pixel += numThreads;
  }
}


// irradiance cache (for global illumination & indirect lighting at a single pixel)
void irradianceCache(int i, int m, LightList lightCache){
  
  // establish pixel location (center)
  float pX = i % w;
  float pY = i / w;
  
  // color value for cache
  Color col;
  
  // set offset to zero
  Point posOffset = Point(0,0,0);
  
  // transform ray into world space
  Point rayDir = cameraRay(pX, pY, posOffset);
  Cone *ray = new Cone();
  ray->pos = camera.pos;
  ray->dir = c->transformFrom(rayDir);
  ray->radius = 0.0;
  ray->tan = dXV->x / (2.0 * imageDistance);
  
  // traverse through scene DOM
  // transform rays into model space
  // detect ray intersections and get back HitInfo
  HitInfo hi = HitInfo();
  bool hit = traceRay(*ray, hi);
  
  // if hit, get the node's material
  if(hit){
    Node *n = hi.node;
    Material *m;
    if(n)
      m = n->getMaterial();
    
    // if there is a material, get our indirect light color for cache
    if(m)
      col = m->shade(*ray, hi, lightCache);
  }
  
  // set our irradiance map variables
  ColorIM cim;
  cim.c = col;
  cim.z = hi.z;
  cim.N = hi.n;
  im.Set(m, cim);
}


// create variables for camera ray generation
void cameraRayVars(){
  float fov = camera.fov * M_PI / 180.0;
  float aspectRatio = (float) w / (float) h;
  imageDistance = camera.focalDist;
  float imageTipY = imageDistance * tan(fov / 2.0);
  float imageTipX = imageTipY * aspectRatio;
  float dX = (2.0 * imageTipX) / (float) w;
  float dY = (2.0 * imageTipY) / (float) h;
  imageTopLeftV = new Point(-imageTipX, imageTipY, -imageDistance);
  dXV = new Point(dX, 0.0, 0.0);
  dYV = new Point(0.0, -dY, 0.0);
  firstPixel = *imageTopLeftV + (*dXV * 0.5) + (*dYV * 0.5);
  
  // set up camera transformation (only need to rotate coordinates)
  c = new Transformation();
  Matrix *rotate = new cyMatrix3f();
  rotate->Set(camera.cross, camera.up, -camera.dir);
  c->transform(*rotate);
  
  // get normalized rays on the focal plane
  dVx = new Point(1.0, 0.0, 0.0);
  dVy = new Point(0.0, 1.0, 0.0);
}


// compute camera ray direction
Point cameraRay(float pX, float pY, Point offset){
  Point ray = firstPixel + (*dXV * pX) + (*dYV * pY) - offset;
  ray.Normalize();
  return ray;
}
