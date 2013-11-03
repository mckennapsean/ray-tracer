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
#include "texture.cpp"
#include "tinyxml2/tinyxml2.cpp"
using namespace scene;
using namespace tinyxml2;


// scene properties
Node rootNode;
Camera camera;
Object *aSphere;
Object *aPlane;
TexturedColor background;
TexturedColor environment;
MaterialList materials;
LightList lights;
TextureList textures;
ObjFileList objList;


// scene image
Render render;


// debug mode for printing out loaded scene
bool print;


// functions for loading scene
void loadScene(XMLElement *e);
void loadNode(Node *n, XMLElement *e, int level = 0);
void loadTransform(Transformation *t, XMLElement *e, int level);
void loadMaterial(XMLElement *e);
void loadLight(XMLElement *e);
TextureMap* loadTexture(XMLElement *e);
void readVector(XMLElement *e, Point &v);
void readColor(XMLElement *e, Color &c);
void readFloat(XMLElement *e, float &f, string name = "value");


// begin loading scene from file
int loadScene(string file, bool p = false){
  
  // load debug mode
  print = p;
  
  // make sure file exists
  XMLDocument doc(file.c_str());
  if(doc.LoadFile(file.c_str())){
    cout << "Failed to load the file'" << file << "'" << endl;
    exit(EXIT_FAILURE);
  }
  
  // make sure file has XML tag
  XMLElement *xml = doc.FirstChildElement("xml");
  if(!xml){
    cout << "No 'xml' tag found." << endl;
    exit(EXIT_FAILURE);
  }
  
  // make sure file has a scene tag
  XMLElement *scene = xml->FirstChildElement("scene");
  if(!scene){
    cout << "No 'scene' tag found." << endl;
    exit(EXIT_FAILURE);
  }
  
  // make sure file has a camera tag
  XMLElement *cam = xml->FirstChildElement("camera");
  if(!cam){
    cout << "No 'camera' tag found." << endl;
    exit(EXIT_FAILURE);
  }
  
  // clear out initial scene variables
  nodeMaterialList.clear();
  rootNode.init();
  materials.deleteAll();
  lights.deleteAll();
  textures.clear();
  objList.clear();
  
  // load object types once
  aSphere = new Sphere();
  aPlane = new Plane();
  
  // pass on scene XML element to load scene elements only
  loadScene(scene);
  
  // calculate bounding boxes for all nodes
  rootNode.computeChildBoundBox();
  
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
    string val(camChild->Value());
    if(val == "position")
      readVector(camChild, camera.pos);
    if(val == "target")
      readVector(camChild, camera.dir);
    if(val == "up")
      readVector(camChild, camera.up);
    if(val == "fov")
      readFloat(camChild, camera.fov);
    if(val == "width")
      camChild->QueryIntAttribute("value", &camera.imgWidth);
    if(val == "height")
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
    cout << "  ";
}


// continue loading the scene from an XML element (recursively load as nodes, materials, or lights)
void loadScene(XMLElement *e){
  for(XMLElement *child = e->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
    
    string val(child->Value());
    if(val == "background"){
      Color c(1.0, 1.0, 1.0);
      readColor(child, c);
      background.setColor(c);
      if(print)
        cout << "Background " << c.r << " " << c.g << " " << c.b << endl;
      background.setTexture(loadTexture(child));
    }else if(val == "environment"){
      Color c(1.0, 1.0, 1.0);
      readColor(child, c);
      environment.setColor(c);
      if(print)
        cout << "Environment " << c.r << " " << c.g << " " << c.b << endl;
      environment.setTexture(loadTexture(child));
    }else if(val == "object")
      loadNode(&rootNode, child);
    else if(val == "material")
      loadMaterial(child);
    else if(val == "light")
      loadLight(child);
  }
}


