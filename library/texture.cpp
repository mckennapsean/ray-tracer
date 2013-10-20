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

// texture class (for loading textures & procedural textures)


#include <stdio.h>
// namespace
using namespace scene;


// Texture from a file
class TextureFile: public Texture{
  private:
    
    // variables
    std::vector<Color24> data;
    int width;
    int height;
    
    // read a line from a file to detect end of line or end of file
    int readLine(FILE *f, int size, char *b){
      int i;
      for(i = 0; i < size; i++){
        b[i] = fgetc(f);
        if(feof(f) || b[i] == '\n' || b[i] == '\r'){
          b[i] = '\0';
          return i + 1;
        }
      }
      return i;
    }
    
    // load a PPM image file into memory
    bool loadPPM(FILE *f, int &w, int &h, std::vector<Color24> &data){
      
      // variables
      int bufferSize = 1024;
      char buffer[bufferSize];
      
      // read the header
      readLine(f, bufferSize, buffer);
      
      // ensure we have a proper PPM file
      if(buffer[0] != 'P' && buffer[1] != '6')
        return false;
      
      // ignore comments in header
      readLine(f, bufferSize, buffer);
      while(buffer[0] == '#')
        readLine(f, bufferSize, buffer);
      
      // grab dimensions of PPM
      sscanf(buffer, "%d %d", &width, &height);
      
      // continue skipping comments
      readLine(f, bufferSize, buffer);
      while(buffer[0] == '#')
        readLine(f, bufferSize, buffer);
      
      // load in data for PPM image
      data.resize(width * height);
      fread(data.data(), sizeof(Color24), width * height, f);
      
      // return a succesful load from file
      return true;
    }
  
  public:
    
    // constructor
    TextureFile(){
      width = 0;
      height = 0;
    }
    
    // load a texture from a file
    bool load(){
      
      // reset data contents
      data.clear();
      width = 0;
      height = 0;
      
      // open PPM file as a texture
      FILE *f = fopen(getName().c_str(), "rb");
      
      // test if we have a succesful PPM image file
      if(!f)
        return false;
      bool success = false;
      success = loadPPM(f, width, height, data);
      
      // close PPM file
      fclose(f);
      return success;
    }
    
    // sample a texture for a color
    Color sample(Point &uvw){
      
      // empty texture
      if(width + height == 0)
        return Color(0.0, 0.0, 0.0);
      
      // clamp position into texture space
      
      // calculate sampling positions
      Point u = tileClamp(uvw);
      float x = width * u.x;
      float y = height * u.y;
      int ix = (int) x;
      int iy = (int) y;
      float fx = x - ix;
      float fy = y - iy;
      
      // test near edges to shift sampling positions
      if(ix < 0)
        ix -= (ix / width - 1.0) * width;
      if(ix >= width)
        ix -= (ix / width) * width;
      int ixp = ix + 1;
      if(ixp >= width)
        ixp -= width;
      if(iy < 0)
        iy -= (iy / height - 1.0) * height;
      if(iy >= height)
        iy -= (iy / height) * height;
      int iyp = iy + 1;
      if(iyp >= height)
        iyp -= height;
      
      // return the color at this point
      return data[iy * width + ix].ToColor() * ((1.0 - fx) * (1.0 - fy)) + data[iy * width + ixp].ToColor() * (fx * ( 1.0 - fy)) + data[iyp * width + ix].ToColor() * ((1.0 - fx) * fy) + data[iyp * width + ixp].ToColor() * (fx * fy);
    }
};


// Procedural texture (checkerboard)
class TextureChecker: public Texture{
  private:
    
    // variables
    Color color1;
    Color color2;
    
  public:
    
    // constructor
    TextureChecker(){
      color1 = Color(0.0, 0.0, 0.0);
      color2 = Color(1.0, 1.0, 1.0);
    }
    
    // set texture checker colors
    void setColor1(Color c){
      color1 = c;
    }
    void setColor2(Color c){
      color2 = c;
    }
    
    // sample a texture for a color
    Color sample(Point &uvw){
      
      // clamp position into texture space
      Point u = tileClamp(uvw);
      
      // one of four cases to one of two colors, in a checkerboard fashion
      if(u.x <= 0.5){
        if(u.y <= 0.5)
          return color1;
        else
          return color2;
      }else{
        if(u.y <= 0.5)
          return color2;
        else
          return color1;
      }
    }
};
