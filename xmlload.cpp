
//-------------------------------------------------------------------------------
///
/// \file       xmlload.cpp 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    1.0
/// \date       August 26, 2013
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-------------------------------------------------------------------------------
 
#include "scene.h"
#include "object/sphere.h"
#include "library/tinyxml2/tinyxml2.h"
#include "library/tinyxml2/tinyxml2.cpp"
#include <string.h>
#include <stdio.h>

using namespace scene;

//-------------------------------------------------------------------------------
 
Node rootNode;
Camera camera;
RenderImage renderImage;
 
//-------------------------------------------------------------------------------
 
#define COMPARE(a,b) (strcmp(a,b)==0)
 
//-------------------------------------------------------------------------------
 
void LoadNode(Node *node, tinyxml2::XMLElement *element, int level=0);
void ReadVector(tinyxml2::XMLElement *element, Point3 &v);
void ReadFloat (tinyxml2::XMLElement *element, float &f);
 
//-------------------------------------------------------------------------------
 
int LoadScene(const char *filename)
{
    tinyxml2::XMLDocument doc(filename);
    if ( doc.LoadFile(filename) ) {
        printf("Failed to load the file \"%s\"\n", filename);
        return 0;
    }
 
    tinyxml2::XMLElement *xml = doc.FirstChildElement("xml");
    if ( ! xml ) {
        printf("No \"xml\" tag found.\n");
        return 0;
    }
 
    tinyxml2::XMLElement *scene = xml->FirstChildElement("scene");
    if ( ! scene ) {
        printf("No \"scene\" tag found.\n");
        return 0;
    }
 
    tinyxml2::XMLElement *cam = xml->FirstChildElement("camera");
    if ( ! cam ) {
        printf("No \"camera\" tag found.\n");
        return 0;
    }
 
    rootNode.Init();
    LoadNode( &rootNode, scene );
 
    // Load Camera
    camera.Init();
    camera.dir += camera.pos;
    tinyxml2::XMLElement *camChild = cam->FirstChildElement();
    while ( camChild ) {
        if      ( COMPARE( camChild->Value(), "position"  ) ) ReadVector(camChild,camera.pos);
        else if ( COMPARE( camChild->Value(), "target"    ) ) ReadVector(camChild,camera.dir);
        else if ( COMPARE( camChild->Value(), "up"        ) ) ReadVector(camChild,camera.up);
        else if ( COMPARE( camChild->Value(), "fov"       ) ) ReadFloat (camChild,camera.fov);
        else if ( COMPARE( camChild->Value(), "width"     ) ) camChild->QueryIntAttribute("value", &camera.imgWidth);
        else if ( COMPARE( camChild->Value(), "height"    ) ) camChild->QueryIntAttribute("value", &camera.imgHeight);
        camChild = camChild->NextSiblingElement();
    }
    camera.dir -= camera.pos;
    camera.dir.Normalize();
    Point3 x = camera.dir ^ camera.up;
    camera.up = (x ^ camera.dir).GetNormalized();
 
    renderImage.Init( camera.imgWidth, camera.imgHeight );
 
    return 1;
}
 
//-------------------------------------------------------------------------------
 
void PrintIndent(int level) { for ( int i=0; i<level; i++) printf("   "); }
 
//-------------------------------------------------------------------------------
 
void LoadNode(Node *node, tinyxml2::XMLElement *element, int level)
{
    tinyxml2::XMLElement *child = element->FirstChildElement();
 
    while ( child ) {
 
        if ( COMPARE( child->Value(), "object" ) ) {
 
            Node *childNode = new Node;
            node->AppendChild(childNode);
 
            // name
            const char* name = child->Attribute("name");
            childNode->SetName(name);
            PrintIndent(level);
            printf("object [");
            if ( name ) printf("%s",name);
            printf("]");
 
            // type
            const char* type = child->Attribute("type");
            if ( type ) {
                if ( COMPARE(type,"sphere") ) {
                    //childNode->SetObject( &theSphere );
                    printf(" - Sphere");
                }
            }
 
            printf("\n");
 
            LoadNode(childNode,child,level+1);
 
        } else if ( COMPARE( child->Value(), "scale" ) ) {
 
            float v=1;
            Point3 s(1,1,1);
            ReadFloat( child, v );
            ReadVector( child, s );
            s *= v;
            node->Scale(s.x,s.y,s.z);
            PrintIndent(level);
            printf("scale %f %f %f\n",s.x,s.y,s.z);
 
        } else if ( COMPARE( child->Value(), "translate" ) ) {
 
            Point3 t(0,0,0);
            ReadVector(child,t);
            node->Translate(t);
            PrintIndent(level);
            printf("translate %f %f %f\n",t.x,t.y,t.z);
 
        }
 
        child = child->NextSiblingElement();
    }
}
 
//-------------------------------------------------------------------------------
 
void ReadVector(tinyxml2::XMLElement *element, Point3 &v)
{
    double x = (double) v.x;
    double y = (double) v.y;
    double z = (double) v.z;
    element->QueryDoubleAttribute( "x", &x );
    element->QueryDoubleAttribute( "y", &y );
    element->QueryDoubleAttribute( "z", &z );
    v.x = (float) x;
    v.y = (float) y;
    v.z = (float) z;
}
 
//-------------------------------------------------------------------------------
 
void ReadFloat (tinyxml2::XMLElement *element, float &f)
{
    double d = (double) f;
    element->QueryDoubleAttribute( "value", &d );
    f = (float) d;
}
 
//-------------------------------------------------------------------------------