// add a node's children from the XML structure
void loadNode(Node *n, XMLElement *e, int level){
  
  // assign a child to the parent node
  Node *node = new Node();
  n->appendChild(node);
  
  // get the name of the parent node (safely)
  const char* na = e->Attribute("name");
  string name = "";
  if(na)
    name = na;
  node->setName(name);
  
  // print out object name
  if(print){
    printIndent(level);
    cout << "Object [" << name << "]";
  }
  
  // get the type of the parent node
  const char* c = e->Attribute("type");
  if(c){
    string type(c);
    
    // for sphere
    if(type == "sphere"){
      node->setObject(&*aSphere);
      
      // print out object type
      if(print)
        cout << " - Sphere";
    
    // for plane
    }else if(type == "plane"){
      node->setObject(&*aPlane);
      
      // print out object type
      if(print)
        cout << " - Plane";
    
    // for object (composed of triangles)
    }else if(type == "obj"){
      Object *obj = objList.find(name);
      
      // no object on list, so load it as a triangular mesh
      if(obj == NULL){
        TriObj *triObj = new TriObj;
        
        // try to load OBJ file
        string objFile = "objects/" + name + ".txt";
        if(!triObj->load(objFile)){
          if(print)
            cout << " -- ERROR: Cannot load file \"" << objFile << ".\"";
          delete triObj;
        
        // add the OBJ file
        }else{
          objList.append(triObj, name);
          obj = triObj;
        }
      }
      
      // set triangular mesh node object
      node->setObject(obj);
    
    // for unknown object
    }else{
      
      // print out object type
      if(print)
        cout << " - UNKNOWN TYPE";
    }
  }
  
  // get the material type of the parent node
  const char* m = e->Attribute("material");
  if(m){
    string materialName(m);
    
    // print out object material
    if(print)
      cout << " <" << materialName << ">";
    
    // load and save object material
    NodeMaterial nm;
    nm.node = node;
    nm.materialName = materialName;
    nodeMaterialList.push_back(nm);
  }
  if(print)
    cout << endl;
  
  // recursively loop through remaining objects
  for(XMLElement *child = e->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
    string val(child->Value());
    if(val == "object")
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
    string val(child->Value());
    if(val == "scale"){
      float v = 1.0;
      Point s(1, 1, 1);
      readFloat(child, v);
      readVector(child, s);
      s *= v;
      t->scale(s.x, s.y, s.z);
      
      // print out scaling term
      if(print){
        printIndent(level + 1);
        cout << "scale " << s.x << " " << s.y << " " << s.z << endl;
      }
      
    // check if child is a rotation term
    }else if(val == "rotate"){
      Point r(0, 0, 0);
      readVector(child, r);
      r.Normalize();
      float a;
      readFloat(child, a, "angle");
      t->rotate(r, a);
      
      // print out rotation term
      if(print){
        printIndent(level + 1);
        cout << "rotate " << a << " degrees around " << r.x << " " << r.y << " " << r.z << endl;
      }
    
    // check if child is a translation term
    }else if(val == "translate"){
      Point p(0, 0, 0);
      readVector(child, p);
      t->translate(p);
      
      // print out translation term
      if(print){
        printIndent(level + 1);
        cout << "translate " << p.x << " " << p.y << " " << p.z << endl;
      }
    }
  }
}


