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
using namespace std;


// output an 8-bit array (256 colors) as an RGB image in ASCII PPM format
bool outputImage(const char *file, int w, int h, int img[][3]){
  ofstream f;
  f.open(file);
  
  // if error writing to file
  if(!f)
    return false;
  
  // otherwise, output the image
  f << "P3\n" << w << " " << h << "\n255\n";
  for(int i = 0; i < h; i++){
    for(int j = 0; j < w; j++){
      for(int k = 0; k < 3; k++){
        int pt = i * w + j;
        f << img[pt][k] << " ";
      }
    }
    f << "\n";
  }
  f.close();
  return true;
}


// ray tracer
int main(){
  
  // initialize image variables
  int w = 400;
  int h = 200;
  int img[h * w][3];
  
  // output ray-traced image
  //outputImage("image.ppm", w, h, img);
}
