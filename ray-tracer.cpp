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
#include "library/loadXML.cpp"
#include "library/scene.cpp"
using namespace std;


// scene to load (project #) & whether to debug
const char* xml = "scenes/prj3-1.xml";
bool printXML = false;
bool zBuffer = false;
bool specGeom = true;


// for ray tracing
int w;
int h;
int size;
Color24* img;
float* zImg;


// for threading
static const int numThreads = 8;
void rayTracing(int i);


// for camera ray generation
void cameraRayVars();
Point *imageTopLeftV;
Point *dXV;
Point *dYV;
Point firstPixel;
Transformation* c;
Point cameraRay(int pX, int pY);


// ray tracer
int main(){
  
  // load scene: root node, camera, image
  loadScene(xml, printXML);
  
  // set the scene as the root node
  setScene(rootNode);
  
  // whether to use geometry term in specular reflection
  setSpecularGeometry(specGeom);
  
  // set variables for ray tracing
  w = render.getWidth();
  h = render.getHeight();
  size = render.getSize(); 
  img = render.getRender();
  zImg = render.getZBuffer();
  
  // set variables for generating camera rays
  cameraRayVars();
  
  // start ray tracing loop (in parallel with threads)
  thread t[numThreads];
  for(int i = 0; i < numThreads; i++)
    t[i] = thread(rayTracing, i);
  
  // when finished, join all threads back to main
  for(int i = 0; i < numThreads; i++)
    t[i].join();
  
  // output ray-traced image & z-buffer (if set)
  render.save("images/image.ppm");
  if(zBuffer){
    render.computeZBuffer();
    render.saveZBuffer("images/imageZ.ppm");
  }
}


// ray tracing loop (for an individual pixel)
void rayTracing(int i){
  
  // initial starting pixel
  int pixel = i;
   
  // thread continuation condition
  while(pixel < size){
    
    // establish pixel location
    int pX = pixel % w;
    int pY = pixel / w;
    
    // transform ray into world space
    Point rayDir = cameraRay(pX, pY);
    Ray *ray = new Ray();
    ray->pos = camera.pos;
    ray->dir = c->transformFrom(rayDir);
    
    // traverse through scene DOM
    // transform rays into model space
    // detect ray intersections and get back HitInfo
    HitInfo h = HitInfo();
    bool hit = traceRay(*ray, h);
    
    // update z-buffer, if necessary
    if(zBuffer)
      zImg[pixel] = h.z;
    
    // try and get the hit object & material
    Node *n = h.node;
    Material *m;
    if(n)
      m = n->getMaterial();
    
    // if we hit nothing
    Color24 c;
    if(!hit){
      c.Set(0, 0, 0);
      
    // shade pixel if it has material
    }else if(m)
      c = Color24(m->shade(*ray, h, lights));
    
    // otherwise, just color it white
    else
      c.Set(237, 237, 237);
    
    // color the pixel image
    img[pixel] = c;
    // re-assign next pixel (naive, but works)
    pixel += numThreads;
  }
}


// create variables for camera ray generation
void cameraRayVars(){
  float fov = camera.fov * M_PI / 180.0;
  float aspectRatio = (float) w / (float) h;
  float imageDistance = 1.0;
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
}


// compute camera rays
Point cameraRay(int pX, int pY){
  Point ray = firstPixel + (*dXV * pX) + (*dYV * pY);
  ray.Normalize();
  return ray;
}