// load in the material information for each element
void loadMaterial(XMLElement *e){
  
  // initial material
  Material *mat = NULL;
  
  // get & print material name
  string name(e->Attribute("name"));
  if(print)
    cout << "Material [" << name << "]";
  
  // get material type
  const char* c = e->Attribute("type");
  if(c){
    string type(c);
    
    // blinn-phong material type
    if(type == "blinn"){
      BlinnMaterial *m = new BlinnMaterial();
      mat = m;
      
      // print out material type
      if(print)
        cout << " - Blinn" << endl;
      
      // pass on environment texture
      m->setEnvironmentTexture(environment);
      
      // check children for material properties
      for(XMLElement *child = e->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
        
        // initialize values
        Color c(1, 1, 1);
        float f = 1.0;
        
        // load diffuse color
        string val(child->Value());
        if(val == "diffuse"){
          readColor(child, c);
          m->setDiffuse(c);
          m->setDiffuseTexture(loadTexture(child));
          
          // print out diffuse color
          if(print)
            cout << "  diffuse " << c.r << " " << c.g << " " << c.b << endl;
        
        // load specular color
        }else if(val == "specular"){
          readColor(child, c);
          m->setSpecular(c);
          m->setSpecularTexture(loadTexture(child));
          
          //print out specular color
          if(print)
            cout << "  specular " << c.r << " " << c.g << " " << c.b << endl;
        
        // load shininess value (from glossiness value)
        }else if(val == "glossiness"){
          readFloat(child, f);
          m->setShininess(f);
          
          // print out shininess value
          if(print)
            cout << "  shininess " << f << endl;
        
        // load reflection color
        }else if(val == "reflection"){
          readColor(child, c);
          m->setReflection(c);
          m->setReflectionTexture(loadTexture(child));
          
          // print out reflection color
          if(print)
            cout << "  reflection " << c.r << " " << c.g << " " << c.b << endl;
        
        // load refraction color and index
        }else if(val == "refraction"){
          readColor(child, c);
          m->setRefraction(c);
          readFloat(child, f, "index");
          m->setRefractionIndex(f);
          m->setRefractionTexture(loadTexture(child));
          
          // print out refraction color and index
          if(print)
            cout << "  refraction " << c.r << " " << c.g << " " << c.b << " (index " << f << ")" << endl;
        
        // load absorption color
        }else if(val == "absorption"){
          readColor(child, c);
          m->setAbsorption(c);
          
          // print out absorption color
          if(print)
            cout << "  absorption " << c.r << " " << c.g << " " << c.b << endl;
        }
      }
    
    // phong material type
    }else if(type == "phong"){
      PhongMaterial *m = new PhongMaterial();
      mat = m;
      
      // print out material type
      if(print)
        cout << " - Phong" << endl;
      
      // pass on environment texture
      m->setEnvironmentTexture(environment);
      
      // check children for material properties
      for(XMLElement *child = e->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
        
        // initialize values
        Color c(1, 1, 1);
        float f = 1.0;
        
        // load diffuse color
        string val(child->Value());
        if(val == "diffuse"){
          readColor(child, c);
          m->setDiffuse(c);
          
          // print out diffuse color
          if(print)
            cout << "  diffuse " << c.r << " " << c.g << " " << c.b << endl;
        
        // load specular color
        }else if(val == "specular"){
          readColor(child, c);
          m->setSpecular(c);
          
          //print out specular color
          if(print)
            cout << "  specular " << c.r << " " << c.g << " " << c.b << endl;
        
        // load shininess value
        }else if(val == "glossiness"){
          readFloat(child, f);
          m->setShininess(f);
          
          // print out shininess value
          if(print)
            cout << "  shininess " << f << endl;
        
        // load reflection color
        }else if(val == "reflection"){
          readColor(child, c);
          m->setReflection(c);
          
          // print out reflection color
          if(print)
            cout << "  reflection " << c.r << " " << c.g << " " << c.b << endl;
        
        // load refraction color and index
        }else if(val == "refraction"){
          readColor(child, c);
          m->setRefraction(c);
          readFloat(child, f, "index");
          m->setRefractionIndex(f);
          
          // print out refraction color and index
          if(print)
            cout << "  refraction " << c.r << " " << c.g << " " << c.b << " (index " << f << ")" << endl;
        
        // load absorption color
        }else if(val == "absorption"){
          readColor(child, c);
          m->setAbsorption(c);
          
          // print out absorption color
          if(print)
            cout << "  absorption " << c.r << " " << c.g << " " << c.b << endl;
        }
      }
    
    // unknown material type
    }else{
      
      // print out material type
      if(print)
        cout << " - UNKNOWN MATERIAL" << endl;
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
  string name(e->Attribute("name"));
  if(print)
    cout << "Light [" << name << "]";
  
  // get light type
  const char* c = e->Attribute("type");
  if(c){
    string type(c);
    
    // ambient light type
    if(type == "ambient"){
      AmbientLight *l = new AmbientLight();
      light = l;
      
      // print out light type
      if(print)
        cout << " - Ambient" << endl;
      
      // check children for light properties
      for(XMLElement *child = e->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
        
        // load intensity (color) of light (for all lights)
        string val(child->Value());
        if(val == "intensity"){
          Color c(1, 1, 1);
          readColor(child, c);
          l->setIntensity(c);
          
          // print out light intensity color
          if(print)
            cout << "  intensity " << c.r << " " << c.g << " " << c.b << endl;
        }
      }
    
    
    // direct light type
    }else if(type == "direct"){
      DirectLight *l = new DirectLight();
      light = l;
      
      // print out light type
      if(print)
        cout << " - Direct" << endl;
      
      // check children for light properties
      for(XMLElement *child = e->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
        
        // load intensity (color) of light (for all lights)
        string val(child->Value());
        if(val == "intensity"){
          Color c(1, 1, 1);
          readColor(child, c);
          l->setIntensity(c);
          
          // print out light intensity color
          if(print)
            cout << "  intensity " << c.r << " " << c.g << " " << c.b << endl;
        
        // load direction of light
        }else if(val == "direction"){
          Point v(1, 1, 1);
          readVector(child, v);
          l->setDirection(v);
          
          // print out light direction
          if(print)
            cout << "  direction " << v.x << " " << v.y << " " << v.z << endl;
        }
      }
    
    // point light type
    }else if(type == "point"){
      PointLight *l = new PointLight();
      light = l;
      
      // print out light type
      if(print)
        cout << " - Point" << endl;
      
      // check children for light properties
      for(XMLElement *child = e->FirstChildElement(); child != NULL; child = child->NextSiblingElement()){
        
        // load intensity (color) of light (for all lights)
        string val(child->Value());
        if(val == "intensity"){
          Color c(1, 1, 1);
          readColor(child, c);
          l->setIntensity(c);
          
          // print out light intensity color
          if(print)
            cout << "  intensity " << c.r << " " << c.g << " " << c.b << endl;
        
        // load position of light
        }else if(val == "position"){
          Point v(0, 0, 0);
          readVector(child, v);
          l->setPosition(v);
          
          // print out position of light
          if(print)
            cout << "  position " << v.x << " " << v.y << " " << v.z << endl;
        }
      }
    
    // unknown light type
    }else{
      if(print)
        cout << " - UNKNOWN LIGHT" << endl;
    }
  }
  
  // add light to lights list
  if(light){
    light->setName(name);
    lights.push_back(light);
  }
}


// load in texture for this element
TextureMap* loadTexture(XMLElement *e){
  
  // set texture name
  const char* t = e->Attribute("texture");
  if(t){
    string name(t);
    
    // catch unset texture
    if(name == "")
      return NULL;
    
    // initialize texture
    Texture *tex = NULL;
    
    // procedural texture (only checkerboard)
    if(name == "checkerboard"){
      TextureChecker *t = new TextureChecker();
      tex = t;
      
      // print out procedural texture
      if(print)
        cout << "  " << "Texture: Checker Board" << endl;
      
      // loop through checkerboard colors
      for(XMLElement *child = e->FirstChildElement(); child != NULL; child   = child->NextSiblingElement()){
        
        // check checkerboard colors
        string cName(child->Value());
        if(cName == "color1"){
          Color c(0.0, 0.0, 0.0);
          readColor(child, c);
          t->setColor1(c);
          if(print)
            cout << "  " << "color1 " << c.r << " " << c.g << " " << c.b   << endl;
        }else if(cName == "color2"){
          Color c(0.0, 0.0, 0.0);
          readColor(child, c);
          t->setColor2(c);
          if(print)
            cout << "  " << "color2 " << c.r << " " << c.g << " " << c.b   << endl;
        }
      }
      
      // add child to texture list
      textures.append(tex, name);
    
      // otherwise, load a texture file
    }else{
      
      // update with texture folder
      name = "textures/" + name;
      
      // print out texture file
      if(print)
        cout << "  " << "Texture: File \"" << name << "\"" << endl;
      
      // get the texture if it exists, else create it!
      tex = textures.find(name);
      if(tex == NULL){
        TextureFile *f = new TextureFile();
        
        // set texture file variables
        tex = f;
        f->setName(name);
        
        // try to load file
        if(!f->load()){
          cout << " -- " << "Error loading file!" << endl;
          delete tex;
          tex = NULL;
          
        // successful load texture
        }else{
          textures.append(tex, name);
        }
      }
    }
    
    // set the texture map to the texture
    TextureMap *m = new TextureMap(tex);
    loadTransform(m, e, 0);
    return m;
  
  // catch no texture
  }else
    return NULL;
}


// read in a vector from an XML element
void readVector(XMLElement *e, Point &v){
  
  // set vector values
  double x = (double) v.x;
  double y = (double) v.y;
  double z = (double) v.z;
  e->QueryDoubleAttribute("x", &x);
  e->QueryDoubleAttribute("y", &y);
  e->QueryDoubleAttribute("z", &z);
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
void readFloat(XMLElement *e, float &f, string name){
  double d = (double) f;
  e->QueryDoubleAttribute(name.c_str(), &d);
  f = (float) d;
}
