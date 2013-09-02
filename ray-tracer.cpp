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
#include "scene.h"
using namespace std;


// ray tracer
int main(){
  
  // initialize image variables
  int w = 400;
  int h = 200;
  
  // load scene
  LoadScene("scenes/prj0.xml");
  
  // output ray-traced image & z-buffer
  //SaveImage("images/image.ppm");
  //SaveZImage("images/z-image.ppm");
}
