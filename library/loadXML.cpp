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
#include "objects.cpp"
#include "lights.cpp"
#include "materials.cpp"
#include "tinyxml2/tinyxml2.cpp"
using namespace scene;
using namespace tinyxml2;


// scene properties
Node rootNode;
Camera camera;
Object *aSphere;
MaterialList materials;
LightList lights;


// scene image
Render render;


// debug mode for printing out loaded scene
bool print;


// compare two strings and return boolean
#define compare(a,b) (strcmp(a, b) == 0)


// functions for loading scene
void loadScene(XMLElement *e);
void loadNode(Node *n, XMLElement *e, int level = 0);
void loadTransform(Transformation *t, XMLElement *e, int level);
void loadMaterial(XMLElement *e);
void loadLight(XMLElement *e);
void readVector(XMLElement *element, Point &v);
void readColor(XMLElement *e, Color &c);
void readFloat(XMLElement *element, float &f, const char *name = "value");


// begin loading scene from file
int loadScene(const char *file, bool p = false){
  
  // load debug mode
  print = p;
  
  // make sure file exists
  XMLDocument doc(file);
  if(doc.LoadFile(file)){
    printf("Failed to load the file '%s'\n", file);
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
  
  // clear out initial scene variables
  nodeMaterialList.clear();
  rootNode.init();
  materials.deleteAll();
  lights.deleteAll();
  
  // load object types once
  aSphere = new Sphere();
  
  // pass on scene XML element to load scene elements only
  loadScene(scene);
  
  // assign materials to each node
  int numNodes = nodeMaterialList.size();
  for(int i = 0; i < numNodes; i++){
    Material *mat = materials.find(nodeMaterialList[i].materialName);
    if(mat)
      nodeMaterialList[i].node->setMaterial(mat);
  }
  nodeMaterialList.clear();
  
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
  
  // compute remaining camera values
  camera.setup();
  
  // initialize scene image
  render.init(camera.imgWidth, camera.imgHeight);
  
  // return success
  return 1;
}


// coordinate print level for debugging scene input
void printIndent(int level){
  for(int i = 0; i < level; i++)
    printf("  ");
}


// continue loading the scene from an XML element (recursively load as nodes, materials, or lights)
void loadScene(XMLElement *e){
  for(XMLElement *child = e->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
    
    if(compare(child->Value(), "object"))
      loadNode(&rootNode, child);
    else if(compare(child->Value(), "material"))
      loadMaterial(child);
    else if(compare(child->Value(), "light"))
      loadLight(child);
  }
}


// add a node's children from the XML structure
void loadNode(Node *n, XMLElement *e, int level){
  
  // assign a child to the parent node
  Node *node = new Node();
  n->appendChild(node);
  
  // get the name of the parent node
  const char* name = e->Attribute("name");
  node->setName(name);
  
  // print out object name
  if(print){
    printIndent(level);
    printf("Object [");
    if(name)
      printf("%s", name);
    printf("]");
  }
  
  // get the type of the parent node
  const char* type = e->Attribute("type");
  if(type){
    
    // for sphere
    if(compare(type, "sphere")){
      node->setObject(&*aSphere);
      
      // print out object type
      if(print)
        printf(" - Sphere");
    
    // for unknown object
    }else{
      
      // print out object type
      if(print)
        printf(" - UNKNOWN TYPE");
    }
  }
  
  // get the material type of the parent node
  const char* materialName = e->Attribute("material");
  if(materialName){
    
    // print out object material
    if(print)
      printf(" <%s>", materialName);
    
    // load and save object material
    NodeMaterial nm;
    nm.node = node;
    nm.materialName = materialName;
    nodeMaterialList.push_back(nm);
  }
  if(print)
    printf("\n");
  
  // recursively loop through remaining objects
  for(XMLElement *child = e->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
    if(compare(child->Value(), "object"))
      loadNode(node, child, level + 1);
  }
  
  // load the appropriate transformation information
  loadTransform(node, e, level);
}


// load in the transformation terms for each node
void loadTransform(Transformation *t, XMLElement *e, int level){
  
  // recursively apply transformations to child nodes that have been set already
  for(XMLElement *child = e->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
    
    // check if child is a scaling term
    if(compare(child->Value(), "scale")){
      float v = 1.0;
      Point s(1, 1, 1);
      readFloat(child, v);
      readVector(child, s);
      s *= v;
      t->scale(s.x, s.y, s.z);
      
      // print out scaling term
      if(print){
        printIndent(level + 1);
        printf("scale %f %f %f\n", s.x, s.y, s.z);
      }
      
    // check if child is a rotation term
    }else if(compare(child->Value(), "rotate")){
      Point r(0, 0, 0);
      readVector(child, r);
      r.Normalize();
      float a;
      readFloat(child, a, "angle");
      t->rotate(r, a);
      
      // print out rotation term
      if(print){
        printIndent(level + 1);
        printf("rotate %f degrees around %f %f %f\n", a, r.x, r.y, r.z);
      }
    
    // check if child is a translation term
    }else if(compare(child->Value(), "translate")){
      Point p(0, 0, 0);
      readVector(child, p);
      t->translate(p);
      
      // print out translation term
      if(print){
        printIndent(level + 1);
        printf("translate %f %f %f\n", p.x, p.y, p.z);
      }
    }
  }
}


// load in the material information for each element
void loadMaterial(XMLElement *e){
  
  // initial material
  Material *mat = NULL;
  
  // get & print material name
  const char* name = e->Attribute("name");
  if(print){
    printf("Material [");
    if(name)
      printf("%s", name);
    printf("]");
  }
  
  // get material type
  const char* type = e->Attribute("type");
  if(type){
    
    // blinn-phong material type
    if(compare(type, "blinn")){
      BlinnMaterial *m = new BlinnMaterial();
      mat = m;
      
      // print out material type
      if(print)
        printf(" - Blinn\n");
      
      // check children for material properties
      for(XMLElement *child = e->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
        
        // initialize values
        Color c(1, 1, 1);
        float f = 1.0;
        
        // load diffuse color
        if(compare(child->Value(), "diffuse")){
          readColor(child, c);
          m->setDiffuse(c);
          
          // print out diffuse color
          if(print)
            printf("  diffuse %f %f %f\n", c.r, c.g, c.b);
        
        // load specular color
        }else if(compare(child->Value(), "specular")){
          readColor(child, c);
          m->setSpecular(c);
          
          //print out specular color
          if(print)
            printf("  specular %f %f %f\n", c.r, c.g, c.b);
        
        // load shininess value
        }else if(compare(child->Value(), "shininess")){
          readFloat(child, f);
          m->setShininess(f);
          
          // print out shininess value
          if(print)
            printf("  shininess %f\n", f);
        }
      }
    
    // phong material type
    }else if(compare(type, "phong")){
      PhongMaterial *m = new PhongMaterial();
      mat = m;
      
      // print out material type
      if(print)
        printf(" - Phong\n");
      
      // check children for material properties
      for(XMLElement *child = e->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
        
        // initialize values
        Color c(1, 1, 1);
        float f = 1.0;
        
        // load diffuse color
        if(compare(child->Value(), "diffuse")){
          readColor(child, c);
          m->setDiffuse(c);
          
          // print out diffuse color
          if(print)
            printf("  diffuse %f %f %f\n", c.r, c.g, c.b);
        
        // load specular color
        }else if(compare(child->Value(), "specular")){
          readColor(child, c);
          m->setSpecular(c);
          
          //print out specular color
          if(print)
            printf("  specular %f %f %f\n", c.r, c.g, c.b);
        
        // load shininess value
        }else if(compare(child->Value(), "shininess")){
          readFloat(child, f);
          m->setShininess(f);
          
          // print out shininess value
          if(print)
            printf("  shininess %f\n", f);
        }
      }
    
    // unknown material type
    }else{
      
      // print out material type
      if(print)
        printf(" - UNKNOWN MATERIAL\n");
    }
  }
  
  // add material to materials list
  if(mat){
    mat->setName(name);
    materials.push_back(mat);
  }
}


// load in the light information for each element
void loadLight(XMLElement *e){
  
  // initialize light
  Light *light = NULL;
  
  // get & print light name
  const char* name = e->Attribute("name");
  if(print){
    printf("Light [");
    if(name)
      printf("%s", name);
    printf("]");
  }
  
  // get light type
  const char* type = e->Attribute("type");
  if(type){
    
    // ambient light type
    if(compare(type, "ambient")){
      AmbientLight *l = new AmbientLight();
      light = l;
      
      // print out light type
      if(print)
        printf(" - Ambient\n");
      
      // check children for light properties
      for(XMLElement *child = e->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
        
        // load intensity (color) of light (for all lights)
        if(compare(child->Value(), "intensity")){
          Color c(1, 1, 1);
          readColor(child, c);
          l->setIntensity(c);
          
          // print out light intensity color
          if(print)
            printf("  intensity %f %f %f\n", c.r, c.g, c.b);
        }
      }
    
    
    // direct light type
    }else if(compare(type, "direct")){
      DirectLight *l = new DirectLight();
      light = l;
      
      // print out light type
      if(print)
        printf(" - Direct\n");
      
      // check children for light properties
      for(XMLElement *child = e->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
        
        // load intensity (color) of light (for all lights)
        if(compare(child->Value(), "intensity")){
          Color c(1, 1, 1);
          readColor(child, c);
          l->setIntensity(c);
          
          // print out light intensity color
          if(print)
            printf("  intensity %f %f %f\n", c.r, c.g, c.b);
        
        // load direction of light
        }else if(compare(child->Value(), "direction")){
          Point v(1, 1, 1);
          readVector(child, v);
          l->setDirection(v);
          
          // print out light direction
          if(print)
            printf("  direction %f %f %f\n", v.x, v.y, v.z);
        }
      }
    
    // point light type
    }else if(compare(type, "point")){
      PointLight *l = new PointLight();
      light = l;
      
      // print out light type
      if(print)
        printf(" - Point\n");
      
      // check children for light properties
      for(XMLElement *child = e->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
        
        // load intensity (color) of light (for all lights)
        if(compare(child->Value(), "intensity")){
          Color c(1, 1, 1);
          readColor(child, c);
          l->setIntensity(c);
          
          // print out light intensity color
          if(print)
            printf("  intensity %f %f %f\n", c.r, c.g, c.b);
        
        // load position of light
        }else if(compare(child->Value(), "position")){
          Point v(0, 0, 0);
          readVector(child, v);
          l->setPosition(v);
          
          // print out position of light
          if(print)
            printf("  position %f %f %f\n", v.x, v.y, v.z);
        }
      }
    
    // unknown light type
    }else{
      if(print)
        printf(" - UNKNOWN LIGHT\n");
    }
  }
  
  // add light to lights list
  if(light){
    light->setName(name);
    lights.push_back(light);
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


// read in a color from an XML element
void readColor(XMLElement *e, Color &c){
  
  // set 3-channel color values
  double r = (double) c.r;
  double g = (double) c.g;
  double b = (double) c.b;
  e->QueryDoubleAttribute("r", &r);
  e->QueryDoubleAttribute("g", &g);
  e->QueryDoubleAttribute("b", &b);
  c.r = (float) r;
  c.g = (float) g;
  c.b = (float) b;
  
  // read in color scaling factor
  float f = 1.0;
  readFloat(e, f);
  c *= f;
}


// read in a float from an XML element
void readFloat(XMLElement *element, float &f, const char *name){
  double d = (double) f;
  element->QueryDoubleAttribute(name, &d);
  f = (float) d;
}
