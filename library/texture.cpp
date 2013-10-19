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


// namespace
using namespace scene;


// Texture from a file
class TextureFile: public Texture{
  private:
    
    // variables
    std:vector<Color24> data;
    int width;
    int height;
  
  public:
    
    // constructor
    TextureFile(){
      width = 0;
      height = 0;
    }
    
    // load a texture from a file
    bool load(){
      
    }
    
    // sample a texture for a color
    Color sample(Point &uvw){
      
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
    
    // sample a texture for a color
    Color sample(&uvw){
      
    }
};
