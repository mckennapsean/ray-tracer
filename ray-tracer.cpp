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


// for ray tracing
int w;
int h;
int size;
Color24 white;
Color24 black;
Color24* img;
float* zImg;
void objectIntersection(Node &n, Ray r, int pixel);


// for threading
static const int numThreads = 8;
void rayTracing(int i);


// for camera ray generation
void cameraRayVars();
Point *imageTopLeftV;
Point *dXV;
Point *dYV;
Point firstPixel;
Point cameraPos;
Point cameraDir;
Transformation* c;
Point cameraRay(int pX, int pY);


// ray tracer
int main(){
  
  // load scene: root node, camera, image
  loadScene("scenes/prj1.xml", true);
  
  // set up colors & background image color
  white.Set(233, 233, 233);
  black.Set(33, 33, 33);
  render.setBackground(black);
  
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
  
  // output ray-traced image & z-buffer
  render.save("images/image.ppm");
  render.computeZBuffer();
  render.saveZBuffer("images/imageZ.ppm");
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
    ray->pos = cameraPos;
    ray->dir = c->transformFrom(rayDir);
    
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
  imageTopLeftV = new Point(-imageTipX, imageTipY, -imageDistance);
  dXV = new Point(dX, 0.0, 0.0);
  dYV = new Point(0.0, -dY, 0.0);
  firstPixel = *imageTopLeftV + (*dXV * 0.5) + (*dYV * 0.5);
  
  // set up camera transformation (translation + rotation)
  Point cameraPos = camera.pos;
  c = new Transformation();
  c->translate(cameraPos);
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


// recursive object intersection through all scene objects
void objectIntersection(Node &n, Ray r, int pixel){
  
  // loop on child nodes
  int j = 0;
  int numChild = n.getNumChild();
  while(j < numChild){
    
    // grab child node
    Node *child = n.getChild(j);
    Object *obj = child->getObject();
    
    // transform rays into model space (or local space)
    Ray r2 = child->toModelSpace(r);
    
    // compute ray intersections
    HitInfo h = HitInfo();
    bool hit = obj->intersectRay(r2, h);
    
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
