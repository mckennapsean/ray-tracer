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


// ray tracer
int main(){
  
  // load scene: root node, camera, image
  LoadScene("scenes/prj0.xml");
  
  // variables for generating camera rays
  int w = renderImage.GetWidth();
  int h = renderImage.GetHeight();
  int size = w * h;
  Color24 white = {237, 237, 237};
  Color24 black = {27, 27, 27};
  Color24* img = renderImage.GetPixels();
  float fov = camera.fov;
  float aspectRatio = (float) w / (float) h;
  float imageDistance = 1;
  float imageTipY = imageDistance * tan(fov / 2.0 * M_PI / 180.0);
  float imageTipX = imageTipY * aspectRatio;
  float dX = (2.0 * imageTipX) / w;
  float dY = (2.0 * imageTipY) / h;
  Point3 *imageTopLeftV = new Point3(-imageTipX, imageTipY, -imageDistance);
  Point3 *dXV = new Point3(dX, 0.0, 0.0);
  Point3 *dYV = new Point3(0.0, -dY, 0.0);
  Point3 firstPixel = *imageTopLeftV + (*dXV * 0.5) + (*dYV * 0.5);
  
  // generate camera rays for every pixel
  for(int i = 0; i < size; i++){
    
    // generate current ray in camera space
    int pX = i % w;
    int pY = i / w;
    Point3 curr = firstPixel + (*dXV * pX) + (*dYV * pY);
    curr.Normalize();
    
    // transform ray into world space
    
    
    // traverse through scene DOM
    // transform rays into model space
    // detect ray intersections ---> update pixel
    
  }
  
  // output ray-traced image & z-buffer
  //renderImage.SaveImage("images/image.ppm");
  //renderImage.SaveZImage("images/z-image.ppm");
}
