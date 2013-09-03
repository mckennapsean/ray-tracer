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
#include <fstream>
#include "xmlload.cpp"
#include "scene.cpp"
using namespace std;


// variables for ray tracing
int w;
int h;
int size;
Color24 white = {237, 237, 237};
Color24 black = {27, 27, 27};
Color24* img;


// variables for camera ray generation
void cameraRayVars();
Point3 *imageTopLeftV;
Point3 *dXV;
Point3 *dYV;
Point3 firstPixel;
Point3 cameraRay(int pX, int pY);


// trace a ray against all objects
void objectIntersection(Node &n, Ray r, int pixel);


// ray tracer
int main(){
  
  // load scene: root node, camera, image
  LoadScene("scenes/prj0.xml");
  
  // variables for ray tracing
  w = renderImage.GetWidth();
  h = renderImage.GetHeight();
  size = w * h; 
  img = renderImage.GetPixels();
  
  // variables for generating camera rays
  cameraRayVars();
  
  // ray-tracing loop, per pixel
  for(int i = 0; i < size; i++){
    
    // establish pixel location
    int pX = i % w;
    int pY = i / w;
    
    // compute current ray (in camera space)
    Point3 rayPos = camera.pos;
    Point3 rayDir = cameraRay(pX, pY);
    
    // transform ray into world space
    Transformation* world = new Transformation();
    //world->Translate(rayPos);
    Ray *curr = new Ray();
    curr->p = world->TransformTo(rayPos);
    curr->dir = world->TransformTo(rayPos + rayDir) - curr->p;
    //curr->Normalize();
    //rayDir=world->VectorTransformFrom(rayDir);
    //Ray *curr = new Ray(rayPos, rayDir);
    
    // traverse through scene DOM
    // transform rays into model space
    // detect ray intersections ---> update pixel
    objectIntersection(rootNode, *curr, i);
  }
  
  // output ray-traced image & z-buffer
  renderImage.SaveImage("images/image.ppm");
  //renderImage.SaveZImage("images/z-image.ppm");
}


// create variables for camera ray generation
void cameraRayVars(){
  float fov = camera.fov;
  float aspectRatio = (float) w / (float) h;
  float imageDistance = 1;
  float imageTipY = imageDistance * tan(fov / 2.0 * M_PI / 180.0);
  float imageTipX = imageTipY * aspectRatio;
  float dX = (2.0 * imageTipX) / w;
  float dY = (2.0 * imageTipY) / h;
  imageTopLeftV = new Point3(-imageTipX, imageTipY, imageDistance);
  dXV = new Point3(dX, 0.0, 0.0);
  dYV = new Point3(0.0, -dY, 0.0);
  firstPixel = *imageTopLeftV + (*dXV * 0.5) + (*dYV * 0.5);
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
    
    // check the ray computation, update pixels
    if(hit)
      img[pixel] = white;
    
    // need to update background pixels?
    
    // recursively check this child's children
    objectIntersection(*child, r2, pixel);
    j++;
  }
}
