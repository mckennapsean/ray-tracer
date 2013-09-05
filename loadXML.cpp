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

// XML scene loader
// originally adapted from code provided by Cem Yuksel


// libraries, namespace
#include "scene.cpp"
#include "object/sphere.cpp"
#include "library/tinyxml2/tinyxml2.cpp"
using namespace scene;
using namespace tinyxml2;


// scene properties
Node rootNode;
Camera camera;


// scene image
RenderImage renderImage;


// to compare two strings and return boolean
#define compare(a,b) (strcmp(a, b) == 0)


// functions for loading scene
void loadNode(Node *node, XMLElement *element, int level = 0);
void readVector(XMLElement *element, Point &v);
void readFloat(XMLElement *element, float &f);


// begin loading scene from file
int loadScene(const char *filename){
  
  // make sure file exists
  XMLDocument doc(filename);
  if(doc.LoadFile(filename)){
    printf("Failed to load the file '%s'\n", filename);
    exit(EXIT_FAILURE);
  }
  
  // make sure file has XML tag
  XMLElement *xml = doc.FirstChildElement("xml");
  if(!xml){
    printf("No 'xml' tag found.\n");
    exit(EXIT_FAILURE);
  }
  
  // make sure file has a scene tag
  XMLElement *scene = xml->FirstChildElement("scene");
  if(!scene){
    printf("No 'scene' tag found.\n");
    exit(EXIT_FAILURE);
  }
  
  // make sure file has a camera tag
  XMLElement *cam = xml->FirstChildElement("camera");
  if(!cam){
    printf("No 'camera' tag found.\n");
    exit(EXIT_FAILURE);
  }
  
  // load root node from file
  rootNode.init();
  loadNode(&rootNode, scene);
  
  // load camera from file
  camera.init();
  camera.dir += camera.pos;
  XMLElement *camChild = cam->FirstChildElement();
  while(camChild){
    
    // load all camera values
    if(compare(camChild->Value(), "position"))
      readVector(camChild, camera.pos);
    if(compare(camChild->Value(), "target"))
      readVector(camChild, camera.dir);
    if(compare(camChild->Value(), "up"))
      readVector(camChild, camera.up);
    if(compare(camChild->Value(), "fov"))
      readFloat(camChild, camera.fov);
    if(compare(camChild->Value(), "width"))
      camChild->QueryIntAttribute("value", &camera.imgWidth);
    if(compare(camChild->Value(), "height"))
      camChild->QueryIntAttribute("value", &camera.imgHeight);
    camChild = camChild->NextSiblingElement();
  }
  
  // ?????
  camera.dir -= camera.pos;
  camera.dir.Normalize();
  Point x = camera.dir ^ camera.up;
  camera.up = (x ^ camera.dir).GetNormalized();
  
  // initialize scene image
  renderImage.Init(camera.imgWidth, camera.imgHeight);
  
  // return success
  return 1;
}


// coordinate print level for debugging scene input
void printIndent(int level){
  for(int i = 0; i < level; i++)
    printf("  ");
}


// add a node's children from the XML structure
void loadNode(Node *node, XMLElement *element, int level){
  
  // grab first child
  XMLElement *child = element->FirstChildElement();
  
  // continue for all children
  while(child){
    
    // check if child is an object
    if(compare(child->Value(), "object")){
      
      // store child as node
      Node *childNode = new Node;
      node->appendChild(childNode);
      
      // set child node's name
      const char* name = child->Attribute("name");
      childNode->setName(name);
      printIndent(level);
      printf("object [");
      if(name)
        printf("%s", name);
      printf("]");
      
      // set child node's type
      const char* type = child->Attribute("type");
      if(type){
        if(compare(type, "sphere")){
          Object *aSphere = new Sphere();
          childNode->setObject(&*aSphere);
          printf(" - Sphere");
        }
      }
      printf("\n");
      
      // load next child
      loadNode(childNode, child, level + 1);
    
    // check if child is a scaling term
    }else if(compare(child->Value(), "scale")){
      float v = 1;
      Point s(1, 1, 1);
      readFloat(child, v);
      readVector(child, s);
      s *= v;
      node->scale(s.x, s.y, s.z);
      printIndent(level);
      printf("scale %f %f %f\n", s.x, s.y, s.z);
      
    // check if child is a translation term
    }else if(compare(child->Value(), "translate")){
      Point t(0, 0, 0);
      readVector(child, t);
      node->translate(t);
      printIndent(level);
      printf("translate %f %f %f\n", t.x, t.y, t.z);
    }
    
    // grab the next child (if any left)
    child = child->NextSiblingElement();
  }
}


// read in a vector from an XML element
void readVector(XMLElement *element, Point &v){
  
  // set vector values
  double x = (double) v.x;
  double y = (double) v.y;
  double z = (double) v.z;
  element->QueryDoubleAttribute("x", &x);
  element->QueryDoubleAttribute("y", &y);
  element->QueryDoubleAttribute("z", &z);
  v.x = (float) x;
  v.y = (float) y;
  v.z = (float) z;
}


// read in a float from an XML element
void readFloat(XMLElement *element, float &f){
  double d = (double) f;
  element->QueryDoubleAttribute("value", &d);
  f = (float) d;
}
