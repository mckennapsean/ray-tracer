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
#include "loadXML.cpp"
#include "scene.cpp"
using namespace std;


// for ray tracing
int w;
int h;
int size;
Color24 white = {233, 233, 233};
Color24 black = {33, 33, 33};
Color24* img;
float* zImg;


// for threading
static const int numThreads = 8;
void rayTracing(int i);


// for camera ray generation
void cameraRayVars();
Point3 *imageTopLeftV;
Point3 *dXV;
Point3 *dYV;
Point3 firstPixel;
Point3 cameraPos;
Point3 cameraDir;
Transformation* c;
Point3 cameraRay(int pX, int pY);


// for tracing rays to objects
void objectIntersection(Node &n, Ray r, int pixel);


// ray tracer
int main(){
  
  // load scene: root node, camera, image
  loadScene("scenes/prj1.xml");
  
  // set up background image color
  renderImage.setBackground(black);
  
  // set variables for ray tracing
  w = renderImage.GetWidth();
  h = renderImage.GetHeight();
  size = w * h; 
  img = renderImage.GetPixels();
  zImg = renderImage.GetZBuffer();
  
  // set variables for generating camera rays
  cameraRayVars();
  
  // start ray tracing loop (in parallel with threads)
  thread t[numThreads];
  for(int i = 0; i < numThreads; i++)
    t[i] = thread(rayTracing, i);
  
  // when finished, join all threads back to main
  for(int i = 0; i < numThreads; i++)
    t[i].join();
  
  // output ray-traced image & z-buffer
  renderImage.SaveImage("images/image.ppm");
  renderImage.ComputeZBufferImage();
  renderImage.SaveZImage("images/imageZ.ppm");
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
    Point3 rayDir = cameraRay(pX, pY);
    Ray *ray = new Ray();
    ray->p = cameraPos;
    ray->dir = c->TransformFrom(rayDir);
    
    // traverse through scene DOM
    // transform rays into model space
    // detect ray intersections & update pixel
    objectIntersection(rootNode, *ray, pixel);
    
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
  imageTopLeftV = new Point3(-imageTipX, imageTipY, -imageDistance);
  dXV = new Point3(dX, 0.0, 0.0);
  dYV = new Point3(0.0, -dY, 0.0);
  firstPixel = *imageTopLeftV + (*dXV * 0.5) + (*dYV * 0.5);
  
  // set up camera transformation (translation + rotation)
  Point3 cameraPos = camera.pos;
  c = new Transformation();
  c->Translate(cameraPos);
  Matrix3 *rotate = new cyMatrix3f();
  Point3 cameraDir = camera.dir;
  Point3 cameraUp = camera.up;
  cameraDir.Normalize();
  cameraUp.Normalize();
  Point3 cameraCross = cameraDir ^ cameraUp;
  cameraCross.Normalize();
  rotate->Set(cameraCross, cameraUp, -cameraDir);
  c->Transform(*rotate);
}


// compute camera rays
Point3 cameraRay(int pX, int pY){
  Point3 ray = firstPixel + (*dXV * pX) + (*dYV * pY);
  ray.Normalize();
  return ray;
}


// recursive object intersection through all scene objects
void objectIntersection(Node &n, Ray r, int pixel){
  
  // loop on child nodes
  int j = 0;
  int numChild = n.GetNumChild();
  while(j < numChild){
    
    // grab child node
    Node *child = n.GetChild(j);
    Object *obj = child->GetObject();
    
    // transform rays into model space (or local space)
    Ray r2 = child->ToNodeCoords(r);
    
    // compute ray intersections
    HitInfo h = HitInfo();
    bool hit = obj->IntersectRay(r2, h);
    
    // check the ray computation, update pixel & z-buffer
    if(hit){
      img[pixel] = white;
      if(h.z < zImg[pixel])
        zImg[pixel] = h.z;
    }
        
    // recursively check this child's children
    objectIntersection(*child, r2, pixel);
    j++;
  }
}
